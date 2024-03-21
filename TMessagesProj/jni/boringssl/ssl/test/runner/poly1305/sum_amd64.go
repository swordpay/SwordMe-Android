// Copyright 2012 The Go Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.


package poly1305

//go:noescape
func poly1305(out *[16]byte, m *byte, mlen uint64, key *[32]byte)

// 16-byte result into out. Authenticating two different messages with the same
// key allows an attacker to forge messages at will.
func Sum(out *[16]byte, m []byte, key *[32]byte) {
	var mPtr *byte
	if len(m) > 0 {
		mPtr = &m[0]
	}
	poly1305(out, mPtr, uint64(len(m)), key)
}
