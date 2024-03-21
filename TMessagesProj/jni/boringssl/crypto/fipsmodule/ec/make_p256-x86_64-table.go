/* Copyright (c) 2018, Google Inc.
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

package main

import (
	"crypto/elliptic"
	"fmt"
	"math/big"
	"os"
)

const fileHeader = `/* Copyright (c) 2015, Intel Inc.
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

// p256-x86_64.c, for the default generator. The table consists of 37
// subtables, each subtable contains 64 affine points. The affine points are
// encoded as eight uint64's, four for the x coordinate and four for the y.
// Both values are in little-endian order. There are 37 tables because a
// signed, 6-bit wNAF form of the scalar is used and ceil(256/(6 + 1)) = 37.
// Within each table there are 64 values because the 6-bit wNAF value can take
// 64 values, ignoring the sign bit, which is implemented by performing a
// negation of the affine point when required. We would like to align it to 2MB
// in order to increase the chances of using a large page but that appears to
// lead to invalid ELF files being produced.


static const alignas(4096) PRECOMP256_ROW ecp_nistz256_precomputed[37] = {
`

func main() {
	os.Stdout.WriteString(fileHeader)

	scalar, tmp := new(big.Int), new(big.Int)
	p256 := elliptic.P256()
	p := p256.Params().P


	for shift := uint(0); shift < 256; shift += 7 {

		for multiple := 1; multiple <= 64; multiple++ {
			scalar.SetInt64(int64(multiple))
			scalar.Lsh(scalar, shift)

			x, y := p256.ScalarBaseMult(scalar.Bytes())

			toMontgomery(x, p)
			toMontgomery(y, p)

			if multiple == 1 {
				os.Stdout.WriteString("        {{")
			} else {
				os.Stdout.WriteString("         {")
			}
			printNum(x, tmp)

			os.Stdout.WriteString(",\n          ")
			printNum(y, tmp)

			if multiple == 64 {
				os.Stdout.WriteString("}}")
			} else {
				os.Stdout.WriteString("},\n")
			}
		}

		if shift+7 < 256 {
			os.Stdout.WriteString(",\n")
		} else {
			os.Stdout.WriteString("};\n")
		}
	}
}

var mask, R *big.Int

func init() {
	mask = new(big.Int).SetUint64(0xffffffffffffffff)
	R = new(big.Int).SetInt64(1)
	R.Lsh(R, 256)
}

func printNum(n, tmp *big.Int) {
	fmt.Printf("{")
	for i := 0; i < 4; i++ {
		tmp.And(n, mask)
		limb := tmp.Uint64()
		fmt.Printf("TOBN(0x%08x, 0x%08x)", uint32(limb>>32), uint32(limb))
		n.Rsh(n, 64)

		switch i {
		case 0, 2:
			os.Stdout.WriteString(", ")
		case 1:
			os.Stdout.WriteString(",\n           ")
		}
	}
	fmt.Printf("}")
}

func toMontgomery(n, p *big.Int) {
	n.Mul(n, R)
	n.Mod(n, p)
}
