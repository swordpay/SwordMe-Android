// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/sync_socket.h"

#include <limits.h>
#include <stddef.h>

#include "base/logging.h"
#include "base/rand_util.h"
#include "base/stl_util.h"
#include "base/threading/scoped_blocking_call.h"
#include "base/win/scoped_handle.h"

namespace base {

using win::ScopedHandle;

namespace {
// IMPORTANT: do not change how this name is generated because it will break
// in sandboxed scenarios as we might have by-name policies that allow pipe
// creation. Also keep the secure random number generation.
const wchar_t kPipeNameFormat[] = L"\\\\.\\pipe\\chrome.sync.%u.%u.%lu";
const size_t kPipePathMax = base::size(kPipeNameFormat) + (3 * 10) + 1;

// we clamp message lengths, which are size_t, to no more than INT_MAX.
const size_t kMaxMessageLength = static_cast<size_t>(INT_MAX);

const int kOutBufferSize = 4096;
const int kInBufferSize = 4096;
const int kDefaultTimeoutMilliSeconds = 1000;

bool CreatePairImpl(ScopedHandle* socket_a,
                    ScopedHandle* socket_b,
                    bool overlapped) {
  DCHECK_NE(socket_a, socket_b);
  DCHECK(!socket_a->IsValid());
  DCHECK(!socket_b->IsValid());

  wchar_t name[kPipePathMax];
  ScopedHandle handle_a;
  DWORD flags = PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE;
  if (overlapped)
    flags |= FILE_FLAG_OVERLAPPED;

  do {
    unsigned long rnd_name;
    RandBytes(&rnd_name, sizeof(rnd_name));

    swprintf(name, kPipePathMax,
             kPipeNameFormat,
             GetCurrentProcessId(),
             GetCurrentThreadId(),
             rnd_name);

    handle_a.Set(CreateNamedPipeW(
        name,
        flags,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
        1,
        kOutBufferSize,
        kInBufferSize,
        kDefaultTimeoutMilliSeconds,
        NULL));
  } while (!handle_a.IsValid() &&
           (GetLastError() == ERROR_PIPE_BUSY));

  if (!handle_a.IsValid()) {
    NOTREACHED();
    return false;
  }



  flags = SECURITY_SQOS_PRESENT | SECURITY_ANONYMOUS;
  if (overlapped)
    flags |= FILE_FLAG_OVERLAPPED;

  ScopedHandle handle_b(CreateFileW(name,
                                    GENERIC_READ | GENERIC_WRITE,
                                    0,          // no sharing.
                                    NULL,       // default security attributes.
                                    OPEN_EXISTING,  // opens existing pipe.
                                    flags,
                                    NULL));     // no template file.
  if (!handle_b.IsValid()) {
    DPLOG(ERROR) << "CreateFileW failed";
    return false;
  }

  if (!ConnectNamedPipe(handle_a.Get(), NULL)) {
    DWORD error = GetLastError();
    if (error != ERROR_PIPE_CONNECTED) {
      DPLOG(ERROR) << "ConnectNamedPipe failed";
      return false;
    }
  }

  *socket_a = std::move(handle_a);
  *socket_b = std::move(handle_b);

  return true;
}

DWORD GetNextChunkSize(size_t current_pos, size_t max_size) {

  return static_cast<DWORD>(((max_size - current_pos) <= UINT_MAX) ?
      (max_size - current_pos) : UINT_MAX);
}

// overlapped fashion and waits for IO completion.  The function also waits
// on an event that can be used to cancel the operation.  If the operation
// is cancelled, the function returns and closes the relevant socket object.
template <typename BufferType, typename Function>
size_t CancelableFileOperation(Function operation,
                               HANDLE file,
                               BufferType* buffer,
                               size_t length,
                               WaitableEvent* io_event,
                               WaitableEvent* cancel_event,
                               CancelableSyncSocket* socket,
                               DWORD timeout_in_ms) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  static_assert(sizeof(buffer[0]) == sizeof(char), "incorrect buffer type");
  DCHECK_GT(length, 0u);
  DCHECK_LE(length, kMaxMessageLength);
  DCHECK_NE(file, SyncSocket::kInvalidHandle);

  TimeTicks current_time, finish_time;
  if (timeout_in_ms != INFINITE) {
    current_time = TimeTicks::Now();
    finish_time =
        current_time + base::TimeDelta::FromMilliseconds(timeout_in_ms);
  }

  size_t count = 0;
  do {

    OVERLAPPED ol = { 0 };
    ol.hEvent = io_event->handle();

    const DWORD chunk = GetNextChunkSize(count, length);


    DWORD len = 0;
    const BOOL operation_ok = operation(
        file, static_cast<BufferType*>(buffer) + count, chunk, &len, &ol);
    if (!operation_ok) {
      if (::GetLastError() == ERROR_IO_PENDING) {
        HANDLE events[] = { io_event->handle(), cancel_event->handle() };
        const int wait_result = WaitForMultipleObjects(
            base::size(events), events, FALSE,
            timeout_in_ms == INFINITE
                ? timeout_in_ms
                : static_cast<DWORD>(
                      (finish_time - current_time).InMilliseconds()));
        if (wait_result != WAIT_OBJECT_0 + 0) {



          CancelIo(file);
        }


        if (!GetOverlappedResult(file, &ol, &len, TRUE))
          len = 0;

        if (wait_result == WAIT_OBJECT_0 + 1) {
          DVLOG(1) << "Shutdown was signaled. Closing socket.";
          socket->Close();
          return count;
        }


        DCHECK(wait_result == WAIT_OBJECT_0 + 0 || wait_result == WAIT_TIMEOUT);
      } else {
        break;
      }
    }

    count += len;

    if (len != chunk)
      break;


    if (timeout_in_ms != INFINITE && count < length)
      current_time = base::TimeTicks::Now();
  } while (count < length &&
           (timeout_in_ms == INFINITE || current_time < finish_time));

  return count;
}

}  // namespace

bool SyncSocket::CreatePair(SyncSocket* socket_a, SyncSocket* socket_b) {
  return CreatePairImpl(&socket_a->handle_, &socket_b->handle_, false);
}

void SyncSocket::Close() {
  handle_.Close();
}

size_t SyncSocket::Send(const void* buffer, size_t length) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  DCHECK_GT(length, 0u);
  DCHECK_LE(length, kMaxMessageLength);
  DCHECK(IsValid());
  size_t count = 0;
  while (count < length) {
    DWORD len;
    DWORD chunk = GetNextChunkSize(count, length);
    if (::WriteFile(handle(), static_cast<const char*>(buffer) + count, chunk,
                    &len, NULL) == FALSE) {
      return count;
    }
    count += len;
  }
  return count;
}

size_t SyncSocket::ReceiveWithTimeout(void* buffer,
                                      size_t length,
                                      TimeDelta timeout) {
  NOTIMPLEMENTED();
  return 0;
}

size_t SyncSocket::Receive(void* buffer, size_t length) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  DCHECK_GT(length, 0u);
  DCHECK_LE(length, kMaxMessageLength);
  DCHECK(IsValid());
  size_t count = 0;
  while (count < length) {
    DWORD len;
    DWORD chunk = GetNextChunkSize(count, length);
    if (::ReadFile(handle(), static_cast<char*>(buffer) + count, chunk, &len,
                   NULL) == FALSE) {
      return count;
    }
    count += len;
  }
  return count;
}

size_t SyncSocket::Peek() {
  DWORD available = 0;
  PeekNamedPipe(handle(), NULL, 0, NULL, &available, NULL);
  return available;
}

bool SyncSocket::IsValid() const {
  return handle_.IsValid();
}

SyncSocket::Handle SyncSocket::handle() const {
  return handle_.Get();
}

SyncSocket::Handle SyncSocket::Release() {
  return handle_.Take();
}

bool CancelableSyncSocket::Shutdown() {


  shutdown_event_.Signal();
  return true;
}

void CancelableSyncSocket::Close() {
  SyncSocket::Close();
  shutdown_event_.Reset();
}

size_t CancelableSyncSocket::Send(const void* buffer, size_t length) {
  static const DWORD kWaitTimeOutInMs = 500;
  return CancelableFileOperation(
      &::WriteFile, handle(), reinterpret_cast<const char*>(buffer), length,
      &file_operation_, &shutdown_event_, this, kWaitTimeOutInMs);
}

size_t CancelableSyncSocket::Receive(void* buffer, size_t length) {
  return CancelableFileOperation(
      &::ReadFile, handle(), reinterpret_cast<char*>(buffer), length,
      &file_operation_, &shutdown_event_, this, INFINITE);
}

size_t CancelableSyncSocket::ReceiveWithTimeout(void* buffer,
                                                size_t length,
                                                TimeDelta timeout) {
  return CancelableFileOperation(&::ReadFile, handle(),
                                 reinterpret_cast<char*>(buffer), length,
                                 &file_operation_, &shutdown_event_, this,
                                 static_cast<DWORD>(timeout.InMilliseconds()));
}

bool CancelableSyncSocket::CreatePair(CancelableSyncSocket* socket_a,
                                      CancelableSyncSocket* socket_b) {
  return CreatePairImpl(&socket_a->handle_, &socket_b->handle_, true);
}

}  // namespace base
