/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 *
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 *
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 *
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.] */

#ifndef OPENSSL_HEADER_BIO_H
#define OPENSSL_HEADER_BIO_H

#include <openssl/base.h>

#include <stdio.h>  // For FILE

#include <openssl/buffer.h>
#include <openssl/err.h>  // for ERR_print_errors_fp
#include <openssl/ex_data.h>
#include <openssl/stack.h>
#include <openssl/thread.h>

#if defined(__cplusplus)
extern "C" {
#endif



DEFINE_STACK_OF(BIO)

// It returns the fresh |BIO|, or NULL on error.
OPENSSL_EXPORT BIO *BIO_new(const BIO_METHOD *method);

// drops to zero, it calls the destroy callback, if present, on the method and
// frees |bio| itself. It then repeats that for the next BIO in the chain, if
// any.
//
// It returns one on success or zero otherwise.
OPENSSL_EXPORT int BIO_free(BIO *bio);

// value. This is provided for API-compat.
//
// TODO(fork): remove.
OPENSSL_EXPORT void BIO_vfree(BIO *bio);

OPENSSL_EXPORT int BIO_up_ref(BIO *bio);


// bytes read, zero on EOF, or a negative number on error.
OPENSSL_EXPORT int BIO_read(BIO *bio, void *data, int len);

// It returns the number of bytes read or a negative number on error. The
// phrase "reads a line" is in quotes in the previous sentence because the
// exact operation depends on the BIO's method. For example, a digest BIO will
// return the digest in response to a |BIO_gets| call.
//
// TODO(fork): audit the set of BIOs that we end up needing. If all actually
// return a line for this call, remove the warning above.
OPENSSL_EXPORT int BIO_gets(BIO *bio, char *buf, int size);

// bytes written or a negative number on error.
OPENSSL_EXPORT int BIO_write(BIO *bio, const void *data, int len);

// It returns one if all bytes were successfully written and zero on error.
OPENSSL_EXPORT int BIO_write_all(BIO *bio, const void *data, size_t len);

// number of bytes written or a negative number on error.
OPENSSL_EXPORT int BIO_puts(BIO *bio, const char *buf);

// otherwise.
OPENSSL_EXPORT int BIO_flush(BIO *bio);

//
// These are generic functions for sending control requests to a BIO. In
// general one should use the wrapper functions like |BIO_get_close|.

// be one of the |BIO_C_*| values.
OPENSSL_EXPORT long BIO_ctrl(BIO *bio, int cmd, long larg, void *parg);

// pointer as |parg| and returns the value that is written to it, or NULL if
// the control request returns <= 0.
OPENSSL_EXPORT char *BIO_ptr_ctrl(BIO *bp, int cmd, long larg);

// as |parg|.
OPENSSL_EXPORT long BIO_int_ctrl(BIO *bp, int cmd, long larg, int iarg);

// depends on the concrete type of |bio|. It returns one on success and zero
// otherwise.
OPENSSL_EXPORT int BIO_reset(BIO *bio);

// meaning of which depends on the concrete type of |bio|. Note that in the
// case of BIO_pair this always returns non-zero.
OPENSSL_EXPORT int BIO_eof(BIO *bio);

OPENSSL_EXPORT void BIO_set_flags(BIO *bio, int flags);

OPENSSL_EXPORT int BIO_test_flags(const BIO *bio, int flags);

// while reading (i.e. EAGAIN), indicating that the caller should retry the
// read.
OPENSSL_EXPORT int BIO_should_read(const BIO *bio);

// while writing (i.e. EAGAIN), indicating that the caller should retry the
// write.
OPENSSL_EXPORT int BIO_should_write(const BIO *bio);

// operation is temporary and thus the operation should be retried. Otherwise,
// it was a permanent error and it returns zero.
OPENSSL_EXPORT int BIO_should_retry(const BIO *bio);

// error while performing a special I/O operation, indicating that the caller
// should retry. The operation that caused the error is returned by
// |BIO_get_retry_reason|.
OPENSSL_EXPORT int BIO_should_io_special(const BIO *bio);

#define BIO_RR_CONNECT 0x02

#define BIO_RR_ACCEPT 0x03

// retried. The return value is one of the |BIO_RR_*| values.
OPENSSL_EXPORT int BIO_get_retry_reason(const BIO *bio);

OPENSSL_EXPORT void BIO_clear_flags(BIO *bio, int flags);

// flags on |bio|.
OPENSSL_EXPORT void BIO_set_retry_read(BIO *bio);

// flags on |bio|.
OPENSSL_EXPORT void BIO_set_retry_write(BIO *bio);

// |BIO_FLAGS_IO_SPECIAL| and |BIO_FLAGS_SHOULD_RETRY| flags from |bio|.
OPENSSL_EXPORT int BIO_get_retry_flags(BIO *bio);

// |BIO_FLAGS_IO_SPECIAL| and |BIO_FLAGS_SHOULD_RETRY| flags from |bio|.
OPENSSL_EXPORT void BIO_clear_retry_flags(BIO *bio);

// values.
OPENSSL_EXPORT int BIO_method_type(const BIO *bio);

#define BIO_CB_FREE 0x01
#define BIO_CB_READ 0x02
#define BIO_CB_WRITE 0x03
#define BIO_CB_PUTS 0x04
#define BIO_CB_GETS 0x05
#define BIO_CB_CTRL 0x06

// The BIO_CB_RETURN flag indicates if it is after the call
#define BIO_CB_RETURN 0x80

// BIO operations. The |event| argument is one of |BIO_CB_*| and can be ORed
// with |BIO_CB_RETURN| if the callback is being made after the operation in
// question. In that case, |return_value| will contain the return value from
// the operation.
typedef long (*bio_info_cb)(BIO *bio, int event, const char *parg, int cmd,
                            long larg, long return_value);

// arg will generally be |BIO_CTRL_SET_CALLBACK| but arbitrary command values
// can be interpreted by the |BIO|.
OPENSSL_EXPORT long BIO_callback_ctrl(BIO *bio, int cmd, bio_info_cb fp);

OPENSSL_EXPORT size_t BIO_pending(const BIO *bio);

// OpenSSL.
OPENSSL_EXPORT size_t BIO_ctrl_pending(const BIO *bio);

OPENSSL_EXPORT size_t BIO_wpending(const BIO *bio);

// the type of |bio| but, for example, a memory BIO interprets the close flag
// as meaning that it owns its buffer. It returns one on success and zero
// otherwise.
OPENSSL_EXPORT int BIO_set_close(BIO *bio, int close_flag);

// |bio|.
OPENSSL_EXPORT size_t BIO_number_read(const BIO *bio);

// |bio|.
OPENSSL_EXPORT size_t BIO_number_written(const BIO *bio);

//
// BIOs can be put into chains where the output of one is used as the input of
// the next etc. The most common case is a buffering BIO, which accepts and
// buffers writes until flushed into the next BIO in the chain.

// It returns |bio|. Note that |appended_bio| may be the head of a chain itself
// and thus this function can be used to join two chains.
//
// BIO_push takes ownership of the caller's reference to |appended_bio|.
OPENSSL_EXPORT BIO *BIO_push(BIO *bio, BIO *appended_bio);

// the chain, or NULL if there is no next BIO.
//
// The caller takes ownership of the chain's reference to |bio|.
OPENSSL_EXPORT BIO *BIO_pop(BIO *bio);

// no such BIO.
OPENSSL_EXPORT BIO *BIO_next(BIO *bio);

//
// TODO(fork): update callers and remove.
OPENSSL_EXPORT void BIO_free_all(BIO *bio);

// |type|, which is one of the |BIO_TYPE_*| values.
OPENSSL_EXPORT BIO *BIO_find_type(BIO *bio, int type);

// the next BIO in the chain.
OPENSSL_EXPORT void BIO_copy_next_retry(BIO *bio);


// It returns the number of bytes written or a negative number on error.
OPENSSL_EXPORT int BIO_printf(BIO *bio, const char *format, ...)
    OPENSSL_PRINTF_FORMAT_FUNC(2, 3);


// success and zero otherwise.
OPENSSL_EXPORT int BIO_indent(BIO *bio, unsigned indent, unsigned max_indent);

// by |indent| spaces.
OPENSSL_EXPORT int BIO_hexdump(BIO *bio, const uint8_t *data, size_t len,
                               unsigned indent);

// using human readable strings where possible.
OPENSSL_EXPORT void ERR_print_errors(BIO *bio);

// |*out| to be an allocated buffer (that should be freed with |OPENSSL_free|),
// |*out_size| to the length, in bytes, of that buffer and returns one.
// Otherwise it returns zero.
//
// If the length of the object is greater than |max_len| or 2^32 then the
// function will fail. Long-form tags are not supported. If the length of the
// object is indefinite the full contents of |bio| are read, unless it would be
// greater than |max_len|, in which case the function fails.
//
// If the function fails then some unknown amount of data may have been read
// from |bio|.
OPENSSL_EXPORT int BIO_read_asn1(BIO *bio, uint8_t **out, size_t *out_len,
                                 size_t max_len);

//
// Memory BIOs can be used as a read-only source (with |BIO_new_mem_buf|) or a
// writable sink (with |BIO_new|, |BIO_s_mem| and |BIO_mem_contents|). Data
// written to a writable, memory BIO can be recalled by reading from it.
//
// Calling |BIO_reset| on a read-only BIO resets it to the original contents.
// On a writable BIO, it clears any data.
//
// If the close flag is set to |BIO_NOCLOSE| (not the default) then the
// underlying |BUF_MEM| will not be freed when the |BIO| is freed.
//
// Memory BIOs support |BIO_gets| and |BIO_puts|.
//
// |BIO_ctrl_pending| returns the number of bytes currently stored.

// flag" is passed to a BIO function.
#define BIO_NOCLOSE 0
#define BIO_CLOSE 1

OPENSSL_EXPORT const BIO_METHOD *BIO_s_mem(void);

// It does not take ownership of |buf|. It returns the BIO or NULL on error.
//
// If |len| is negative, then |buf| is treated as a NUL-terminated string, but
// don't depend on this in new code.
OPENSSL_EXPORT BIO *BIO_new_mem_buf(const void *buf, int len);

// |bio| and |*out_len| to contain the length of that data. It returns one on
// success and zero otherwise.
OPENSSL_EXPORT int BIO_mem_contents(const BIO *bio,
                                    const uint8_t **out_contents,
                                    size_t *out_len);

// and returns the length of the data.
//
// WARNING: don't use this, use |BIO_mem_contents|. A return value of zero from
// this function can mean either that it failed or that the memory buffer is
// empty.
OPENSSL_EXPORT long BIO_get_mem_data(BIO *bio, char **contents);

// |bio|. It returns one on success or zero on error.
OPENSSL_EXPORT int BIO_get_mem_ptr(BIO *bio, BUF_MEM **out);

// non-zero, then |b| will be freed when |bio| is closed. Returns one on
// success or zero otherwise.
OPENSSL_EXPORT int BIO_set_mem_buf(BIO *bio, BUF_MEM *b, int take_ownership);

// |bio| when empty. If |eof_value| is zero then an empty memory BIO will
// return EOF (that is it will return zero and |BIO_should_retry| will be
// false). If |eof_value| is non zero then it will return |eof_value| when it
// is empty and it will set the read retry flag (that is |BIO_read_retry| is
// true). To avoid ambiguity with a normal positive return value, |eof_value|
// should be set to a negative value, typically -1.
//
// For a read-only BIO, the default is zero (EOF). For a writable BIO, the
// default is -1 so that additional data can be written once exhausted.
OPENSSL_EXPORT int BIO_set_mem_eof_return(BIO *bio, int eof_value);

//
// File descriptor BIOs are wrappers around the system's |read| and |write|
// functions. If the close flag is set then then |close| is called on the
// underlying file descriptor when the BIO is freed.
//
// |BIO_reset| attempts to seek the file pointer to the start of file using
// |lseek|.

OPENSSL_EXPORT const BIO_METHOD *BIO_s_fd(void);

// is non-zero, then |fd| will be closed when the BIO is.
OPENSSL_EXPORT BIO *BIO_new_fd(int fd, int close_flag);

// non-zero then |fd| will be closed when |bio| is. It returns one on success
// or zero on error.
//
// This function may also be used with socket BIOs (see |BIO_s_socket| and
// |BIO_new_socket|).
OPENSSL_EXPORT int BIO_set_fd(BIO *bio, int fd, int close_flag);

// |bio| does not wrap a file descriptor. If there is a file descriptor and
// |out_fd| is not NULL, it also sets |*out_fd| to the file descriptor.
//
// This function may also be used with socket BIOs (see |BIO_s_socket| and
// |BIO_new_socket|).
OPENSSL_EXPORT int BIO_get_fd(BIO *bio, int *out_fd);

//
// File BIOs are wrappers around a C |FILE| object.
//
// |BIO_flush| on a file BIO calls |fflush| on the wrapped stream.
//
// |BIO_reset| attempts to seek the file pointer to the start of file using
// |fseek|.
//
// Setting the close flag causes |fclose| to be called on the stream when the
// BIO is freed.

OPENSSL_EXPORT const BIO_METHOD *BIO_s_file(void);

// See the |fopen| manual page for details of the mode argument.
OPENSSL_EXPORT BIO *BIO_new_file(const char *filename, const char *mode);

// |close_flag| is |BIO_CLOSE|, then |fclose| will be called on |stream| when
// the BIO is closed.
OPENSSL_EXPORT BIO *BIO_new_fp(FILE *stream, int close_flag);

// on success and zero otherwise.
OPENSSL_EXPORT int BIO_get_fp(BIO *bio, FILE **out_file);

// |fclose| will be called on |file| when |bio| is closed. It returns one on
// success and zero otherwise.
OPENSSL_EXPORT int BIO_set_fp(BIO *bio, FILE *file, int close_flag);

// |FILE| for |bio|. It returns one on success and zero otherwise. The |FILE|
// will be closed when |bio| is freed.
OPENSSL_EXPORT int BIO_read_filename(BIO *bio, const char *filename);

// |FILE| for |bio|. It returns one on success and zero otherwise. The |FILE|
// will be closed when |bio| is freed.
OPENSSL_EXPORT int BIO_write_filename(BIO *bio, const char *filename);

// the |FILE| for |bio|. It returns one on success and zero otherwise. The
// |FILE| will be closed when |bio| is freed.
OPENSSL_EXPORT int BIO_append_filename(BIO *bio, const char *filename);

// as the |FILE| for |bio|. It returns one on success and zero otherwise. The
// |FILE| will be closed when |bio| is freed.
OPENSSL_EXPORT int BIO_rw_filename(BIO *bio, const char *filename);

//
// Socket BIOs behave like file descriptor BIOs but, on Windows systems, wrap
// the system's |recv| and |send| functions instead of |read| and |write|. On
// Windows, file descriptors are provided by C runtime and are not
// interchangeable with sockets.
//
// Socket BIOs may be used with |BIO_set_fd| and |BIO_get_fd|.
//
// TODO(davidben): Add separate APIs and fix the internals to use |SOCKET|s
// around rather than rely on int casts.

OPENSSL_EXPORT const BIO_METHOD *BIO_s_socket(void);

// write to the socket |fd|. If |close_flag| is |BIO_CLOSE| then closing the
// BIO will close |fd|. It returns the fresh |BIO| or NULL on error.
OPENSSL_EXPORT BIO *BIO_new_socket(int fd, int close_flag);

//
// A connection BIO creates a network connection and transfers data over the
// resulting socket.

OPENSSL_EXPORT const BIO_METHOD *BIO_s_connect(void);

// The |host_and_optional_port| argument should be of the form
// "www.example.com" or "www.example.com:443". If the port is omitted, it must
// be provided with |BIO_set_conn_port|.
//
// It returns the new BIO on success, or NULL on error.
OPENSSL_EXPORT BIO *BIO_new_connect(const char *host_and_optional_port);

// optional port that |bio| will connect to. If the port is omitted, it must be
// provided with |BIO_set_conn_port|.
//
// It returns one on success and zero otherwise.
OPENSSL_EXPORT int BIO_set_conn_hostname(BIO *bio,
                                         const char *host_and_optional_port);

// will connect to. It returns one on success and zero otherwise.
OPENSSL_EXPORT int BIO_set_conn_port(BIO *bio, const char *port_str);

// It returns one on success and zero otherwise.
OPENSSL_EXPORT int BIO_set_conn_int_port(BIO *bio, const int *port);

// returns one on success and zero otherwise.
OPENSSL_EXPORT int BIO_set_nbio(BIO *bio, int on);

// one on success and <= 0 otherwise.
OPENSSL_EXPORT int BIO_do_connect(BIO *bio);

//
// TODO(fork): not implemented.

#define BIO_CTRL_DGRAM_QUERY_MTU 40  // as kernel for current MTU

#define BIO_CTRL_DGRAM_SET_MTU 42 /* set cached value for  MTU. want to use
                                     this if asking the kernel fails */

#define BIO_CTRL_DGRAM_MTU_EXCEEDED 43 /* check whether the MTU was exceed in
                                          the previous write operation. */

// and depends on |timeval|, which is not 2038-clean on all platforms.

#define BIO_CTRL_DGRAM_GET_PEER           46

#define BIO_CTRL_DGRAM_GET_FALLBACK_MTU   47

//
// BIO pairs provide a "loopback" like system: a pair of BIOs where data
// written to one can be read from the other and vice versa.

// data written to one can be read from the other and vice versa. The
// |writebuf1| argument gives the size of the buffer used in |*out1| and
// |writebuf2| for |*out2|. It returns one on success and zero on error.
OPENSSL_EXPORT int BIO_new_bio_pair(BIO **out1, size_t writebuf1, BIO **out2,
                                    size_t writebuf2);

// |bio| tried (unsuccessfully) to read.
OPENSSL_EXPORT size_t BIO_ctrl_get_read_request(BIO *bio);

// must have been returned by |BIO_new_bio_pair|) will accept on the next
// |BIO_write| call.
OPENSSL_EXPORT size_t BIO_ctrl_get_write_guarantee(BIO *bio);

// side of the pair. Future |BIO_write| calls on |bio| will fail. It returns
// one on success and zero otherwise.
OPENSSL_EXPORT int BIO_shutdown_wr(BIO *bio);

//
// Consumers can create custom |BIO|s by filling in a |BIO_METHOD| and using
// low-level control functions to set state.

OPENSSL_EXPORT int BIO_get_new_index(void);

// error. The |type| specifies the type that will be returned by
// |BIO_method_type|. If this is unnecessary, this value may be zero. The |name|
// parameter is vestigial and may be NULL.
//
// Use the |BIO_meth_set_*| functions below to initialize the |BIO_METHOD|. The
// function implementations may use |BIO_set_data| and |BIO_get_data| to add
// method-specific state to associated |BIO|s. Additionally, |BIO_set_init| must
// be called after an associated |BIO| is fully initialized. State set via
// |BIO_set_data| may be released by configuring a destructor with
// |BIO_meth_set_destroy|.
OPENSSL_EXPORT BIO_METHOD *BIO_meth_new(int type, const char *name);

OPENSSL_EXPORT void BIO_meth_free(BIO_METHOD *method);

// and returns one. The function should return one on success and zero on
// error.
OPENSSL_EXPORT int BIO_meth_set_create(BIO_METHOD *method,
                                       int (*create)(BIO *));

// and returns one. The function's return value is ignored.
OPENSSL_EXPORT int BIO_meth_set_destroy(BIO_METHOD *method,
                                        int (*destroy)(BIO *));

// returns one. |BIO_METHOD|s which implement |BIO_write| should also implement
// |BIO_CTRL_FLUSH|. (See |BIO_meth_set_ctrl|.)
OPENSSL_EXPORT int BIO_meth_set_write(BIO_METHOD *method,
                                      int (*write)(BIO *, const char *, int));

// returns one.
OPENSSL_EXPORT int BIO_meth_set_read(BIO_METHOD *method,
                                     int (*read)(BIO *, char *, int));

// returns one.
OPENSSL_EXPORT int BIO_meth_set_gets(BIO_METHOD *method,
                                     int (*gets)(BIO *, char *, int));

// returns one.
OPENSSL_EXPORT int BIO_meth_set_ctrl(BIO_METHOD *method,
                                     long (*ctrl)(BIO *, int, long, void *));

// |BIO_get_data|.
OPENSSL_EXPORT void BIO_set_data(BIO *bio, void *ptr);

OPENSSL_EXPORT void *BIO_get_data(BIO *bio);

// initialized, |BIO_read| and |BIO_write| will fail.
OPENSSL_EXPORT void BIO_set_init(BIO *bio, int init);

OPENSSL_EXPORT int BIO_get_init(BIO *bio);


#define BIO_CTRL_RESET 1

#define BIO_CTRL_EOF 2

// type of |BIO|. It is not safe to call generically and should not be
// implemented in new |BIO| types.
#define BIO_CTRL_INFO 3

// arguments are unused.
#define BIO_CTRL_GET_CLOSE 8

// close flag.
#define BIO_CTRL_SET_CLOSE 9

#define BIO_CTRL_PENDING 10

#define BIO_CTRL_FLUSH 11

#define BIO_CTRL_WPENDING 13

// int cb(BIO *bio, int state, int ret)
#define BIO_CTRL_SET_CALLBACK 14

#define BIO_CTRL_GET_CALLBACK 15

#define BIO_CTRL_SET 4
#define BIO_CTRL_GET 5
#define BIO_CTRL_PUSH 6
#define BIO_CTRL_POP 7
#define BIO_CTRL_DUP 12
#define BIO_CTRL_SET_FILENAME 30


// it, and decodes data read from it. |BIO_gets| is not supported. Call
// |BIO_flush| when done writing, to signal that no more data are to be
// encoded. The flag |BIO_FLAGS_BASE64_NO_NL| may be set to encode all the data
// on one line.
//
// Use |EVP_EncodeBlock| and |EVP_DecodeBase64| instead.
OPENSSL_EXPORT const BIO_METHOD *BIO_f_base64(void);

OPENSSL_EXPORT void BIO_set_retry_special(BIO *bio);

OPENSSL_EXPORT int BIO_set_write_buffer_size(BIO *bio, int buffer_size);

OPENSSL_EXPORT void BIO_set_shutdown(BIO *bio, int shutdown);

OPENSSL_EXPORT int BIO_get_shutdown(BIO *bio);

// BoringSSL.
OPENSSL_EXPORT int BIO_meth_set_puts(BIO_METHOD *method,
                                     int (*puts)(BIO *, const char *));


#define BIO_FLAGS_READ 0x01
#define BIO_FLAGS_WRITE 0x02
#define BIO_FLAGS_IO_SPECIAL 0x04
#define BIO_FLAGS_RWS (BIO_FLAGS_READ | BIO_FLAGS_WRITE | BIO_FLAGS_IO_SPECIAL)
#define BIO_FLAGS_SHOULD_RETRY 0x08
#define BIO_FLAGS_BASE64_NO_NL 0x100
// BIO_FLAGS_MEM_RDONLY is used with memory BIOs. It means we shouldn't free up
// or change the data in any way.
#define BIO_FLAGS_MEM_RDONLY 0x200

#define BIO_TYPE_NONE 0
#define BIO_TYPE_MEM (1 | 0x0400)
#define BIO_TYPE_FILE (2 | 0x0400)
#define BIO_TYPE_FD (4 | 0x0400 | 0x0100)
#define BIO_TYPE_SOCKET (5 | 0x0400 | 0x0100)
#define BIO_TYPE_NULL (6 | 0x0400)
#define BIO_TYPE_SSL (7 | 0x0200)
#define BIO_TYPE_MD (8 | 0x0200)                 // passive filter
#define BIO_TYPE_BUFFER (9 | 0x0200)             // filter
#define BIO_TYPE_CIPHER (10 | 0x0200)            // filter
#define BIO_TYPE_BASE64 (11 | 0x0200)            // filter
#define BIO_TYPE_CONNECT (12 | 0x0400 | 0x0100)  // socket - connect
#define BIO_TYPE_ACCEPT (13 | 0x0400 | 0x0100)   // socket for accept
#define BIO_TYPE_PROXY_CLIENT (14 | 0x0200)      // client proxy BIO
#define BIO_TYPE_PROXY_SERVER (15 | 0x0200)      // server proxy BIO
#define BIO_TYPE_NBIO_TEST (16 | 0x0200)         // server proxy BIO
#define BIO_TYPE_NULL_FILTER (17 | 0x0200)
#define BIO_TYPE_BER (18 | 0x0200)         // BER -> bin filter
#define BIO_TYPE_BIO (19 | 0x0400)         // (half a) BIO pair
#define BIO_TYPE_LINEBUFFER (20 | 0x0200)  // filter
#define BIO_TYPE_DGRAM (21 | 0x0400 | 0x0100)
#define BIO_TYPE_ASN1 (22 | 0x0200)  // filter
#define BIO_TYPE_COMP (23 | 0x0200)  // filter

// (|BIO_set_fd|) and |BIO_C_GET_FD| (|BIO_get_fd|) control hooks.
#define BIO_TYPE_DESCRIPTOR 0x0100  // socket, fd, connect or accept
#define BIO_TYPE_FILTER 0x0200
#define BIO_TYPE_SOURCE_SINK 0x0400

// flag bits aside, may exceed this value.
#define BIO_TYPE_START 128

struct bio_method_st {
  int type;
  const char *name;
  int (*bwrite)(BIO *, const char *, int);
  int (*bread)(BIO *, char *, int);

  int (*bputs)(BIO *, const char *);
  int (*bgets)(BIO *, char *, int);
  long (*ctrl)(BIO *, int, long, void *);
  int (*create)(BIO *);
  int (*destroy)(BIO *);
  long (*callback_ctrl)(BIO *, int, bio_info_cb);
};

struct bio_st {
  const BIO_METHOD *method;

  int init;




  int shutdown;
  int flags;
  int retry_reason;


  int num;
  CRYPTO_refcount_t references;
  void *ptr;


  BIO *next_bio;  // used by filter BIOs
  size_t num_read, num_write;
};

#define BIO_C_SET_CONNECT 100
#define BIO_C_DO_STATE_MACHINE 101
#define BIO_C_SET_NBIO 102
#define BIO_C_SET_PROXY_PARAM 103
#define BIO_C_SET_FD 104
#define BIO_C_GET_FD 105
#define BIO_C_SET_FILE_PTR 106
#define BIO_C_GET_FILE_PTR 107
#define BIO_C_SET_FILENAME 108
#define BIO_C_SET_SSL 109
#define BIO_C_GET_SSL 110
#define BIO_C_SET_MD 111
#define BIO_C_GET_MD 112
#define BIO_C_GET_CIPHER_STATUS 113
#define BIO_C_SET_BUF_MEM 114
#define BIO_C_GET_BUF_MEM_PTR 115
#define BIO_C_GET_BUFF_NUM_LINES 116
#define BIO_C_SET_BUFF_SIZE 117
#define BIO_C_SET_ACCEPT 118
#define BIO_C_SSL_MODE 119
#define BIO_C_GET_MD_CTX 120
#define BIO_C_GET_PROXY_PARAM 121
#define BIO_C_SET_BUFF_READ_DATA 122  // data to read first
#define BIO_C_GET_ACCEPT 124
#define BIO_C_SET_SSL_RENEGOTIATE_BYTES 125
#define BIO_C_GET_SSL_NUM_RENEGOTIATES 126
#define BIO_C_SET_SSL_RENEGOTIATE_TIMEOUT 127
#define BIO_C_FILE_SEEK 128
#define BIO_C_GET_CIPHER_CTX 129
#define BIO_C_SET_BUF_MEM_EOF_RETURN 130  // return end of input value
#define BIO_C_SET_BIND_MODE 131
#define BIO_C_GET_BIND_MODE 132
#define BIO_C_FILE_TELL 133
#define BIO_C_GET_SOCKS 134
#define BIO_C_SET_SOCKS 135

#define BIO_C_SET_WRITE_BUF_SIZE 136  // for BIO_s_bio
#define BIO_C_GET_WRITE_BUF_SIZE 137
#define BIO_C_GET_WRITE_GUARANTEE 140
#define BIO_C_GET_READ_REQUEST 141
#define BIO_C_SHUTDOWN_WR 142
#define BIO_C_NREAD0 143
#define BIO_C_NREAD 144
#define BIO_C_NWRITE0 145
#define BIO_C_NWRITE 146
#define BIO_C_RESET_READ_REQUEST 147
#define BIO_C_SET_MD_CTX 148

#define BIO_C_SET_PREFIX 149
#define BIO_C_GET_PREFIX 150
#define BIO_C_SET_SUFFIX 151
#define BIO_C_GET_SUFFIX 152

#define BIO_C_SET_EX_ARG 153
#define BIO_C_GET_EX_ARG 154


#if defined(__cplusplus)
}  // extern C

extern "C++" {

BSSL_NAMESPACE_BEGIN

BORINGSSL_MAKE_DELETER(BIO, BIO_free)
BORINGSSL_MAKE_UP_REF(BIO, BIO_up_ref)
BORINGSSL_MAKE_DELETER(BIO_METHOD, BIO_meth_free)

BSSL_NAMESPACE_END

}  // extern C++

#endif

#define BIO_R_BAD_FOPEN_MODE 100
#define BIO_R_BROKEN_PIPE 101
#define BIO_R_CONNECT_ERROR 102
#define BIO_R_ERROR_SETTING_NBIO 103
#define BIO_R_INVALID_ARGUMENT 104
#define BIO_R_IN_USE 105
#define BIO_R_KEEPALIVE 106
#define BIO_R_NBIO_CONNECT_ERROR 107
#define BIO_R_NO_HOSTNAME_SPECIFIED 108
#define BIO_R_NO_PORT_SPECIFIED 109
#define BIO_R_NO_SUCH_FILE 110
#define BIO_R_NULL_PARAMETER 111
#define BIO_R_SYS_LIB 112
#define BIO_R_UNABLE_TO_CREATE_SOCKET 113
#define BIO_R_UNINITIALIZED 114
#define BIO_R_UNSUPPORTED_METHOD 115
#define BIO_R_WRITE_TO_READ_ONLY_BIO 116

#endif  // OPENSSL_HEADER_BIO_H
