// Copyright 2012 The Go Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

// the elliptic curve known as curve25519. See http://cr.yp.to/ecdh.html
package curve25519 // import "golang.org/x/crypto/curve25519"

var basePoint = [32]byte{9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}

// coordinates of group points and all values are in little-endian form.
func ScalarMult(dst, in, base *[32]byte) {
	scalarMult(dst, in, base)
}

// coordinates of group points, base is the standard generator and all values
// are in little-endian form.
func ScalarBaseMult(dst, in *[32]byte) {
	ScalarMult(dst, in, &basePoint)
}
