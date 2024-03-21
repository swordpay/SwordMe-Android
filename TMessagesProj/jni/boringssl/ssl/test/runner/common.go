// Copyright 2009 The Go Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package runner

import (
	"container/list"
	"crypto"
	"crypto/ecdsa"
	"crypto/rand"
	"crypto/x509"
	"errors"
	"fmt"
	"io"
	"math/big"
	"strings"
	"sync"
	"time"
)

const (
	VersionSSL30 = 0x0300
	VersionTLS10 = 0x0301
	VersionTLS11 = 0x0302
	VersionTLS12 = 0x0303
	VersionTLS13 = 0x0304
)

const (
	VersionDTLS10 = 0xfeff
	VersionDTLS12 = 0xfefd
)

var allTLSWireVersions = []uint16{
	VersionTLS13,
	VersionTLS12,
	VersionTLS11,
	VersionTLS10,
	VersionSSL30,
}

var allDTLSWireVersions = []uint16{
	VersionDTLS12,
	VersionDTLS10,
}

const (
	maxPlaintext        = 16384        // maximum plaintext payload length
	maxCiphertext       = 16384 + 2048 // maximum ciphertext payload length
	tlsRecordHeaderLen  = 5            // record header length
	dtlsRecordHeaderLen = 13
	maxHandshake        = 65536 // maximum handshake we support (protocol max is 16 MB)

	minVersion = VersionSSL30
	maxVersion = VersionTLS13
)

type recordType uint8

const (
	recordTypeChangeCipherSpec   recordType = 20
	recordTypeAlert              recordType = 21
	recordTypeHandshake          recordType = 22
	recordTypeApplicationData    recordType = 23
	recordTypePlaintextHandshake recordType = 24
)

const (
	typeHelloRequest          uint8 = 0
	typeClientHello           uint8 = 1
	typeServerHello           uint8 = 2
	typeHelloVerifyRequest    uint8 = 3
	typeNewSessionTicket      uint8 = 4
	typeEndOfEarlyData        uint8 = 5
	typeHelloRetryRequest     uint8 = 6
	typeEncryptedExtensions   uint8 = 8
	typeCertificate           uint8 = 11
	typeServerKeyExchange     uint8 = 12
	typeCertificateRequest    uint8 = 13
	typeServerHelloDone       uint8 = 14
	typeCertificateVerify     uint8 = 15
	typeClientKeyExchange     uint8 = 16
	typeFinished              uint8 = 20
	typeCertificateStatus     uint8 = 22
	typeKeyUpdate             uint8 = 24
	typeCompressedCertificate uint8 = 25  // Not IANA assigned
	typeNextProtocol          uint8 = 67  // Not IANA assigned
	typeChannelID             uint8 = 203 // Not IANA assigned
	typeMessageHash           uint8 = 254
)

const (
	compressionNone uint8 = 0
)

const (
	extensionServerName                 uint16 = 0
	extensionStatusRequest              uint16 = 5
	extensionSupportedCurves            uint16 = 10
	extensionSupportedPoints            uint16 = 11
	extensionSignatureAlgorithms        uint16 = 13
	extensionUseSRTP                    uint16 = 14
	extensionALPN                       uint16 = 16
	extensionSignedCertificateTimestamp uint16 = 18
	extensionPadding                    uint16 = 21
	extensionExtendedMasterSecret       uint16 = 23
	extensionTokenBinding               uint16 = 24
	extensionCompressedCertAlgs         uint16 = 27
	extensionSessionTicket              uint16 = 35
	extensionPreSharedKey               uint16 = 41
	extensionEarlyData                  uint16 = 42
	extensionSupportedVersions          uint16 = 43
	extensionCookie                     uint16 = 44
	extensionPSKKeyExchangeModes        uint16 = 45
	extensionCertificateAuthorities     uint16 = 47
	extensionSignatureAlgorithmsCert    uint16 = 50
	extensionKeyShare                   uint16 = 51
	extensionCustom                     uint16 = 1234  // not IANA assigned
	extensionNextProtoNeg               uint16 = 13172 // not IANA assigned
	extensionRenegotiationInfo          uint16 = 0xff01
	extensionQUICTransportParams        uint16 = 0xffa5 // draft-ietf-quic-tls-13
	extensionChannelID                  uint16 = 30032  // not IANA assigned
	extensionDelegatedCredentials       uint16 = 0xff02 // not IANA assigned
	extensionPQExperimentSignal         uint16 = 54538
)

const (
	scsvRenegotiation uint16 = 0x00ff
)

var tls13HelloRetryRequest = []uint8{
	0xcf, 0x21, 0xad, 0x74, 0xe5, 0x9a, 0x61, 0x11, 0xbe, 0x1d, 0x8c,
	0x02, 0x1e, 0x65, 0xb8, 0x91, 0xc2, 0xa2, 0x11, 0x16, 0x7a, 0xbb,
	0x8c, 0x5e, 0x07, 0x9e, 0x09, 0xe2, 0xc8, 0xa8, 0x33, 0x9c,
}

// http://www.iana.org/assignments/tls-parameters/tls-parameters.xml#tls-parameters-8
type CurveID uint16

const (
	CurveP224    CurveID = 21
	CurveP256    CurveID = 23
	CurveP384    CurveID = 24
	CurveP521    CurveID = 25
	CurveX25519  CurveID = 29
	CurveCECPQ2  CurveID = 16696
	CurveCECPQ2b CurveID = 65074
)

// http://www.iana.org/assignments/tls-parameters/tls-parameters.xml#tls-parameters-9
const (
	pointFormatUncompressed    uint8 = 0
	pointFormatCompressedPrime uint8 = 1
)

const (
	statusTypeOCSP uint8 = 1
)

const (
	CertTypeRSASign    = 1 // A certificate containing an RSA key
	CertTypeDSSSign    = 2 // A certificate containing a DSA key
	CertTypeRSAFixedDH = 3 // A certificate containing a static DH key
	CertTypeDSSFixedDH = 4 // A certificate containing a static DH key

	CertTypeECDSASign      = 64 // A certificate containing an ECDSA-capable public key, signed with ECDSA.
	CertTypeRSAFixedECDH   = 65 // A certificate containing an ECDH-capable public key, signed with RSA.
	CertTypeECDSAFixedECDH = 66 // A certificate containing an ECDH-capable public key, signed with ECDSA.

)

// that TLS 1.3 names the production 'SignatureScheme' to avoid colliding with
// TLS 1.2's SignatureAlgorithm but otherwise refers to them as 'signature
// algorithms' throughout. We match the latter.
type signatureAlgorithm uint16

const (

	signatureRSAPKCS1WithMD5    signatureAlgorithm = 0x0101
	signatureRSAPKCS1WithSHA1   signatureAlgorithm = 0x0201
	signatureRSAPKCS1WithSHA256 signatureAlgorithm = 0x0401
	signatureRSAPKCS1WithSHA384 signatureAlgorithm = 0x0501
	signatureRSAPKCS1WithSHA512 signatureAlgorithm = 0x0601

	signatureECDSAWithSHA1          signatureAlgorithm = 0x0203
	signatureECDSAWithP256AndSHA256 signatureAlgorithm = 0x0403
	signatureECDSAWithP384AndSHA384 signatureAlgorithm = 0x0503
	signatureECDSAWithP521AndSHA512 signatureAlgorithm = 0x0603

	signatureRSAPSSWithSHA256 signatureAlgorithm = 0x0804
	signatureRSAPSSWithSHA384 signatureAlgorithm = 0x0805
	signatureRSAPSSWithSHA512 signatureAlgorithm = 0x0806

	signatureEd25519 signatureAlgorithm = 0x0807
	signatureEd448   signatureAlgorithm = 0x0808
)

// algorithms.
var supportedSignatureAlgorithms = []signatureAlgorithm{
	signatureRSAPSSWithSHA256,
	signatureRSAPKCS1WithSHA256,
	signatureECDSAWithP256AndSHA256,
	signatureRSAPKCS1WithSHA1,
	signatureECDSAWithSHA1,
	signatureEd25519,
}

const (
	SRTP_AES128_CM_HMAC_SHA1_80 uint16 = 0x0001
	SRTP_AES128_CM_HMAC_SHA1_32        = 0x0002
)

const (
	pskKEMode    = 0
	pskDHEKEMode = 1
)

const (
	keyUpdateNotRequested = 0
	keyUpdateRequested    = 1
)

type ConnectionState struct {
	Version                    uint16                // TLS version used by the connection (e.g. VersionTLS12)
	HandshakeComplete          bool                  // TLS handshake is complete
	DidResume                  bool                  // connection resumes a previous TLS connection
	CipherSuite                uint16                // cipher suite in use (TLS_RSA_WITH_RC4_128_SHA, ...)
	NegotiatedProtocol         string                // negotiated next protocol (from Config.NextProtos)
	NegotiatedProtocolIsMutual bool                  // negotiated protocol was advertised by server
	NegotiatedProtocolFromALPN bool                  // protocol negotiated with ALPN
	ServerName                 string                // server name requested by client, if any (server side only)
	PeerCertificates           []*x509.Certificate   // certificate chain presented by remote peer
	VerifiedChains             [][]*x509.Certificate // verified chains built from PeerCertificates
	ChannelID                  *ecdsa.PublicKey      // the channel ID for this connection
	TokenBindingNegotiated     bool                  // whether Token Binding was negotiated
	TokenBindingParam          uint8                 // the negotiated Token Binding key parameter
	SRTPProtectionProfile      uint16                // the negotiated DTLS-SRTP protection profile
	TLSUnique                  []byte                // the tls-unique channel binding
	SCTList                    []byte                // signed certificate timestamp list
	PeerSignatureAlgorithm     signatureAlgorithm    // algorithm used by the peer in the handshake
	CurveID                    CurveID               // the curve used in ECDHE
	QUICTransportParams        []byte                // the QUIC transport params received from the peer
}

// TLS Client Authentication.
type ClientAuthType int

const (
	NoClientCert ClientAuthType = iota
	RequestClientCert
	RequireAnyClientCert
	VerifyClientCertIfGiven
	RequireAndVerifyClientCert
)

// sessions.
type ClientSessionState struct {
	sessionId            []uint8             // Session ID supplied by the server. nil if the session has a ticket.
	sessionTicket        []uint8             // Encrypted ticket used for session resumption with server
	vers                 uint16              // SSL/TLS version negotiated for the session
	wireVersion          uint16              // Wire SSL/TLS version negotiated for the session
	cipherSuite          uint16              // Ciphersuite negotiated for the session
	masterSecret         []byte              // MasterSecret generated by client on a full handshake
	handshakeHash        []byte              // Handshake hash for Channel ID purposes.
	serverCertificates   []*x509.Certificate // Certificate chain presented by the server
	extendedMasterSecret bool                // Whether an extended master secret was used to generate the session
	sctList              []byte
	ocspResponse         []byte
	earlyALPN            string
	ticketCreationTime   time.Time
	ticketExpiration     time.Time
	ticketAgeAdd         uint32
	maxEarlyDataSize     uint32
}

// by a client to resume a TLS session with a given server. ClientSessionCache
// implementations should expect to be called concurrently from different
// goroutines.
type ClientSessionCache interface {


	Get(sessionKey string) (session *ClientSessionState, ok bool)

	Put(sessionKey string, cs *ClientSessionState)
}

// client to resume a TLS session with a given server. ServerSessionCache
// implementations should expect to be called concurrently from different
// goroutines.
type ServerSessionCache interface {


	Get(sessionId string) (session *sessionState, ok bool)

	Put(sessionId string, session *sessionState)
}

// pair of functions for compressing and decompressing certificates.
type CertCompressionAlg struct {

	Compress func([]byte) []byte


	Decompress func(out, in []byte) bool
}

// After one has been passed to a TLS function it must not be
// modified. A Config may be reused; the tls package will also not
// modify it.
type Config struct {




	Rand io.Reader


	Time func() time.Time



	Certificates []Certificate






	NameToCertificate map[string]*Certificate



	RootCAs *x509.CertPool

	NextProtos []string



	ServerName string


	ClientAuth ClientAuthType



	ClientCAs *x509.CertPool


	ClientCertificateTypes []byte






	InsecureSkipVerify bool


	CipherSuites []uint16




	PreferServerCipherSuites bool


	SessionTicketsDisabled bool








	SessionTicketKey [32]byte


	ClientSessionCache ClientSessionCache


	ServerSessionCache ServerSessionCache


	MinVersion uint16



	MaxVersion uint16



	CurvePreferences []CurveID




	DefaultCurves []CurveID


	ChannelID *ecdsa.PrivateKey



	RequestChannelID bool



	TokenBindingParams []byte


	TokenBindingVersion uint16



	ExpectTokenBindingParams []byte


	PreSharedKey []byte


	PreSharedKeyIdentity string




	MaxEarlyDataSize uint32


	SRTPProtectionProfiles []uint16


	SignSignatureAlgorithms []signatureAlgorithm


	VerifySignatureAlgorithms []signatureAlgorithm


	QUICTransportParams []byte

	CertCompressionAlgs map[uint16]CertCompressionAlg



	PQExperimentSignal bool


	Bugs ProtocolBugs

	serverInitOnce sync.Once // guards calling (*Config).serverInit
}

type BadValue int

const (
	BadValueNone BadValue = iota
	BadValueNegative
	BadValueZero
	BadValueLimit
	BadValueLarge
	NumBadValues
)

type RSABadValue int

const (
	RSABadValueNone RSABadValue = iota
	RSABadValueCorrupt
	RSABadValueTooLong
	RSABadValueTooShort
	RSABadValueWrongVersion1
	RSABadValueWrongVersion2
	RSABadValueWrongBlockType
	RSABadValueWrongLeadingByte
	RSABadValueNoZero
	NumRSABadValues
)

type RSAPSSSupport int

const (
	RSAPSSSupportAny RSAPSSSupport = iota
	RSAPSSSupportNone
	RSAPSSSupportOnlineSignatureOnly
	RSAPSSSupportBoth
)

type ProtocolBugs struct {


	InvalidSignature bool



	SendCurve CurveID


	InvalidECDHPoint bool


	BadECDSAR BadValue
	BadECDSAS BadValue

	MaxPadding bool


	PaddingFirstByteBad bool


	PaddingFirstByteBadIf255 bool


	FailIfNotFallbackSCSV bool


	DuplicateExtension bool




	UnauthenticatedECDH bool


	SkipHelloVerifyRequest bool



	SkipCertificateStatus bool


	SkipServerKeyExchange bool


	SkipNewSessionTicket bool


	UseFirstSessionTicket bool


	SkipClientCertificate bool



	SkipChangeCipherSpec bool


	SkipFinished bool


	SkipEndOfEarlyData bool


	NonEmptyEndOfEarlyData bool


	SkipCertificateVerify bool




	EarlyChangeCipherSpec int



	StrayChangeCipherSpec bool



	ReorderChangeCipherSpec bool



	FragmentAcrossChangeCipherSpec bool


	SendExtraChangeCipherSpec int


	SendPostHandshakeChangeCipherSpec bool


	SendUnencryptedFinished bool



	PartialEncryptedExtensionsWithServerHello bool



	PartialClientFinishedWithClientHello bool


	SendV2ClientHello bool


	SendFallbackSCSV bool


	SendRenegotiationSCSV bool






	MaxHandshakeRecordLength int


	FragmentClientVersion bool


	FragmentAlert bool


	DoubleAlert bool


	SendSpuriousAlert alert


	BadRSAClientKeyExchange RSABadValue


	RenewTicketOnResume bool


	SendClientVersion uint16


	OmitSupportedVersions bool


	SendSupportedVersions []uint16



	NegotiateVersion uint16


	NegotiateVersionOnRenego uint16




	ExpectFalseStart bool





	AlertBeforeFalseStartTest alert


	ExpectServerName string


	SwapNPNAndALPN bool


	ALPNProtocol *string




	AcceptAnySession bool


	SendBothTickets bool


	FilterTicket func([]byte) ([]byte, error)


	TicketSessionIDLength int





	EmptyTicketSessionID bool


	SendClientHelloSessionID []byte


	ExpectClientHelloSessionID bool


	EchoSessionIDInFullHandshake bool


	ExpectNoTLS12Session bool


	ExpectNoTLS13PSK bool


	ExpectNoTLS13PSKAfterHRR bool


	RequireExtendedMasterSecret bool



	NoExtendedMasterSecret bool



	NoExtendedMasterSecretOnRenegotiation bool


	EmptyRenegotiationInfo bool


	BadRenegotiationInfo bool


	BadRenegotiationInfoEnd bool


	NoRenegotiationInfo bool


	NoRenegotiationInfoInInitial bool


	NoRenegotiationInfoAfterInitial bool


	RequireRenegotiationInfo bool




	SequenceNumberMapping func(uint64) uint64



	RSAEphemeralKey bool



	SRTPMasterKeyIdentifer string


	SendSRTPProtectionProfile uint16






	NoSignatureAlgorithms bool


	NoSupportedCurves bool



	RequireSameRenegoClientVersion bool


	ExpectInitialRecordVersion uint16


	SendRecordVersion uint16


	SendInitialRecordVersion uint16


	MaxPacketLength int



	SendCipherSuite uint16



	SendCipherSuites []uint16


	AppDataBeforeHandshake []byte


	AppDataAfterChangeCipherSpec []byte


	AlertAfterChangeCipherSpec alert


	TimeoutSchedule []time.Duration

	PacketAdaptor *packetAdaptor




	ReorderHandshakeFragments bool


	ReverseHandshakeFragments bool



	MixCompleteMessageWithFragments bool


	RetransmitFinished bool


	SendInvalidRecordType bool


	SendWrongMessageType byte


	SendTrailingMessageData byte


	FragmentMessageTypeMismatch bool


	FragmentMessageLengthMismatch bool



	SplitFragments int


	SendEmptyFragments bool



	SendSplitAlert bool


	FailIfResumeOnRenego bool


	IgnorePeerCipherPreferences bool


	IgnorePeerSignatureAlgorithmPreferences bool


	IgnorePeerCurvePreferences bool

	BadFinished bool



	PackHandshakeFragments int



	PackHandshakeRecords int





	PackAppDataWithHandshake bool


	SplitAndPackAppData bool


	PackHandshakeFlight bool


	AdvertiseAllConfiguredCiphers bool


	EmptyCertificateList bool


	ExpectNewTicket bool


	RequireClientHelloSize int


	CustomExtension string


	CustomUnencryptedExtension string


	ExpectedCustomExtension *string


	CustomTicketExtension string


	CustomHelloRetryRequestExtension string


	NoCloseNotify bool


	SendAlertOnShutdown alert



	ExpectCloseNotify bool


	SendLargeRecords bool


	NegotiateALPNAndNPN bool



	SendALPN string


	SendUnencryptedALPN string


	SendEmptySessionTicket bool


	SendPSKKeyExchangeModes []byte


	ExpectNoNewSessionTicket bool


	DuplicateTicketEarlyData bool


	ExpectTicketEarlyData bool


	ExpectTicketAge time.Duration


	SendTicketAge time.Duration



	FailIfSessionOffered bool



	SendHelloRequestBeforeEveryAppDataRecord bool



	SendHelloRequestBeforeEveryHandshakeMessage bool


	BadChangeCipherSpec []byte


	BadHelloRequest []byte


	RequireSessionTickets bool


	RequireSessionIDs bool


	NullAllCiphers bool


	SendSCTListOnResume []byte


	SendSCTListOnRenegotiation []byte



	SendOCSPResponseOnResume []byte


	SendOCSPResponseOnRenegotiation []byte


	SendExtensionOnCertificate []byte


	SendOCSPOnIntermediates []byte


	SendSCTOnIntermediates []byte


	SendDuplicateCertExtensions bool


	ExpectNoExtensionsOnIntermediate bool


	RecordPadding int



	OmitRecordContents bool


	OuterRecordType recordType


	SendSignatureAlgorithm signatureAlgorithm


	SkipECDSACurveCheck bool


	IgnoreSignatureVersionChecks bool


	NegotiateRenegotiationInfoAtAllVersions bool


	NegotiateNPNAtAllVersions bool


	NegotiateEMSAtAllVersions bool


	AdvertiseTicketExtension bool


	NegotiatePSKResumption bool


	AlwaysSelectPSKIdentity bool


	SelectPSKIdentityOnResume uint16


	ExtraPSKIdentity bool



	MissingKeyShare bool



	SecondClientHelloMissingKeyShare bool



	MisinterpretHelloRetryRequestCurve CurveID


	DuplicateKeyShares bool

	SendEarlyAlert bool


	SendFakeEarlyDataLength int


	SendStrayEarlyHandshake bool


	OmitEarlyDataExtension bool


	SendEarlyDataOnSecondClientHello bool


	InterleaveEarlyData bool




	SendEarlyData [][]byte


	ExpectEarlyDataAccepted bool


	AlwaysAcceptEarlyData bool

	AlwaysRejectEarlyData bool



	SendEarlyDataExtension bool






	ExpectEarlyData [][]byte






	ExpectLateEarlyData [][]byte



	SendHalfRTTData [][]byte






	ExpectHalfRTTData [][]byte


	EmptyEncryptedExtensions bool


	EncryptedExtensionsWithKeyShare bool


	AlwaysSendHelloRetryRequest bool


	SecondHelloRetryRequest bool


	SendHelloRetryRequestCurve CurveID


	SendHelloRetryRequestCipherSuite uint16


	SendHelloRetryRequestCookie []byte


	DuplicateHelloRetryRequestExtensions bool


	SendServerHelloVersion uint16



	SendServerSupportedVersionExtension uint16



	OmitServerSupportedVersionExtension bool


	SkipHelloRetryRequest bool


	PackHelloRequestWithFinished bool



	ExpectMissingKeyShare bool


	SendExtraFinished bool


	SendRequestContext []byte


	OmitCertificateRequestAlgorithms bool


	SendCustomCertificateRequest uint16


	SendSNIWarningAlert bool


	SendCompressionMethods []byte


	SendCompressionMethod byte



	AlwaysSendPreSharedKeyIdentityHint bool


	TrailingKeyShareData bool


	InvalidChannelIDSignature bool


	ExpectGREASE bool


	OmitPSKsOnSecondClientHello bool


	OnlyCorruptSecondPSKBinder bool


	SendShortPSKBinder bool


	SendInvalidPSKBinder bool

	SendNoPSKBinder bool


	SendExtraPSKBinder bool


	PSKBinderFirst bool


	NoOCSPStapling bool


	NoSignedCertificateTimestamps bool



	SendSupportedPointFormats []byte




	SendServerSupportedCurves bool


	MaxReceivePlaintext int



	ExpectPackedEncryptedHandshake int


	SendTicketLifetime time.Duration


	SendServerNameAck bool


	ExpectCertificateReqNames [][]byte


	RenegotiationCertificate *Certificate


	ExpectNoCertificateAuthoritiesExtension bool



	UseLegacySigningAlgorithm signatureAlgorithm


	SendServerHelloAsHelloRetryRequest bool


	RejectUnsolicitedKeyUpdate bool


	OmitExtensions bool


	EmptyExtensions bool


	ExpectOmitExtensions bool


	ExpectRecordSplitting bool


	PadClientHello int


	SendTLS13DowngradeRandom bool


	CheckTLS13DowngradeRandom bool


	IgnoreTLS13DowngradeRandom bool


	SendCompressedCoordinates bool


	ExpectRSAPSSSupport RSAPSSSupport


	SetX25519HighBit bool


	DuplicateCompressedCertAlgs bool


	ExpectedCompressedCert uint16


	SendCertCompressionAlgId uint16


	SendCertUncompressedLength uint32









	SendClientHelloWithFixes []byte


	SendJDK11DowngradeRandom bool


	ExpectJDK11DowngradeRandom bool


	FailIfHelloRetryRequested bool


	FailIfCECPQ2Offered bool


	ExpectedKeyShares []CurveID


	ExpectDelegatedCredentials bool


	FailIfDelegatedCredentials bool


	DisableDelegatedCredentials bool


	ExpectPQExperimentSignal bool
}

func (c *Config) serverInit() {
	if c.SessionTicketsDisabled {
		return
	}

	for _, b := range c.SessionTicketKey {
		if b != 0 {
			return
		}
	}

	if _, err := io.ReadFull(c.rand(), c.SessionTicketKey[:]); err != nil {
		c.SessionTicketsDisabled = true
	}
}

func (c *Config) rand() io.Reader {
	r := c.Rand
	if r == nil {
		return rand.Reader
	}
	return r
}

func (c *Config) time() time.Time {
	t := c.Time
	if t == nil {
		t = time.Now
	}
	return t()
}

func (c *Config) cipherSuites() []uint16 {
	s := c.CipherSuites
	if s == nil {
		s = defaultCipherSuites()
	}
	return s
}

func (c *Config) minVersion(isDTLS bool) uint16 {
	ret := uint16(minVersion)
	if c != nil && c.MinVersion != 0 {
		ret = c.MinVersion
	}
	if isDTLS {

		if ret < VersionTLS10 {
			return VersionTLS10
		}

		if ret == VersionTLS11 {
			return VersionTLS12
		}
	}
	return ret
}

func (c *Config) maxVersion(isDTLS bool) uint16 {
	ret := uint16(maxVersion)
	if c != nil && c.MaxVersion != 0 {
		ret = c.MaxVersion
	}
	if isDTLS {

		if ret > VersionTLS12 {
			return VersionTLS12
		}

		if ret == VersionTLS11 {
			return VersionTLS10
		}
	}
	return ret
}

var defaultCurvePreferences = []CurveID{CurveCECPQ2b, CurveCECPQ2, CurveX25519, CurveP256, CurveP384, CurveP521}

func (c *Config) curvePreferences() []CurveID {
	if c == nil || len(c.CurvePreferences) == 0 {
		return defaultCurvePreferences
	}
	return c.CurvePreferences
}

func (c *Config) defaultCurves() map[CurveID]bool {
	defaultCurves := make(map[CurveID]bool)
	curves := c.DefaultCurves
	if c == nil || c.DefaultCurves == nil {
		curves = c.curvePreferences()
	}
	for _, curveID := range curves {
		defaultCurves[curveID] = true
	}
	return defaultCurves
}

func wireToVersion(vers uint16, isDTLS bool) (uint16, bool) {
	if isDTLS {
		switch vers {
		case VersionDTLS12:
			return VersionTLS12, true
		case VersionDTLS10:
			return VersionTLS10, true
		}
	} else {
		switch vers {
		case VersionSSL30, VersionTLS10, VersionTLS11, VersionTLS12, VersionTLS13:
			return vers, true
		}
	}

	return 0, false
}

// it returns true and the corresponding protocol version. Otherwise, it returns
// false.
func (c *Config) isSupportedVersion(wireVers uint16, isDTLS bool) (uint16, bool) {
	vers, ok := wireToVersion(wireVers, isDTLS)
	if !ok || c.minVersion(isDTLS) > vers || vers > c.maxVersion(isDTLS) {
		return 0, false
	}
	return vers, true
}

func (c *Config) supportedVersions(isDTLS bool) []uint16 {
	versions := allTLSWireVersions
	if isDTLS {
		versions = allDTLSWireVersions
	}
	var ret []uint16
	for _, vers := range versions {
		if _, ok := c.isSupportedVersion(vers, isDTLS); ok {
			ret = append(ret, vers)
		}
	}
	return ret
}

// defaulting to the first element of c.Certificates if there are no good
// options.
func (c *Config) getCertificateForName(name string) *Certificate {
	if len(c.Certificates) == 1 || c.NameToCertificate == nil {

		return &c.Certificates[0]
	}

	name = strings.ToLower(name)
	for len(name) > 0 && name[len(name)-1] == '.' {
		name = name[:len(name)-1]
	}

	if cert, ok := c.NameToCertificate[name]; ok {
		return cert
	}


	labels := strings.Split(name, ".")
	for i := range labels {
		labels[i] = "*"
		candidate := strings.Join(labels, ".")
		if cert, ok := c.NameToCertificate[candidate]; ok {
			return cert
		}
	}

	return &c.Certificates[0]
}

func (c *Config) signSignatureAlgorithms() []signatureAlgorithm {
	if c != nil && c.SignSignatureAlgorithms != nil {
		return c.SignSignatureAlgorithms
	}
	return supportedSignatureAlgorithms
}

func (c *Config) verifySignatureAlgorithms() []signatureAlgorithm {
	if c != nil && c.VerifySignatureAlgorithms != nil {
		return c.VerifySignatureAlgorithms
	}
	return supportedSignatureAlgorithms
}

// from the CommonName and SubjectAlternateName fields of each of the leaf
// certificates.
func (c *Config) BuildNameToCertificate() {
	c.NameToCertificate = make(map[string]*Certificate)
	for i := range c.Certificates {
		cert := &c.Certificates[i]
		x509Cert, err := x509.ParseCertificate(cert.Certificate[0])
		if err != nil {
			continue
		}
		if len(x509Cert.Subject.CommonName) > 0 {
			c.NameToCertificate[x509Cert.Subject.CommonName] = cert
		}
		for _, san := range x509Cert.DNSNames {
			c.NameToCertificate[san] = cert
		}
	}
}

type Certificate struct {
	Certificate [][]byte
	PrivateKey  crypto.PrivateKey // supported types: *rsa.PrivateKey, *ecdsa.PrivateKey


	OCSPStaple []byte



	SignedCertificateTimestampList []byte




	Leaf *x509.Certificate
}

type record struct {
	contentType  recordType
	major, minor uint8
	payload      []byte
}

type handshakeMessage interface {
	marshal() []byte
	unmarshal([]byte) bool
}

// that uses an LRU caching strategy.
type lruSessionCache struct {
	sync.Mutex

	m        map[string]*list.Element
	q        *list.List
	capacity int
}

type lruSessionCacheEntry struct {
	sessionKey string
	state      interface{}
}

func (c *lruSessionCache) Put(sessionKey string, cs interface{}) {
	c.Lock()
	defer c.Unlock()

	if elem, ok := c.m[sessionKey]; ok {
		entry := elem.Value.(*lruSessionCacheEntry)
		entry.state = cs
		c.q.MoveToFront(elem)
		return
	}

	if c.q.Len() < c.capacity {
		entry := &lruSessionCacheEntry{sessionKey, cs}
		c.m[sessionKey] = c.q.PushFront(entry)
		return
	}

	elem := c.q.Back()
	entry := elem.Value.(*lruSessionCacheEntry)
	delete(c.m, entry.sessionKey)
	entry.sessionKey = sessionKey
	entry.state = cs
	c.q.MoveToFront(elem)
	c.m[sessionKey] = elem
}

// false) if no value is found.
func (c *lruSessionCache) Get(sessionKey string) (interface{}, bool) {
	c.Lock()
	defer c.Unlock()

	if elem, ok := c.m[sessionKey]; ok {
		c.q.MoveToFront(elem)
		return elem.Value.(*lruSessionCacheEntry).state, true
	}
	return nil, false
}

// uses an LRU caching strategy.
type lruClientSessionCache struct {
	lruSessionCache
}

func (c *lruClientSessionCache) Put(sessionKey string, cs *ClientSessionState) {
	c.lruSessionCache.Put(sessionKey, cs)
}

func (c *lruClientSessionCache) Get(sessionKey string) (*ClientSessionState, bool) {
	cs, ok := c.lruSessionCache.Get(sessionKey)
	if !ok {
		return nil, false
	}
	return cs.(*ClientSessionState), true
}

// uses an LRU caching strategy.
type lruServerSessionCache struct {
	lruSessionCache
}

func (c *lruServerSessionCache) Put(sessionId string, session *sessionState) {
	c.lruSessionCache.Put(sessionId, session)
}

func (c *lruServerSessionCache) Get(sessionId string) (*sessionState, bool) {
	cs, ok := c.lruSessionCache.Get(sessionId)
	if !ok {
		return nil, false
	}
	return cs.(*sessionState), true
}

// capacity that uses an LRU strategy. If capacity is < 1, a default capacity
// is used instead.
func NewLRUClientSessionCache(capacity int) ClientSessionCache {
	const defaultSessionCacheCapacity = 64

	if capacity < 1 {
		capacity = defaultSessionCacheCapacity
	}
	return &lruClientSessionCache{
		lruSessionCache{
			m:        make(map[string]*list.Element),
			q:        list.New(),
			capacity: capacity,
		},
	}
}

// capacity that uses an LRU strategy. If capacity is < 1, a default capacity
// is used instead.
func NewLRUServerSessionCache(capacity int) ServerSessionCache {
	const defaultSessionCacheCapacity = 64

	if capacity < 1 {
		capacity = defaultSessionCacheCapacity
	}
	return &lruServerSessionCache{
		lruSessionCache{
			m:        make(map[string]*list.Element),
			q:        list.New(),
			capacity: capacity,
		},
	}
}

type dsaSignature struct {
	R, S *big.Int
}

type ecdsaSignature dsaSignature

var emptyConfig Config

func defaultConfig() *Config {
	return &emptyConfig
}

var (
	once                   sync.Once
	varDefaultCipherSuites []uint16
)

func defaultCipherSuites() []uint16 {
	once.Do(initDefaultCipherSuites)
	return varDefaultCipherSuites
}

func initDefaultCipherSuites() {
	for _, suite := range cipherSuites {
		if suite.flags&suitePSK == 0 {
			varDefaultCipherSuites = append(varDefaultCipherSuites, suite.id)
		}
	}
}

func unexpectedMessageError(wanted, got interface{}) error {
	return fmt.Errorf("tls: received unexpected handshake message of type %T when waiting for %T", got, wanted)
}

func isSupportedSignatureAlgorithm(sigAlg signatureAlgorithm, sigAlgs []signatureAlgorithm) bool {
	for _, s := range sigAlgs {
		if s == sigAlg {
			return true
		}
	}
	return false
}

var (

	downgradeTLS13 = []byte{0x44, 0x4f, 0x57, 0x4e, 0x47, 0x52, 0x44, 0x01}
	downgradeTLS12 = []byte{0x44, 0x4f, 0x57, 0x4e, 0x47, 0x52, 0x44, 0x00}

	downgradeJDK11 = []byte{0xed, 0xbf, 0xb4, 0xa8, 0xc2, 0x47, 0x10, 0xff}
)

func containsGREASE(values []uint16) bool {
	for _, v := range values {
		if isGREASEValue(v) {
			return true
		}
	}
	return false
}

func checkRSAPSSSupport(support RSAPSSSupport, sigAlgs, sigAlgsCert []signatureAlgorithm) error {
	if sigAlgsCert == nil {
		sigAlgsCert = sigAlgs
	} else if eqSignatureAlgorithms(sigAlgs, sigAlgsCert) {

		return errors.New("tls: signature_algorithms and signature_algorithms_cert extensions were identical")
	}

	if support == RSAPSSSupportAny {
		return nil
	}

	var foundPSS, foundPSSCert bool
	for _, sigAlg := range sigAlgs {
		if sigAlg == signatureRSAPSSWithSHA256 || sigAlg == signatureRSAPSSWithSHA384 || sigAlg == signatureRSAPSSWithSHA512 {
			foundPSS = true
			break
		}
	}
	for _, sigAlg := range sigAlgsCert {
		if sigAlg == signatureRSAPSSWithSHA256 || sigAlg == signatureRSAPSSWithSHA384 || sigAlg == signatureRSAPSSWithSHA512 {
			foundPSSCert = true
			break
		}
	}

	expectPSS := support != RSAPSSSupportNone
	if foundPSS != expectPSS {
		if expectPSS {
			return errors.New("tls: peer did not support PSS")
		} else {
			return errors.New("tls: peer unexpectedly supported PSS")
		}
	}

	expectPSSCert := support == RSAPSSSupportBoth
	if foundPSSCert != expectPSSCert {
		if expectPSSCert {
			return errors.New("tls: peer did not support PSS in certificates")
		} else {
			return errors.New("tls: peer unexpectedly supported PSS in certificates")
		}
	}

	return nil
}
