/*!
 * \copy
 *     Copyright (c)  2009-2015, Cisco Systems
 *     All rights reserved.
 *
 *     Redistribution and use in source and binary forms, with or without
 *     modification, are permitted provided that the following conditions
 *     are met:
 *
 *        * Redistributions of source code must retain the above copyright
 *          notice, this list of conditions and the following disclaimer.
 *
 *        * Redistributions in binary form must reproduce the above copyright
 *          notice, this list of conditions and the following disclaimer in
 *          the documentation and/or other materials provided with the
 *          distribution.
 *
 *     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *     "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *     LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *     FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *     COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *     INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *     BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *     CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *     LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *     ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *     POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * \file    WelsThreadPool.cpp
 *
 * \brief   functions for Thread Pool
 *
 * \date    5/09/2012 Created
 *
 *************************************************************************************
 */
#include "typedefs.h"
#include "memory_align.h"
#include "WelsThreadPool.h"

namespace WelsCommon {

namespace {

CWelsLock& GetInitLock() {
  static CWelsLock *initLock = new CWelsLock;
  return *initLock;
}

}

int32_t CWelsThreadPool::m_iRefCount = 0;
int32_t CWelsThreadPool::m_iMaxThreadNum = DEFAULT_THREAD_NUM;
CWelsThreadPool* CWelsThreadPool::m_pThreadPoolSelf = NULL;

CWelsThreadPool::CWelsThreadPool() :
  m_cWaitedTasks (NULL), m_cIdleThreads (NULL), m_cBusyThreads (NULL) {
}


CWelsThreadPool::~CWelsThreadPool() {

  if (0 != m_iRefCount) {
    m_iRefCount = 0;
    Uninit();
  }
}

WELS_THREAD_ERROR_CODE CWelsThreadPool::SetThreadNum (int32_t iMaxThreadNum) {
  CWelsAutoLock  cLock (GetInitLock());

  if (m_iRefCount != 0) {
    return WELS_THREAD_ERROR_GENERAL;
  }

  if (iMaxThreadNum <= 0) {
    iMaxThreadNum = 1;
  }
  m_iMaxThreadNum = iMaxThreadNum;
  return WELS_THREAD_ERROR_OK;
}


CWelsThreadPool* CWelsThreadPool::AddReference() {
  CWelsAutoLock  cLock (GetInitLock());
  if (m_pThreadPoolSelf == NULL) {
    m_pThreadPoolSelf = new CWelsThreadPool();
    if (!m_pThreadPoolSelf) {
      return NULL;
    }
  }

  if (m_iRefCount == 0) {
    if (WELS_THREAD_ERROR_OK != m_pThreadPoolSelf->Init()) {
      m_pThreadPoolSelf->Uninit();
      delete m_pThreadPoolSelf;
      m_pThreadPoolSelf = NULL;
      return NULL;
    }
  }


  ++ m_iRefCount;

  return m_pThreadPoolSelf;
}

void CWelsThreadPool::RemoveInstance() {
  CWelsAutoLock  cLock (GetInitLock());

  -- m_iRefCount;
  if (0 == m_iRefCount) {
    StopAllRunning();
    Uninit();
    if (m_pThreadPoolSelf) {
      delete m_pThreadPoolSelf;
      m_pThreadPoolSelf = NULL;
    }

  }
}


bool CWelsThreadPool::IsReferenced() {
  CWelsAutoLock  cLock (GetInitLock());
  return (m_iRefCount > 0);
}


WELS_THREAD_ERROR_CODE CWelsThreadPool::OnTaskStart (CWelsTaskThread* pThread, IWelsTask* pTask) {
  AddThreadToBusyList (pThread);

  return WELS_THREAD_ERROR_OK;
}

WELS_THREAD_ERROR_CODE CWelsThreadPool::OnTaskStop (CWelsTaskThread* pThread, IWelsTask* pTask) {


  RemoveThreadFromBusyList (pThread);
  AddThreadToIdleQueue (pThread);

  if (pTask && pTask->GetSink()) {

    pTask->GetSink()->OnTaskExecuted();

  }





  SignalThread();

  return WELS_THREAD_ERROR_OK;
}

WELS_THREAD_ERROR_CODE CWelsThreadPool::Init() {


  CWelsAutoLock  cLock (m_cLockPool);

  m_cWaitedTasks = new CWelsNonDuplicatedList<IWelsTask>();
  m_cIdleThreads = new CWelsNonDuplicatedList<CWelsTaskThread>();
  m_cBusyThreads = new CWelsList<CWelsTaskThread>();
  if (NULL == m_cWaitedTasks || NULL == m_cIdleThreads || NULL == m_cBusyThreads) {
    return WELS_THREAD_ERROR_GENERAL;
  }

  for (int32_t i = 0; i < m_iMaxThreadNum; i++) {
    if (WELS_THREAD_ERROR_OK != CreateIdleThread()) {
      return WELS_THREAD_ERROR_GENERAL;
    }
  }

  if (WELS_THREAD_ERROR_OK != Start()) {
    return WELS_THREAD_ERROR_GENERAL;
  }

  return WELS_THREAD_ERROR_OK;
}

WELS_THREAD_ERROR_CODE CWelsThreadPool::StopAllRunning() {
  WELS_THREAD_ERROR_CODE iReturn = WELS_THREAD_ERROR_OK;

  ClearWaitedTasks();

  while (GetBusyThreadNum() > 0) {

    WelsSleep (10);
  }

  if (GetIdleThreadNum() != m_iMaxThreadNum) {
    iReturn = WELS_THREAD_ERROR_GENERAL;
  }

  return iReturn;
}

WELS_THREAD_ERROR_CODE CWelsThreadPool::Uninit() {
  WELS_THREAD_ERROR_CODE iReturn = WELS_THREAD_ERROR_OK;
  CWelsAutoLock  cLock (m_cLockPool);

  iReturn = StopAllRunning();
  if (WELS_THREAD_ERROR_OK != iReturn) {
    return iReturn;
  }

  m_cLockIdleTasks.Lock();
  while (m_cIdleThreads->size() > 0) {
    DestroyThread (m_cIdleThreads->begin());
    m_cIdleThreads->pop_front();
  }
  m_cLockIdleTasks.Unlock();

  Kill();

  WELS_DELETE_OP (m_cWaitedTasks);
  WELS_DELETE_OP (m_cIdleThreads);
  WELS_DELETE_OP (m_cBusyThreads);

  return iReturn;
}

void CWelsThreadPool::ExecuteTask() {

  CWelsTaskThread* pThread = NULL;
  IWelsTask*    pTask = NULL;
  while (GetWaitedTaskNum() > 0) {

    pThread = GetIdleThread();
    if (pThread == NULL) {


      break;
    }
    pTask = GetWaitedTask();

    if (pTask) {
      pThread->SetTask (pTask);
    } else {
      AddThreadToIdleQueue (pThread);
    }
  }
}

WELS_THREAD_ERROR_CODE CWelsThreadPool::QueueTask (IWelsTask* pTask) {
  CWelsAutoLock  cLock (m_cLockPool);

  if (GetWaitedTaskNum() == 0) {
    CWelsTaskThread* pThread = GetIdleThread();

    if (pThread != NULL) {

      pThread->SetTask (pTask);

      return WELS_THREAD_ERROR_OK;
    }
  }

  if (false == AddTaskToWaitedList (pTask)) {
    return WELS_THREAD_ERROR_GENERAL;
  }

  SignalThread();
  return WELS_THREAD_ERROR_OK;
}

WELS_THREAD_ERROR_CODE CWelsThreadPool::CreateIdleThread() {
  CWelsTaskThread* pThread = new CWelsTaskThread (this);

  if (NULL == pThread) {
    return WELS_THREAD_ERROR_GENERAL;
  }

  if (WELS_THREAD_ERROR_OK != pThread->Start()) {
    return WELS_THREAD_ERROR_GENERAL;
  }

  AddThreadToIdleQueue (pThread);

  return WELS_THREAD_ERROR_OK;
}

void  CWelsThreadPool::DestroyThread (CWelsTaskThread* pThread) {
  pThread->Kill();
  WELS_DELETE_OP (pThread);

  return;
}

WELS_THREAD_ERROR_CODE CWelsThreadPool::AddThreadToIdleQueue (CWelsTaskThread* pThread) {
  CWelsAutoLock cLock (m_cLockIdleTasks);
  m_cIdleThreads->push_back (pThread);
  return WELS_THREAD_ERROR_OK;
}

WELS_THREAD_ERROR_CODE CWelsThreadPool::AddThreadToBusyList (CWelsTaskThread* pThread) {
  CWelsAutoLock cLock (m_cLockBusyTasks);
  m_cBusyThreads->push_back (pThread);
  return WELS_THREAD_ERROR_OK;
}

WELS_THREAD_ERROR_CODE CWelsThreadPool::RemoveThreadFromBusyList (CWelsTaskThread* pThread) {
  CWelsAutoLock cLock (m_cLockBusyTasks);
  if (m_cBusyThreads->erase (pThread)) {
    return WELS_THREAD_ERROR_OK;
  } else {
    return WELS_THREAD_ERROR_GENERAL;
  }
}

bool  CWelsThreadPool::AddTaskToWaitedList (IWelsTask* pTask) {
  CWelsAutoLock  cLock (m_cLockWaitedTasks);

  return m_cWaitedTasks->push_back (pTask);
}

CWelsTaskThread*   CWelsThreadPool::GetIdleThread() {
  CWelsAutoLock cLock (m_cLockIdleTasks);

  if (NULL == m_cIdleThreads || m_cIdleThreads->size() == 0) {
    return NULL;
  }


  CWelsTaskThread* pThread = m_cIdleThreads->begin();
  m_cIdleThreads->pop_front();
  return pThread;
}

int32_t  CWelsThreadPool::GetBusyThreadNum() {
  return (m_cBusyThreads?m_cBusyThreads->size():0);
}

int32_t  CWelsThreadPool::GetIdleThreadNum() {
  return (m_cIdleThreads?m_cIdleThreads->size():0);
}

int32_t  CWelsThreadPool::GetWaitedTaskNum() {
  return (m_cWaitedTasks?m_cWaitedTasks->size():0);
}

IWelsTask* CWelsThreadPool::GetWaitedTask() {
  CWelsAutoLock lock (m_cLockWaitedTasks);

  if (NULL==m_cWaitedTasks || m_cWaitedTasks->size() == 0) {
    return NULL;
  }

  IWelsTask* pTask = m_cWaitedTasks->begin();

  m_cWaitedTasks->pop_front();

  return pTask;
}

void  CWelsThreadPool::ClearWaitedTasks() {
  CWelsAutoLock cLock (m_cLockWaitedTasks);
  if (NULL == m_cWaitedTasks) {
    return;
  }
  IWelsTask* pTask = NULL;
  while (0 != m_cWaitedTasks->size()) {
    pTask = m_cWaitedTasks->begin();
    if (pTask->GetSink()) {
      pTask->GetSink()->OnTaskCancelled();
    }
    m_cWaitedTasks->pop_front();
  }
}

}
