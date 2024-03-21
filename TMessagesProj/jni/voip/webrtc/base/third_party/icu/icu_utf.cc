// © 2016 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html
/*
******************************************************************************
*
*   Copyright (C) 1999-2012, International Business Machines
*   Corporation and others.  All Rights Reserved.
*
******************************************************************************
*   file name:  utf_impl.cpp
*   encoding:   UTF-8
*   tab size:   8 (not used)
*   indentation:4
*
*   created on: 1999sep13
*   created by: Markus W. Scherer
*
*   This file provides implementation functions for macros in the utfXX.h
*   that would otherwise be too long as macros.
*/

#include "base/third_party/icu/icu_utf.h"

namespace base_icu {


static const UChar32
utf8_errorValue[6]={


    0x15, 0x9f, 0xffff,
    0x10ffff
};

static UChar32
errorValue(int32_t count, int8_t strict) {
    if(strict>=0) {
        return utf8_errorValue[count];
    } else if(strict==-3) {
        return 0xfffd;
    } else {
        return CBU_SENTINEL;
    }
}

/*
 * Handle the non-inline part of the U8_NEXT() and U8_NEXT_FFFD() macros
 * and their obsolete sibling UTF8_NEXT_CHAR_SAFE().
 *
 * U8_NEXT() supports NUL-terminated strings indicated via length<0.
 *
 * The "strict" parameter controls the error behavior:
 * <0  "Safe" behavior of U8_NEXT():
 *     -1: All illegal byte sequences yield U_SENTINEL=-1.
 *     -2: Same as -1, except for lenient treatment of surrogate code points as legal.
 *         Some implementations use this for roundtripping of
 *         Unicode 16-bit strings that are not well-formed UTF-16, that is, they
 *         contain unpaired surrogates.
 *     -3: All illegal byte sequences yield U+FFFD.
 *  0  Obsolete "safe" behavior of UTF8_NEXT_CHAR_SAFE(..., FALSE):
 *     All illegal byte sequences yield a positive code point such that this
 *     result code point would be encoded with the same number of bytes as
 *     the illegal sequence.
 * >0  Obsolete "strict" behavior of UTF8_NEXT_CHAR_SAFE(..., TRUE):
 *     Same as the obsolete "safe" behavior, but non-characters are also treated
 *     like illegal sequences.
 *
 * Note that a UBool is the same as an int8_t.
 */
UChar32
utf8_nextCharSafeBody(const uint8_t *s, int32_t *pi, int32_t length, UChar32 c, UBool strict) {

    int32_t i=*pi;

    if(i==length || c>0xf4) {

    } else if(c>=0xf0) {


        uint8_t t1=s[i], t2, t3;
        c&=7;
        if(CBU8_IS_VALID_LEAD4_AND_T1(c, t1) &&
                ++i!=length && (t2=s[i]-0x80)<=0x3f &&
                ++i!=length && (t3=s[i]-0x80)<=0x3f) {
            ++i;
            c=(c<<18)|((t1&0x3f)<<12)|(t2<<6)|t3;

            if(strict<=0 || !CBU_IS_UNICODE_NONCHAR(c)) {
                *pi=i;
                return c;
            }
        }
    } else if(c>=0xe0) {
        c&=0xf;
        if(strict!=-2) {
            uint8_t t1=s[i], t2;
            if(CBU8_IS_VALID_LEAD3_AND_T1(c, t1) &&
                    ++i!=length && (t2=s[i]-0x80)<=0x3f) {
                ++i;
                c=(c<<12)|((t1&0x3f)<<6)|t2;

                if(strict<=0 || !CBU_IS_UNICODE_NONCHAR(c)) {
                    *pi=i;
                    return c;
                }
            }
        } else {

            uint8_t t1=s[i]-0x80, t2;
            if(t1<=0x3f && (c>0 || t1>=0x20) &&
                    ++i!=length && (t2=s[i]-0x80)<=0x3f) {
                *pi=i+1;
                return (c<<12)|(t1<<6)|t2;
            }
        }
    } else if(c>=0xc2) {
        uint8_t t1=s[i]-0x80;
        if(t1<=0x3f) {
            *pi=i+1;
            return ((c-0xc0)<<6)|t1;
        }
    }  // else 0x80<=c<0xc2 is not a lead byte

    /* error handling */
    c=errorValue(i-*pi, strict);
    *pi=i;
    return c;
}

}  // namespace base_icu
