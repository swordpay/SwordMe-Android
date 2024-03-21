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

type KeyVariant uint

//
// No particular meaning is assigned to the representation -- it could represent
// an element in Montgomery form, or not.  Tracking the meaning of the field
// element is left to higher types.
type Fp [FP_WORDS]uint64

type FpX2 [2 * FP_WORDS]uint64

type Fp2 struct {
	A Fp
	B Fp
}

type DomainParams struct {

	Affine_P, Affine_Q, Affine_R Fp2

	IsogenyStrategy []uint32

	SecretBitLen uint

	SecretByteLen uint
}

type SidhParams struct {
	Id uint8

	Bytelen int

	PublicKeySize int

	SharedSecretSize int

	InitCurve ProjectiveCurveParameters

	A, B DomainParams

	HalfFp2 Fp2

	OneFp2 Fp2


	MsgLen int

	KemSize int

	CiphertextSize int
}

// values depends on the context. When working with isogenies over
// subgroup that are powers of:
// * three then  (A:C) ~ (A+2C:A-2C)
// * four then   (A:C) ~ (A+2C:  4C)
// See Appendix A of SIKE for more details
type CurveCoefficientsEquiv struct {
	A Fp2
	C Fp2
}

//
// This represents a point on the Kummer line of a Montgomery curve.  The
// curve is specified by a ProjectiveCurveParameters struct.
type ProjectivePoint struct {
	X Fp2
	Z Fp2
}

// parameters.
type key struct {

	params *SidhParams

	keyVariant KeyVariant
}

type PrivateKey struct {
	key

	Scalar []byte

	S []byte
}

type PublicKey struct {
	key
	affine_xP   Fp2
	affine_xQ   Fp2
	affine_xQmP Fp2
}

//
// This is used to work projectively with the curve coefficients.
type ProjectiveCurveParameters struct {
	A Fp2
	C Fp2
}

const (



	KeyVariant_SIDH_A KeyVariant = 1 << 0

	KeyVariant_SIDH_B = 1 << 1

	KeyVariant_SIKE = 1<<2 | KeyVariant_SIDH_B

	FP_WORDS = 7
)

// -------------------------------

var (
	p = Fp{
		0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFDC1767AE2FFFFFF,
		0x7BC65C783158AEA3, 0x6CFC5FD681C52056, 0x2341F27177344,
	}

	pX2 = Fp{
		0xFFFFFFFFFFFFFFFE, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFB82ECF5C5FFFFFF,
		0xF78CB8F062B15D47, 0xD9F8BFAD038A40AC, 0x4683E4E2EE688,
	}

	p1 = Fp{
		0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0xFDC1767AE3000000,
		0x7BC65C783158AEA3, 0x6CFC5FD681C52056, 0x0002341F27177344,
	}

	R2 = Fp{
		0x28E55B65DCD69B30, 0xACEC7367768798C2, 0xAB27973F8311688D, 0x175CC6AF8D6C7C0B,
		0xABCD92BF2DDE347E, 0x69E16A61C7686D9A, 0x000025A89BCDD12A,
	}

	half = Fp2{
		A: Fp{
			0x0000000000003A16, 0x0000000000000000, 0x0000000000000000, 0x5C87FA027E000000,
			0x6C00D27DAACFD66A, 0x74992A2A2FBBA086, 0x0000767753DE976D},
	}

	one = Fp2{
		A: Fp{
			0x000000000000742C, 0x0000000000000000, 0x0000000000000000, 0xB90FF404FC000000,
			0xD801A4FB559FACD4, 0xE93254545F77410C, 0x0000ECEEA7BD2EDA},
	}

	six = Fp2{
		A: Fp{
			0x000000000002B90A, 0x0000000000000000, 0x0000000000000000, 0x5ADCCB2822000000,
			0x187D24F39F0CAFB4, 0x9D353A4D394145A0, 0x00012559A0403298},
	}

	Params SidhParams
)

func init() {
	Params = SidhParams{

		PublicKeySize: 330,

		SharedSecretSize: 110,
		InitCurve: ProjectiveCurveParameters{
			A: six,
			C: one,
		},
		A: DomainParams{

			Affine_P: Fp2{
				A: Fp{
					0x05ADF455C5C345BF, 0x91935C5CC767AC2B, 0xAFE4E879951F0257, 0x70E792DC89FA27B1,
					0xF797F526BB48C8CD, 0x2181DB6131AF621F, 0x00000A1C08B1ECC4,
				},
				B: Fp{
					0x74840EB87CDA7788, 0x2971AA0ECF9F9D0B, 0xCB5732BDF41715D5, 0x8CD8E51F7AACFFAA,
					0xA7F424730D7E419F, 0xD671EB919A179E8C, 0x0000FFA26C5A924A,
				},
			},

			Affine_Q: Fp2{
				A: Fp{
					0xFEC6E64588B7273B, 0xD2A626D74CBBF1C6, 0xF8F58F07A78098C7, 0xE23941F470841B03,
					0x1B63EDA2045538DD, 0x735CFEB0FFD49215, 0x0001C4CB77542876,
				},
				B: Fp{
					0xADB0F733C17FFDD6, 0x6AFFBD037DA0A050, 0x680EC43DB144E02F, 0x1E2E5D5FF524E374,
					0xE2DDA115260E2995, 0xA6E4B552E2EDE508, 0x00018ECCDDF4B53E,
				},
			},

			Affine_R: Fp2{
				A: Fp{
					0x01BA4DB518CD6C7D, 0x2CB0251FE3CC0611, 0x259B0C6949A9121B, 0x60E17AC16D2F82AD,
					0x3AA41F1CE175D92D, 0x413FBE6A9B9BC4F3, 0x00022A81D8D55643,
				},
				B: Fp{
					0xB8ADBC70FC82E54A, 0xEF9CDDB0D5FADDED, 0x5820C734C80096A0, 0x7799994BAA96E0E4,
					0x044961599E379AF8, 0xDB2B94FBF09F27E2, 0x0000B87FC716C0C6,
				},
			},

			SecretBitLen: 216,

			SecretByteLen: 27,

			IsogenyStrategy: []uint32{
				0x30, 0x1C, 0x10, 0x08, 0x04, 0x02, 0x01, 0x01, 0x02, 0x01,
				0x01, 0x04, 0x02, 0x01, 0x01, 0x02, 0x01, 0x01, 0x08, 0x04,
				0x02, 0x01, 0x01, 0x02, 0x01, 0x01, 0x04, 0x02, 0x01, 0x01,
				0x02, 0x01, 0x01, 0x0D, 0x07, 0x04, 0x02, 0x01, 0x01, 0x02,
				0x01, 0x01, 0x03, 0x02, 0x01, 0x01, 0x01, 0x01, 0x05, 0x04,
				0x02, 0x01, 0x01, 0x02, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01,
				0x15, 0x0C, 0x07, 0x04, 0x02, 0x01, 0x01, 0x02, 0x01, 0x01,
				0x03, 0x02, 0x01, 0x01, 0x01, 0x01, 0x05, 0x03, 0x02, 0x01,
				0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01, 0x09, 0x05, 0x03,
				0x02, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01, 0x04,
				0x02, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01},
		},
		B: DomainParams{

			Affine_P: Fp2{
				A: Fp{
					0x6E5497556EDD48A3, 0x2A61B501546F1C05, 0xEB919446D049887D, 0x5864A4A69D450C4F,
					0xB883F276A6490D2B, 0x22CC287022D5F5B9, 0x0001BED4772E551F,
				},
				B: Fp{
					0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
					0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
				},
			},

			Affine_Q: Fp2{
				A: Fp{
					0xFAE2A3F93D8B6B8E, 0x494871F51700FE1C, 0xEF1A94228413C27C, 0x498FF4A4AF60BD62,
					0xB00AD2A708267E8A, 0xF4328294E017837F, 0x000034080181D8AE,
				},
				B: Fp{
					0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
					0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
				},
			},

			Affine_R: Fp2{
				A: Fp{
					0x283B34FAFEFDC8E4, 0x9208F44977C3E647, 0x7DEAE962816F4E9A, 0x68A2BA8AA262EC9D,
					0x8176F112EA43F45B, 0x02106D022634F504, 0x00007E8A50F02E37,
				},
				B: Fp{
					0xB378B7C1DA22CCB1, 0x6D089C99AD1D9230, 0xEBE15711813E2369, 0x2B35A68239D48A53,
					0x445F6FD138407C93, 0xBEF93B29A3F6B54B, 0x000173FA910377D3,
				},
			},

			SecretBitLen: 217,

			SecretByteLen: 28,

			IsogenyStrategy: []uint32{
				0x42, 0x21, 0x11, 0x09, 0x05, 0x03, 0x02, 0x01, 0x01, 0x01,
				0x01, 0x02, 0x01, 0x01, 0x01, 0x04, 0x02, 0x01, 0x01, 0x01,
				0x02, 0x01, 0x01, 0x08, 0x04, 0x02, 0x01, 0x01, 0x01, 0x02,
				0x01, 0x01, 0x04, 0x02, 0x01, 0x01, 0x02, 0x01, 0x01, 0x10,
				0x08, 0x04, 0x02, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x04,
				0x02, 0x01, 0x01, 0x02, 0x01, 0x01, 0x08, 0x04, 0x02, 0x01,
				0x01, 0x02, 0x01, 0x01, 0x04, 0x02, 0x01, 0x01, 0x02, 0x01,
				0x01, 0x20, 0x10, 0x08, 0x04, 0x03, 0x01, 0x01, 0x01, 0x01,
				0x02, 0x01, 0x01, 0x04, 0x02, 0x01, 0x01, 0x02, 0x01, 0x01,
				0x08, 0x04, 0x02, 0x01, 0x01, 0x02, 0x01, 0x01, 0x04, 0x02,
				0x01, 0x01, 0x02, 0x01, 0x01, 0x10, 0x08, 0x04, 0x02, 0x01,
				0x01, 0x02, 0x01, 0x01, 0x04, 0x02, 0x01, 0x01, 0x02, 0x01,
				0x01, 0x08, 0x04, 0x02, 0x01, 0x01, 0x02, 0x01, 0x01, 0x04,
				0x02, 0x01, 0x01, 0x02, 0x01, 0x01},
		},
		OneFp2:  one,
		HalfFp2: half,
		MsgLen:  16,

		KemSize: 16,

		Bytelen:        55,
		CiphertextSize: 16 + 330,
	}
}
