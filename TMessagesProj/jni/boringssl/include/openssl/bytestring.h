/* Copyright (c) 2014, Google Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. */

#ifndef OPENSSL_HEADER_BYTESTRING_H
#define OPENSSL_HEADER_BYTESTRING_H

#include <openssl/base.h>

#include <openssl/span.h>

#if defined(__cplusplus)
extern "C" {
#endif

//
// A "CBS" (CRYPTO ByteString) represents a string of bytes in memory and
// provides utility functions for safely parsing length-prefixed structures
// like TLS and ASN.1 from it.
//
// A "CBB" (CRYPTO ByteBuilder) is a memory buffer that grows as needed and
// provides utility functions for building length-prefixed messages.


struct cbs_st {
  const uint8_t *data;
  size_t len;

#if !defined(BORINGSSL_NO_CXX)

  cbs_st(bssl::Span<const uint8_t> span)
      : data(span.data()), len(span.size()) {}
  operator bssl::Span<const uint8_t>() const {
    return bssl::MakeConstSpan(data, len);
  }

  cbs_st() = default;
  cbs_st(const cbs_st &) = default;
#endif
};

// |data|.
OPENSSL_EXPORT void CBS_init(CBS *cbs, const uint8_t *data, size_t len);

// otherwise.
OPENSSL_EXPORT int CBS_skip(CBS *cbs, size_t len);

OPENSSL_EXPORT const uint8_t *CBS_data(const CBS *cbs);

OPENSSL_EXPORT size_t CBS_len(const CBS *cbs);

// |*out_len|. If |*out_ptr| is not NULL, the contents are freed with
// OPENSSL_free. It returns one on success and zero on allocation failure. On
// success, |*out_ptr| should be freed with OPENSSL_free. If |cbs| is empty,
// |*out_ptr| will be NULL.
OPENSSL_EXPORT int CBS_stow(const CBS *cbs, uint8_t **out_ptr, size_t *out_len);

// NUL-terminated C string. If |*out_ptr| is not NULL, the contents are freed
// with OPENSSL_free. It returns one on success and zero on allocation
// failure. On success, |*out_ptr| should be freed with OPENSSL_free.
//
// NOTE: If |cbs| contains NUL bytes, the string will be truncated. Call
// |CBS_contains_zero_byte(cbs)| to check for NUL bytes.
OPENSSL_EXPORT int CBS_strdup(const CBS *cbs, char **out_ptr);

// a NUL byte and zero otherwise.
OPENSSL_EXPORT int CBS_contains_zero_byte(const CBS *cbs);

// starting at |data|. If they're equal, it returns one, otherwise zero. If the
// lengths match, it uses a constant-time comparison.
OPENSSL_EXPORT int CBS_mem_equal(const CBS *cbs, const uint8_t *data,
                                 size_t len);

// returns one on success and zero on error.
OPENSSL_EXPORT int CBS_get_u8(CBS *cbs, uint8_t *out);

// advances |cbs|. It returns one on success and zero on error.
OPENSSL_EXPORT int CBS_get_u16(CBS *cbs, uint16_t *out);

// advances |cbs|. It returns one on success and zero on error.
OPENSSL_EXPORT int CBS_get_u24(CBS *cbs, uint32_t *out);

// and advances |cbs|. It returns one on success and zero on error.
OPENSSL_EXPORT int CBS_get_u32(CBS *cbs, uint32_t *out);

// and advances |cbs|. It returns one on success and zero on error.
OPENSSL_EXPORT int CBS_get_u64(CBS *cbs, uint64_t *out);

// |cbs|. It returns one on success and zero on error.
OPENSSL_EXPORT int CBS_get_last_u8(CBS *cbs, uint8_t *out);

// |cbs|. It returns one on success and zero on error.
OPENSSL_EXPORT int CBS_get_bytes(CBS *cbs, CBS *out, size_t len);

// |cbs|. It returns one on success and zero on error.
OPENSSL_EXPORT int CBS_copy_bytes(CBS *cbs, uint8_t *out, size_t len);

// length-prefixed value from |cbs| and advances |cbs| over it. It returns one
// on success and zero on error.
OPENSSL_EXPORT int CBS_get_u8_length_prefixed(CBS *cbs, CBS *out);

// big-endian, length-prefixed value from |cbs| and advances |cbs| over it. It
// returns one on success and zero on error.
OPENSSL_EXPORT int CBS_get_u16_length_prefixed(CBS *cbs, CBS *out);

// big-endian, length-prefixed value from |cbs| and advances |cbs| over it. It
// returns one on success and zero on error.
OPENSSL_EXPORT int CBS_get_u24_length_prefixed(CBS *cbs, CBS *out);

//
// |CBS| may be used to parse DER structures. Rather than using a schema
// compiler, the following functions act on tag-length-value elements in the
// serialization itself. Thus the caller is responsible for looping over a
// SEQUENCE, branching on CHOICEs or OPTIONAL fields, checking for trailing
// data, and handling explict vs. implicit tagging.
//
// Tags are represented as |unsigned| values in memory. The upper few bits store
// the class and constructed bit, and the remaining bits store the tag
// number. Note this differs from the DER serialization, to support tag numbers
// beyond 31. Consumers must use the constants defined below to decompose or
// assemble tags.
//
// This library treats an element's constructed bit as part of its tag. In DER,
// the constructed bit is computable from the type. The constants for universal
// types have the bit set. Callers must set it correctly for tagged types.
// Explicitly-tagged types are always constructed, and implicitly-tagged types
// inherit the underlying type's bit.

// and constructed bits from the DER serialization.
#define CBS_ASN1_TAG_SHIFT 24

#define CBS_ASN1_CONSTRUCTED (0x20u << CBS_ASN1_TAG_SHIFT)

// to produce the final tag. If none is used, the tag will be UNIVERSAL.
#define CBS_ASN1_UNIVERSAL (0u << CBS_ASN1_TAG_SHIFT)
#define CBS_ASN1_APPLICATION (0x40u << CBS_ASN1_TAG_SHIFT)
#define CBS_ASN1_CONTEXT_SPECIFIC (0x80u << CBS_ASN1_TAG_SHIFT)
#define CBS_ASN1_PRIVATE (0xc0u << CBS_ASN1_TAG_SHIFT)

// give one of the four values above.
#define CBS_ASN1_CLASS_MASK (0xc0u << CBS_ASN1_TAG_SHIFT)

#define CBS_ASN1_TAG_NUMBER_MASK ((1u << (5 + CBS_ASN1_TAG_SHIFT)) - 1)

// include the constructed bit.
#define CBS_ASN1_BOOLEAN 0x1u
#define CBS_ASN1_INTEGER 0x2u
#define CBS_ASN1_BITSTRING 0x3u
#define CBS_ASN1_OCTETSTRING 0x4u
#define CBS_ASN1_NULL 0x5u
#define CBS_ASN1_OBJECT 0x6u
#define CBS_ASN1_ENUMERATED 0xau
#define CBS_ASN1_UTF8STRING 0xcu
#define CBS_ASN1_SEQUENCE (0x10u | CBS_ASN1_CONSTRUCTED)
#define CBS_ASN1_SET (0x11u | CBS_ASN1_CONSTRUCTED)
#define CBS_ASN1_NUMERICSTRING 0x12u
#define CBS_ASN1_PRINTABLESTRING 0x13u
#define CBS_ASN1_T61STRING 0x14u
#define CBS_ASN1_VIDEOTEXSTRING 0x15u
#define CBS_ASN1_IA5STRING 0x16u
#define CBS_ASN1_UTCTIME 0x17u
#define CBS_ASN1_GENERALIZEDTIME 0x18u
#define CBS_ASN1_GRAPHICSTRING 0x19u
#define CBS_ASN1_VISIBLESTRING 0x1au
#define CBS_ASN1_GENERALSTRING 0x1bu
#define CBS_ASN1_UNIVERSALSTRING 0x1cu
#define CBS_ASN1_BMPSTRING 0x1eu

// including tag and length bytes) and advances |cbs| over it. The ASN.1
// element must match |tag_value|. It returns one on success and zero
// on error.
OPENSSL_EXPORT int CBS_get_asn1(CBS *cbs, CBS *out, unsigned tag_value);

// ASN.1 header bytes too.
OPENSSL_EXPORT int CBS_get_asn1_element(CBS *cbs, CBS *out, unsigned tag_value);

// if the next ASN.1 element on |cbs| would have tag |tag_value|. If
// |cbs| is empty or the tag does not match, it returns zero. Note: if
// it returns one, CBS_get_asn1 may still fail if the rest of the
// element is malformed.
OPENSSL_EXPORT int CBS_peek_asn1_tag(const CBS *cbs, unsigned tag_value);

// (not including tag and length bytes), sets |*out_tag| to the tag number, and
// advances |*cbs|. It returns one on success and zero on error. Either of |out|
// and |out_tag| may be NULL to ignore the value.
OPENSSL_EXPORT int CBS_get_any_asn1(CBS *cbs, CBS *out, unsigned *out_tag);

// |*cbs| (including header bytes) and advances |*cbs|. It sets |*out_tag| to
// the tag number and |*out_header_len| to the length of the ASN.1 header. Each
// of |out|, |out_tag|, and |out_header_len| may be NULL to ignore the value.
OPENSSL_EXPORT int CBS_get_any_asn1_element(CBS *cbs, CBS *out,
                                            unsigned *out_tag,
                                            size_t *out_header_len);

// also allows indefinite-length elements to be returned. In that case,
// |*out_header_len| and |CBS_len(out)| will both be two as only the header is
// returned, otherwise it behaves the same as the previous function.
OPENSSL_EXPORT int CBS_get_any_ber_asn1_element(CBS *cbs, CBS *out,
                                                unsigned *out_tag,
                                                size_t *out_header_len);

// and sets |*out| to its value. It returns one on success and zero on error,
// where error includes the integer being negative, or too large to represent
// in 64 bits.
OPENSSL_EXPORT int CBS_get_asn1_uint64(CBS *cbs, uint64_t *out);

// or one based on its value. It returns one on success or zero on error.
OPENSSL_EXPORT int CBS_get_asn1_bool(CBS *cbs, int *out);

// tagged with |tag| and sets |*out| to its contents, or ignores it if |out| is
// NULL. If present and if |out_present| is not NULL, it sets |*out_present| to
// one, otherwise zero. It returns one on success, whether or not the element
// was present, and zero on decode failure.
OPENSSL_EXPORT int CBS_get_optional_asn1(CBS *cbs, CBS *out, int *out_present,
                                         unsigned tag);

// explicitly-tagged OCTET STRING from |cbs|. If present, it sets
// |*out| to the string and |*out_present| to one. Otherwise, it sets
// |*out| to empty and |*out_present| to zero. |out_present| may be
// NULL. It returns one on success, whether or not the element was
// present, and zero on decode failure.
OPENSSL_EXPORT int CBS_get_optional_asn1_octet_string(CBS *cbs, CBS *out,
                                                      int *out_present,
                                                      unsigned tag);

// INTEGER from |cbs|. If present, it sets |*out| to the
// value. Otherwise, it sets |*out| to |default_value|. It returns one
// on success, whether or not the element was present, and zero on
// decode failure.
OPENSSL_EXPORT int CBS_get_optional_asn1_uint64(CBS *cbs, uint64_t *out,
                                                unsigned tag,
                                                uint64_t default_value);

// |cbs|. If present, it sets |*out| to either zero or one, based on the
// boolean. Otherwise, it sets |*out| to |default_value|. It returns one on
// success, whether or not the element was present, and zero on decode
// failure.
OPENSSL_EXPORT int CBS_get_optional_asn1_bool(CBS *cbs, int *out, unsigned tag,
                                              int default_value);

// and zero otherwise.
OPENSSL_EXPORT int CBS_is_valid_asn1_bitstring(const CBS *cbs);

// and the specified bit is present and set. Otherwise, it returns zero. |bit|
// is indexed starting from zero.
OPENSSL_EXPORT int CBS_asn1_bitstring_has_bit(const CBS *cbs, unsigned bit);

// contents (not including the element framing) and returns the ASCII
// representation (e.g., "1.2.840.113554.4.1.72585") in a newly-allocated
// string, or NULL on failure. The caller must release the result with
// |OPENSSL_free|.
OPENSSL_EXPORT char *CBS_asn1_oid_to_text(const CBS *cbs);

//
// |CBB| objects allow one to build length-prefixed serialisations. A |CBB|
// object is associated with a buffer and new buffers are created with
// |CBB_init|. Several |CBB| objects can point at the same buffer when a
// length-prefix is pending, however only a single |CBB| can be 'current' at
// any one time. For example, if one calls |CBB_add_u8_length_prefixed| then
// the new |CBB| points at the same buffer as the original. But if the original
// |CBB| is used then the length prefix is written out and the new |CBB| must
// not be used again.
//
// If one needs to force a length prefix to be written out because a |CBB| is
// going out of scope, use |CBB_flush|. If an operation on a |CBB| fails, it is
// in an undefined state and must not be used except to call |CBB_cleanup|.

struct cbb_buffer_st {
  uint8_t *buf;
  size_t len;      // The number of valid bytes.
  size_t cap;      // The size of buf.
  char can_resize; /* One iff |buf| is owned by this object. If not then |buf|
                      cannot be resized. */
  char error;      /* One iff there was an error writing to this CBB. All future
                      operations will fail. */
};

struct cbb_st {
  struct cbb_buffer_st *base;

  CBB *child;


  size_t offset;


  uint8_t pending_len_len;
  char pending_is_asn1;


  char is_child;
};

// initialised with |CBB_init| or |CBB_init_fixed| before use, but it is safe to
// call |CBB_cleanup| without a successful |CBB_init|. This may be used for more
// uniform cleanup of a |CBB|.
OPENSSL_EXPORT void CBB_zero(CBB *cbb);

// needed, the |initial_capacity| is just a hint. It returns one on success or
// zero on allocation failure.
OPENSSL_EXPORT int CBB_init(CBB *cbb, size_t initial_capacity);

// |buf| cannot grow, trying to write more than |len| bytes will cause CBB
// functions to fail. It returns one on success or zero on error.
OPENSSL_EXPORT int CBB_init_fixed(CBB *cbb, uint8_t *buf, size_t len);

// writing to the same buffer. This should be used in an error case where a
// serialisation is abandoned.
//
// This function can only be called on a "top level" |CBB|, i.e. one initialised
// with |CBB_init| or |CBB_init_fixed|, or a |CBB| set to the zero state with
// |CBB_zero|.
OPENSSL_EXPORT void CBB_cleanup(CBB *cbb);

// malloced buffer and |*out_len| to the length of that buffer. The caller
// takes ownership of the buffer and, unless the buffer was fixed with
// |CBB_init_fixed|, must call |OPENSSL_free| when done.
//
// It can only be called on a "top level" |CBB|, i.e. one initialised with
// |CBB_init| or |CBB_init_fixed|. It returns one on success and zero on
// error.
OPENSSL_EXPORT int CBB_finish(CBB *cbb, uint8_t **out_data, size_t *out_len);

// |CBB| objects of |cbb| to be invalidated. This allows |cbb| to continue to be
// used after the children go out of scope, e.g. when local |CBB| objects are
// added as children to a |CBB| that persists after a function returns. This
// function returns one on success or zero on error.
OPENSSL_EXPORT int CBB_flush(CBB *cbb);

// |cbb|. The pointer is valid until the next operation to |cbb|.
//
// To avoid unfinalized length prefixes, it is a fatal error to call this on a
// CBB with any active children.
OPENSSL_EXPORT const uint8_t *CBB_data(const CBB *cbb);

// |cbb|.
//
// To avoid unfinalized length prefixes, it is a fatal error to call this on a
// CBB with any active children.
OPENSSL_EXPORT size_t CBB_len(const CBB *cbb);

// data written to |*out_contents| will be prefixed in |cbb| with an 8-bit
// length. It returns one on success or zero on error.
OPENSSL_EXPORT int CBB_add_u8_length_prefixed(CBB *cbb, CBB *out_contents);

// The data written to |*out_contents| will be prefixed in |cbb| with a 16-bit,
// big-endian length. It returns one on success or zero on error.
OPENSSL_EXPORT int CBB_add_u16_length_prefixed(CBB *cbb, CBB *out_contents);

// The data written to |*out_contents| will be prefixed in |cbb| with a 24-bit,
// big-endian length. It returns one on success or zero on error.
OPENSSL_EXPORT int CBB_add_u24_length_prefixed(CBB *cbb, CBB *out_contents);

// ASN.1 object can be written. The |tag| argument will be used as the tag for
// the object. It returns one on success or zero on error.
OPENSSL_EXPORT int CBB_add_asn1(CBB *cbb, CBB *out_contents, unsigned tag);

// success and zero otherwise.
OPENSSL_EXPORT int CBB_add_bytes(CBB *cbb, const uint8_t *data, size_t len);

// the beginning of that space. The caller must then write |len| bytes of
// actual contents to |*out_data|. It returns one on success and zero
// otherwise.
OPENSSL_EXPORT int CBB_add_space(CBB *cbb, uint8_t **out_data, size_t len);

// |*out_data| to point to the beginning of that space. It returns one on
// success and zero otherwise. The caller may write up to |len| bytes to
// |*out_data| and call |CBB_did_write| to complete the write. |*out_data| is
// valid until the next operation on |cbb| or an ancestor |CBB|.
OPENSSL_EXPORT int CBB_reserve(CBB *cbb, uint8_t **out_data, size_t len);

// written to by the caller. It returns one on success and zero on error.
OPENSSL_EXPORT int CBB_did_write(CBB *cbb, size_t len);

// success and zero otherwise.
OPENSSL_EXPORT int CBB_add_u8(CBB *cbb, uint8_t value);

// returns one on success and zero otherwise.
OPENSSL_EXPORT int CBB_add_u16(CBB *cbb, uint16_t value);

// returns one on success and zero otherwise.
OPENSSL_EXPORT int CBB_add_u24(CBB *cbb, uint32_t value);

// returns one on success and zero otherwise.
OPENSSL_EXPORT int CBB_add_u32(CBB *cbb, uint32_t value);

// returns one on success and zero otherwise.
OPENSSL_EXPORT int CBB_add_u64(CBB *cbb, uint64_t value);

// child's contents nor the length prefix will be included in the output.
OPENSSL_EXPORT void CBB_discard_child(CBB *cbb);

// and writes |value| in its contents. It returns one on success and zero on
// error.
OPENSSL_EXPORT int CBB_add_asn1_uint64(CBB *cbb, uint64_t value);

// given contents. It returns one on success and zero on error.
OPENSSL_EXPORT int CBB_add_asn1_octet_string(CBB *cbb, const uint8_t *data,
                                             size_t data_len);

// |value| is non-zero.  It returns one on success and zero on error.
OPENSSL_EXPORT int CBB_add_asn1_bool(CBB *cbb, int value);

// representation, e.g. "1.2.840.113554.4.1.72585", and writes the DER-encoded
// contents to |cbb|. It returns one on success and zero on malloc failure or if
// |text| was invalid. It does not include the OBJECT IDENTIFER framing, only
// the element's contents.
//
// This function considers OID strings with components which do not fit in a
// |uint64_t| to be invalid.
OPENSSL_EXPORT int CBB_add_asn1_oid_from_text(CBB *cbb, const char *text,
                                              size_t len);

// contents for a DER-encoded ASN.1 SET OF type. It returns one on success and
// zero on failure. DER canonicalizes SET OF contents by sorting
// lexicographically by encoding. Call this function when encoding a SET OF
// type in an order that is not already known to be canonical.
//
// Note a SET type has a slightly different ordering than a SET OF.
OPENSSL_EXPORT int CBB_flush_asn1_set_of(CBB *cbb);


#if defined(__cplusplus)
}  // extern C


#if !defined(BORINGSSL_NO_CXX)
extern "C++" {

BSSL_NAMESPACE_BEGIN

using ScopedCBB = internal::StackAllocated<CBB, void, CBB_zero, CBB_cleanup>;

BSSL_NAMESPACE_END

}  // extern C++
#endif

#endif

#endif  // OPENSSL_HEADER_BYTESTRING_H
