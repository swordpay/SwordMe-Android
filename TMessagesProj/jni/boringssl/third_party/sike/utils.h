/********************************************************************************************
* SIDH: an efficient supersingular isogeny cryptography library
*
* Abstract: internal header file for P434
*********************************************************************************************/

#ifndef UTILS_H_
#define UTILS_H_

#include <openssl/base.h>

#include "../crypto/internal.h"
#include "sike.h"

#define BITS_TO_BYTES(nbits)      (((nbits)+7)/8)

#define BITS_FIELD              434
// Byte size of the field
#define FIELD_BYTESZ            BITS_TO_BYTES(BITS_FIELD)
// Number of 64-bit words of a 224-bit element
#define NBITS_ORDER             224
#define NWORDS64_ORDER          ((NBITS_ORDER+63)/64)
// Number of elements in Alice's strategy
#define A_max                   108
// Number of elements in Bob's strategy
#define B_max                   137
// Word size size
#define RADIX                   sizeof(crypto_word_t)*8
// Byte size of a limb
#define LSZ                     sizeof(crypto_word_t)

#if defined(OPENSSL_64_BIT)

    #define NWORDS_FIELD    7

    #define ZERO_WORDS 3

    #define U64_TO_WORDS(x) UINT64_C(x)
#else

    #define NWORDS_FIELD    14

    #define ZERO_WORDS 6

    #define U64_TO_WORDS(x) \
        (uint32_t)(UINT64_C(x) & 0xffffffff), (uint32_t)(UINT64_C(x) >> 32)
#endif

#if !defined(BORINGSSL_HAS_UINT128)
    typedef uint64_t uint128_t[2];
#endif

// Digit multiplication
#define MUL(multiplier, multiplicand, hi, lo) digit_x_digit((multiplier), (multiplicand), &(lo));

#define M2B(x) ((x)>>(RADIX-1))

#define ADDC(carryIn, addend1, addend2, carryOut, sumOut)                   \
do {                                                                        \
  crypto_word_t tempReg = (addend1) + (crypto_word_t)(carryIn);             \
  (sumOut) = (addend2) + tempReg;                                           \
  (carryOut) = M2B(constant_time_lt_w(tempReg, (crypto_word_t)(carryIn)) |  \
                   constant_time_lt_w((sumOut), tempReg));                  \
} while(0)

#define SUBC(borrowIn, minuend, subtrahend, borrowOut, differenceOut)           \
do {                                                                            \
    crypto_word_t tempReg = (minuend) - (subtrahend);                           \
    crypto_word_t borrowReg = M2B(constant_time_lt_w((minuend), (subtrahend))); \
    borrowReg |= ((borrowIn) & constant_time_is_zero_w(tempReg));               \
    (differenceOut) = tempReg - (crypto_word_t)(borrowIn);                      \
    (borrowOut) = borrowReg;                                                    \
} while(0)

/* Old GCC 4.9 (jessie) doesn't implement {0} initialization properly,
   which violates C11 as described in 6.7.9, 21 (similarily C99, 6.7.8).
   Defines below are used to work around the bug, and provide a way
   to initialize f2elem_t and point_proj_t structs.
   Bug has been fixed in GCC6 (debian stretch).
*/
#define F2ELM_INIT {{ {0}, {0} }}
#define POINT_PROJ_INIT {{ F2ELM_INIT, F2ELM_INIT }}

// Elements over GF(p434) are encoded in 63 octets in little endian format
// (i.e., the least significant octet is located in the lowest memory address).
typedef crypto_word_t felm_t[NWORDS_FIELD];

// Fp2 element = c0 + c1*i in F_{p^2}
// Datatype for representing double-precision 2x434-bit field elements (448-bit max.)
// Elements (a+b*i) over GF(p434^2), where a and b are defined over GF(p434), are
// encoded as {a, b}, with a in the lowest memory portion.
typedef struct {
    felm_t c0;
    felm_t c1;
} fp2;

typedef fp2 f2elm_t[1];

// field elements in contiguous memory.
typedef crypto_word_t dfelm_t[2*NWORDS_FIELD];

struct params_t {

    const crypto_word_t prime[NWORDS_FIELD];

    const crypto_word_t prime_p1[NWORDS_FIELD];

    const crypto_word_t prime_x2[NWORDS_FIELD];


    const crypto_word_t A_gen[6*NWORDS_FIELD];


    const crypto_word_t B_gen[6*NWORDS_FIELD];

    const crypto_word_t mont_R2[NWORDS_FIELD];

    const crypto_word_t mont_one[NWORDS_FIELD];

    const crypto_word_t mont_six[NWORDS_FIELD];

    const unsigned int A_strat[A_max-1];
    const unsigned int B_strat[B_max-1];
};

typedef struct {
    f2elm_t X;
    f2elm_t Z;
} point_proj;
typedef point_proj point_proj_t[1];

#endif // UTILS_H_
