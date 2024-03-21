// Copyright (c) 2019, Cloudflare Inc.
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
// OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
// CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

package sike

import (
	"crypto/sha256"
	"crypto/subtle"
	"errors"
	"io"
)

func zeroize(fp *Fp2) {


	for i := range fp.A {
		fp.A[i] = 0
	}
	for i := range fp.B {
		fp.B[i] = 0
	}
}

//
// The output byte slice must be at least 2*bytelen(p) bytes long.
func convFp2ToBytes(output []byte, fp2 *Fp2) {
	if len(output) < 2*Params.Bytelen {
		panic("output byte slice too short")
	}
	var a Fp2
	fromMontDomain(fp2, &a)

	for i := 0; i < Params.Bytelen; i++ {

		tmp := i / 8
		k := uint64(i % 8)
		output[i] = byte(a.A[tmp] >> (8 * k))
		output[i+Params.Bytelen] = byte(a.B[tmp] >> (8 * k))
	}
}

//
// It is an error to call this function if the input byte slice is less than 2*bytelen(p) bytes long.
func convBytesToFp2(fp2 *Fp2, input []byte) {
	if len(input) < 2*Params.Bytelen {
		panic("input byte slice too short")
	}

	for i := 0; i < Params.Bytelen; i++ {
		j := i / 8
		k := uint64(i % 8)
		fp2.A[j] |= uint64(input[i]) << (8 * k)
		fp2.B[j] |= uint64(input[i+Params.Bytelen]) << (8 * k)
	}
	toMontDomain(fp2)
}

// Functions for traversing isogeny trees acoording to strategy. Key type 'A' is
//

// for public key generation.
func traverseTreePublicKeyA(curve *ProjectiveCurveParameters, xR, phiP, phiQ, phiR *ProjectivePoint, pub *PublicKey) {
	var points = make([]ProjectivePoint, 0, 8)
	var indices = make([]int, 0, 8)
	var i, sidx int

	cparam := CalcCurveParamsEquiv4(curve)
	phi := NewIsogeny4()
	strat := pub.params.A.IsogenyStrategy
	stratSz := len(strat)

	for j := 1; j <= stratSz; j++ {
		for i <= stratSz-j {
			points = append(points, *xR)
			indices = append(indices, i)

			k := strat[sidx]
			sidx++
			Pow2k(xR, &cparam, 2*k)
			i += int(k)
		}

		cparam = phi.GenerateCurve(xR)
		for k := 0; k < len(points); k++ {
			points[k] = phi.EvaluatePoint(&points[k])
		}

		*phiP = phi.EvaluatePoint(phiP)
		*phiQ = phi.EvaluatePoint(phiQ)
		*phiR = phi.EvaluatePoint(phiR)

		*xR, points = points[len(points)-1], points[:len(points)-1]
		i, indices = int(indices[len(indices)-1]), indices[:len(indices)-1]
	}
}

// for public key generation.
func traverseTreeSharedKeyA(curve *ProjectiveCurveParameters, xR *ProjectivePoint, pub *PublicKey) {
	var points = make([]ProjectivePoint, 0, 8)
	var indices = make([]int, 0, 8)
	var i, sidx int

	cparam := CalcCurveParamsEquiv4(curve)
	phi := NewIsogeny4()
	strat := pub.params.A.IsogenyStrategy
	stratSz := len(strat)

	for j := 1; j <= stratSz; j++ {
		for i <= stratSz-j {
			points = append(points, *xR)
			indices = append(indices, i)

			k := strat[sidx]
			sidx++
			Pow2k(xR, &cparam, 2*k)
			i += int(k)
		}

		cparam = phi.GenerateCurve(xR)
		for k := 0; k < len(points); k++ {
			points[k] = phi.EvaluatePoint(&points[k])
		}

		*xR, points = points[len(points)-1], points[:len(points)-1]
		i, indices = int(indices[len(indices)-1]), indices[:len(indices)-1]
	}
}

// for public key generation.
func traverseTreePublicKeyB(curve *ProjectiveCurveParameters, xR, phiP, phiQ, phiR *ProjectivePoint, pub *PublicKey) {
	var points = make([]ProjectivePoint, 0, 8)
	var indices = make([]int, 0, 8)
	var i, sidx int

	cparam := CalcCurveParamsEquiv3(curve)
	phi := NewIsogeny3()
	strat := pub.params.B.IsogenyStrategy
	stratSz := len(strat)

	for j := 1; j <= stratSz; j++ {
		for i <= stratSz-j {
			points = append(points, *xR)
			indices = append(indices, i)

			k := strat[sidx]
			sidx++
			Pow3k(xR, &cparam, k)
			i += int(k)
		}

		cparam = phi.GenerateCurve(xR)
		for k := 0; k < len(points); k++ {
			points[k] = phi.EvaluatePoint(&points[k])
		}

		*phiP = phi.EvaluatePoint(phiP)
		*phiQ = phi.EvaluatePoint(phiQ)
		*phiR = phi.EvaluatePoint(phiR)

		*xR, points = points[len(points)-1], points[:len(points)-1]
		i, indices = int(indices[len(indices)-1]), indices[:len(indices)-1]
	}
}

// for public key generation.
func traverseTreeSharedKeyB(curve *ProjectiveCurveParameters, xR *ProjectivePoint, pub *PublicKey) {
	var points = make([]ProjectivePoint, 0, 8)
	var indices = make([]int, 0, 8)
	var i, sidx int

	cparam := CalcCurveParamsEquiv3(curve)
	phi := NewIsogeny3()
	strat := pub.params.B.IsogenyStrategy
	stratSz := len(strat)

	for j := 1; j <= stratSz; j++ {
		for i <= stratSz-j {
			points = append(points, *xR)
			indices = append(indices, i)

			k := strat[sidx]
			sidx++
			Pow3k(xR, &cparam, k)
			i += int(k)
		}

		cparam = phi.GenerateCurve(xR)
		for k := 0; k < len(points); k++ {
			points[k] = phi.EvaluatePoint(&points[k])
		}

		*xR, points = points[len(points)-1], points[:len(points)-1]
		i, indices = int(indices[len(indices)-1]), indices[:len(indices)-1]
	}
}

func publicKeyGenA(prv *PrivateKey) (pub *PublicKey) {
	var xPA, xQA, xRA ProjectivePoint
	var xPB, xQB, xRB, xK ProjectivePoint
	var invZP, invZQ, invZR Fp2

	pub = NewPublicKey(KeyVariant_SIDH_A)
	var phi = NewIsogeny4()

	xPA = ProjectivePoint{X: prv.params.A.Affine_P, Z: prv.params.OneFp2}
	xQA = ProjectivePoint{X: prv.params.A.Affine_Q, Z: prv.params.OneFp2}
	xRA = ProjectivePoint{X: prv.params.A.Affine_R, Z: prv.params.OneFp2}

	xRB = ProjectivePoint{X: prv.params.B.Affine_R, Z: prv.params.OneFp2}
	xQB = ProjectivePoint{X: prv.params.B.Affine_Q, Z: prv.params.OneFp2}
	xPB = ProjectivePoint{X: prv.params.B.Affine_P, Z: prv.params.OneFp2}

	xK = ScalarMul3Pt(&pub.params.InitCurve, &xPA, &xQA, &xRA, prv.params.A.SecretBitLen, prv.Scalar)
	traverseTreePublicKeyA(&pub.params.InitCurve, &xK, &xPB, &xQB, &xRB, pub)

	phi.GenerateCurve(&xK)
	xPA = phi.EvaluatePoint(&xPB)
	xQA = phi.EvaluatePoint(&xQB)
	xRA = phi.EvaluatePoint(&xRB)
	Fp2Batch3Inv(&xPA.Z, &xQA.Z, &xRA.Z, &invZP, &invZQ, &invZR)

	mul(&pub.affine_xP, &xPA.X, &invZP)
	mul(&pub.affine_xQ, &xQA.X, &invZQ)
	mul(&pub.affine_xQmP, &xRA.X, &invZR)
	return
}

func publicKeyGenB(prv *PrivateKey) (pub *PublicKey) {
	var xPB, xQB, xRB, xK ProjectivePoint
	var xPA, xQA, xRA ProjectivePoint
	var invZP, invZQ, invZR Fp2

	pub = NewPublicKey(prv.keyVariant)
	var phi = NewIsogeny3()

	xRB = ProjectivePoint{X: prv.params.B.Affine_R, Z: prv.params.OneFp2}
	xQB = ProjectivePoint{X: prv.params.B.Affine_Q, Z: prv.params.OneFp2}
	xPB = ProjectivePoint{X: prv.params.B.Affine_P, Z: prv.params.OneFp2}

	xPA = ProjectivePoint{X: prv.params.A.Affine_P, Z: prv.params.OneFp2}
	xQA = ProjectivePoint{X: prv.params.A.Affine_Q, Z: prv.params.OneFp2}
	xRA = ProjectivePoint{X: prv.params.A.Affine_R, Z: prv.params.OneFp2}

	xK = ScalarMul3Pt(&pub.params.InitCurve, &xPB, &xQB, &xRB, prv.params.B.SecretBitLen, prv.Scalar)
	traverseTreePublicKeyB(&pub.params.InitCurve, &xK, &xPA, &xQA, &xRA, pub)

	phi.GenerateCurve(&xK)
	xPB = phi.EvaluatePoint(&xPA)
	xQB = phi.EvaluatePoint(&xQA)
	xRB = phi.EvaluatePoint(&xRA)
	Fp2Batch3Inv(&xPB.Z, &xQB.Z, &xRB.Z, &invZP, &invZQ, &invZR)

	mul(&pub.affine_xP, &xPB.X, &invZP)
	mul(&pub.affine_xQ, &xQB.X, &invZQ)
	mul(&pub.affine_xQmP, &xRB.X, &invZR)
	return
}

// Key agreement functions
//

func deriveSecretA(prv *PrivateKey, pub *PublicKey) []byte {
	var sharedSecret = make([]byte, pub.params.SharedSecretSize)
	var xP, xQ, xQmP ProjectivePoint
	var xK ProjectivePoint
	var cparam ProjectiveCurveParameters
	var phi = NewIsogeny4()
	var jInv Fp2

	RecoverCoordinateA(&cparam, &pub.affine_xP, &pub.affine_xQ, &pub.affine_xQmP)

	cparam.C = Params.OneFp2

	xP = ProjectivePoint{X: pub.affine_xP, Z: pub.params.OneFp2}
	xQ = ProjectivePoint{X: pub.affine_xQ, Z: pub.params.OneFp2}
	xQmP = ProjectivePoint{X: pub.affine_xQmP, Z: pub.params.OneFp2}
	xK = ScalarMul3Pt(&cparam, &xP, &xQ, &xQmP, pub.params.A.SecretBitLen, prv.Scalar)

	traverseTreeSharedKeyA(&cparam, &xK, pub)

	c := phi.GenerateCurve(&xK)
	RecoverCurveCoefficients4(&cparam, &c)
	Jinvariant(&cparam, &jInv)
	convFp2ToBytes(sharedSecret, &jInv)
	return sharedSecret
}

func deriveSecretB(prv *PrivateKey, pub *PublicKey) []byte {
	var sharedSecret = make([]byte, pub.params.SharedSecretSize)
	var xP, xQ, xQmP ProjectivePoint
	var xK ProjectivePoint
	var cparam ProjectiveCurveParameters
	var phi = NewIsogeny3()
	var jInv Fp2

	RecoverCoordinateA(&cparam, &pub.affine_xP, &pub.affine_xQ, &pub.affine_xQmP)

	cparam.C = Params.OneFp2

	xP = ProjectivePoint{X: pub.affine_xP, Z: pub.params.OneFp2}
	xQ = ProjectivePoint{X: pub.affine_xQ, Z: pub.params.OneFp2}
	xQmP = ProjectivePoint{X: pub.affine_xQmP, Z: pub.params.OneFp2}
	xK = ScalarMul3Pt(&cparam, &xP, &xQ, &xQmP, pub.params.B.SecretBitLen, prv.Scalar)

	traverseTreeSharedKeyB(&cparam, &xK, pub)

	c := phi.GenerateCurve(&xK)
	RecoverCurveCoefficients3(&cparam, &c)
	Jinvariant(&cparam, &jInv)
	convFp2ToBytes(sharedSecret, &jInv)
	return sharedSecret
}

func encrypt(skA *PrivateKey, pkA, pkB *PublicKey, ptext []byte) ([]byte, error) {
	if pkB.keyVariant != KeyVariant_SIKE {
		return nil, errors.New("wrong key type")
	}

	j, err := DeriveSecret(skA, pkB)
	if err != nil {
		return nil, err
	}

	if len(ptext) != pkA.params.KemSize {
		panic("Implementation error")
	}

	digest := sha256.Sum256(j)

	for i, _ := range ptext {
		digest[i] ^= ptext[i]
	}

	ret := make([]byte, pkA.Size()+len(ptext))
	copy(ret, pkA.Export())
	copy(ret[pkA.Size():], digest[:pkA.params.KemSize])
	return ret, nil
}

// Usage of this function guarantees that the object is correctly initialized.
func NewPrivateKey(v KeyVariant) *PrivateKey {
	prv := &PrivateKey{key: key{params: &Params, keyVariant: v}}
	if (v & KeyVariant_SIDH_A) == KeyVariant_SIDH_A {
		prv.Scalar = make([]byte, prv.params.A.SecretByteLen)
	} else {
		prv.Scalar = make([]byte, prv.params.B.SecretByteLen)
	}
	if v == KeyVariant_SIKE {
		prv.S = make([]byte, prv.params.MsgLen)
	}
	return prv
}

// Usage of this function guarantees that the object is correctly initialized.
func NewPublicKey(v KeyVariant) *PublicKey {
	return &PublicKey{key: key{params: &Params, keyVariant: v}}
}

// and imports key stored in the byte string. Returns error in case byte string
// size is wrong. Doesn't perform any validation.
func (pub *PublicKey) Import(input []byte) error {
	if len(input) != pub.Size() {
		return errors.New("sidh: input to short")
	}
	ssSz := pub.params.SharedSecretSize
	convBytesToFp2(&pub.affine_xP, input[0:ssSz])
	convBytesToFp2(&pub.affine_xQ, input[ssSz:2*ssSz])
	convBytesToFp2(&pub.affine_xQmP, input[2*ssSz:3*ssSz])
	return nil
}

// returned byte string is filled with zeros.
func (pub *PublicKey) Export() []byte {
	output := make([]byte, pub.params.PublicKeySize)
	ssSz := pub.params.SharedSecretSize
	convFp2ToBytes(output[0:ssSz], &pub.affine_xP)
	convFp2ToBytes(output[ssSz:2*ssSz], &pub.affine_xQ)
	convFp2ToBytes(output[2*ssSz:3*ssSz], &pub.affine_xQmP)
	return output
}

func (pub *PublicKey) Size() int {
	return pub.params.PublicKeySize
}

// returned byte string is filled with zeros.
func (prv *PrivateKey) Export() []byte {
	ret := make([]byte, len(prv.Scalar)+len(prv.S))
	copy(ret, prv.S)
	copy(ret[len(prv.S):], prv.Scalar)
	return ret
}

func (prv *PrivateKey) Size() int {
	tmp := len(prv.Scalar)
	if prv.keyVariant == KeyVariant_SIKE {
		tmp += int(prv.params.MsgLen)
	}
	return tmp
}

// and imports key from octet string. In case of SIKE, the random value 'S'
// must be prepended to the value of actual private key (see SIKE spec for details).
// Function doesn't import public key value to PrivateKey object.
func (prv *PrivateKey) Import(input []byte) error {
	if len(input) != prv.Size() {
		return errors.New("sidh: input to short")
	}
	copy(prv.S, input[:len(prv.S)])
	copy(prv.Scalar, input[len(prv.S):])
	return nil
}

// formed as little-endian integer from key-space <2^(e2-1)..2^e2 - 1>
// for KeyVariant_A or <2^(s-1)..2^s - 1>, where s = floor(log_2(3^e3)),
// for KeyVariant_B.
//
// Returns error in case user provided RNG fails.
func (prv *PrivateKey) Generate(rand io.Reader) error {
	var err error
	var dp *DomainParams

	if (prv.keyVariant & KeyVariant_SIDH_A) == KeyVariant_SIDH_A {
		dp = &prv.params.A
	} else {
		dp = &prv.params.B
	}

	if prv.keyVariant == KeyVariant_SIKE {
		_, err = io.ReadFull(rand, prv.S)
	}






	_, err = io.ReadFull(rand, prv.Scalar)
	if err != nil {
		return err
	}
	prv.Scalar[len(prv.Scalar)-1] &= (1 << (dp.SecretBitLen % 8)) - 1




	prv.Scalar[len(prv.Scalar)-1] |= 1 << ((dp.SecretBitLen % 8) - 1)
	return err
}

//
// Constant time.
func (prv *PrivateKey) GeneratePublicKey() *PublicKey {
	if (prv.keyVariant & KeyVariant_SIDH_A) == KeyVariant_SIDH_A {
		return publicKeyGenA(prv)
	}
	return publicKeyGenB(prv)
}

// different KeyVariant than prv. Length of returned output is 2*ceil(log_2 P)/8),
// where P is a prime defining finite field.
//
// It's important to notice that each keypair must not be used more than once
// to calculate shared secret.
//
// Function may return error. This happens only in case provided input is invalid.
// Constant time for properly initialized private and public key.
func DeriveSecret(prv *PrivateKey, pub *PublicKey) ([]byte, error) {

	if (pub == nil) || (prv == nil) {
		return nil, errors.New("sidh: invalid arguments")
	}

	if (pub.keyVariant == prv.keyVariant) || (pub.params.Id != prv.params.Id) {
		return nil, errors.New("sidh: public and private are incompatbile")
	}

	if (prv.keyVariant & KeyVariant_SIDH_A) == KeyVariant_SIDH_A {
		return deriveSecretA(prv, pub), nil
	} else {
		return deriveSecretB(prv, pub), nil
	}
}

// Returns ciphertext in case encryption succeeds. Returns error in case PRNG fails
// or wrongly formatted input was provided.
func Encrypt(rng io.Reader, pub *PublicKey, ptext []byte) ([]byte, error) {
	var ptextLen = len(ptext)

	if ptextLen != pub.params.KemSize {
		return nil, errors.New("Unsupported message length")
	}

	skA := NewPrivateKey(KeyVariant_SIDH_A)
	err := skA.Generate(rng)
	if err != nil {
		return nil, err
	}

	pkA := skA.GeneratePublicKey()
	return encrypt(skA, pkA, pub, ptext)
}

// decryption succeeds or error in case unexptected input was provided.
// Constant time
func Decrypt(prv *PrivateKey, ctext []byte) ([]byte, error) {
	var c1_len int
	n := make([]byte, prv.params.KemSize)
	pk_len := prv.params.PublicKeySize

	if prv.keyVariant != KeyVariant_SIKE {
		return nil, errors.New("wrong key type")
	}


	c1_len = len(ctext) - pk_len
	if c1_len != int(prv.params.KemSize) {
		return nil, errors.New("wrong size of cipher text")
	}

	c0 := NewPublicKey(KeyVariant_SIDH_A)
	err := c0.Import(ctext[:pk_len])
	if err != nil {
		return nil, err
	}
	j, err := DeriveSecret(prv, c0)
	if err != nil {
		return nil, err
	}

	digest := sha256.Sum256(j)
	copy(n, digest[:])

	for i, _ := range n {
		n[i] ^= ctext[pk_len+i]
	}
	return n[:c1_len], nil
}

// The generated ciphertext is used for authentication.
// The rng must be cryptographically secure PRNG.
// Error is returned in case PRNG fails or wrongly formatted input was provided.
func Encapsulate(rng io.Reader, pub *PublicKey) (ctext []byte, secret []byte, err error) {

	ptext := make([]byte, pub.params.MsgLen)

	d := sha256.New()

	_, err = io.ReadFull(rng, ptext)
	if err != nil {
		return nil, nil, err
	}

	d.Write(ptext)
	d.Write(pub.Export())
	digest := d.Sum(nil)

	r := digest[:pub.params.A.SecretByteLen]

	skA := NewPrivateKey(KeyVariant_SIDH_A)
	err = skA.Import(r)
	if err != nil {
		return nil, nil, err
	}

	pkA := skA.GeneratePublicKey()
	ctext, err = encrypt(skA, pkA, pub, ptext)
	if err != nil {
		return nil, nil, err
	}

	d.Reset()
	d.Write(ptext)
	d.Write(ctext)
	digest = d.Sum(digest[:0])
	return ctext, digest[:pub.params.KemSize], nil
}

// secret if plaintext verifies correctly, otherwise function outputs random value.
// Decapsulation may fail in case input is wrongly formatted.
// Constant time for properly initialized input.
func Decapsulate(prv *PrivateKey, pub *PublicKey, ctext []byte) ([]byte, error) {
	var skA = NewPrivateKey(KeyVariant_SIDH_A)

	d := sha256.New()

	m, err := Decrypt(prv, ctext)
	if err != nil {
		return nil, err
	}

	d.Write(m)
	d.Write(pub.Export())
	digest := d.Sum(nil)

	skA.Import(digest[:pub.params.A.SecretByteLen])

	pkA := skA.GeneratePublicKey()
	c0 := pkA.Export()

	d.Reset()
	if subtle.ConstantTimeCompare(c0, ctext[:len(c0)]) == 1 {
		d.Write(m)
	} else {







		d.Write(prv.S)
	}
	d.Write(ctext)
	digest = d.Sum(digest[:0])
	return digest[:pub.params.KemSize], nil
}
