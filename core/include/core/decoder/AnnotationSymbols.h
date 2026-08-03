#pragma once

// Backend-neutral contract for feeding one decoder's annotations into
// another decoder.
//
// This is the *whole* public surface for that feature. A new decoder
// implementation (vendor .so, in-process library, anything) participates by
// registering an IAnnotationExtractor for each upstream protocol it wants to
// consume. Everything past that — how symbols reach the actual decoder — is
// the backend's private business and must not appear here. The built-in
// sigrok-cli backend, for instance, re-synthesizes a waveform and pipes it to
// a child process; that machinery lives under src/decoder/sigrok/ and is
// deliberately not exported.

#include "scopy-core_export.h"
#include "IDecoderBackend.h"

#include <core/acq_engine/SampleBuffer.h>

#include <QString>
#include <QStringList>

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace scopy {
namespace decoder {

using scopy::acq::AnnotationStreamInfo;
using scopy::acq::TextRadix;

// ---------------------------------------------------------------------------
// Symbol vocabulary
// ---------------------------------------------------------------------------

// Typed rather than stringly-keyed on purpose: these values are matched
// across translation units, and a literal mismatch fails silently. (The
// upstream i2c class is "repeat-start", not "start-repeat"; spelling it
// wrong dropped every repeated START without a word of complaint.)
enum class SymbolTag
{
	Unknown = 0,

	// Payload tags — ExtractedSymbol::byte is meaningful.
	Data,      // direction-agnostic payload byte
	Mosi,      // SPI controller -> peripheral
	Miso,      // SPI peripheral -> controller
	AddrRead,  // I2C address, read transfer
	AddrWrite, // I2C address, write transfer

	// Framing tags — byte is unused.
	Start,       // I2C START
	RepeatStart, // I2C repeated START
	Stop,        // I2C STOP
	Ack,         // acknowledges the *preceding* payload byte
	Nack,
	TransferEnd, // closes a framed block (e.g. SPI CS-asserted window)
};

SCOPY_CORE_EXPORT const char *symbolTagName(SymbolTag tag);
// True for tags whose ExtractedSymbol::byte carries data.
SCOPY_CORE_EXPORT bool symbolTagCarriesByte(SymbolTag tag);

// One symbol recovered from an upstream annotation stream.
// upstreamAnnIndex indexes the input vector, so downstream results can be
// mapped back onto the upstream timeline.
struct ExtractedSymbol
{
	uint8_t   byte{0};
	SymbolTag tag{SymbolTag::Unknown};
	int       upstreamAnnIndex{-1};
};

// ---------------------------------------------------------------------------
// Stream descriptor <-> DecoderConfig::meta
// ---------------------------------------------------------------------------

// The producer's AnnotationStreamInfo travels to the backend inside
// DecoderConfig::meta under "annIn.stream.", so it reaches vendor libraries
// over the C ABI (which carries meta as JSON) without a signature change.
SCOPY_CORE_EXPORT void streamInfoToMeta(const AnnotationStreamInfo &info,
                                        std::map<std::string, std::string> &meta);
SCOPY_CORE_EXPORT AnnotationStreamInfo
streamInfoFromMeta(const std::map<std::string, std::string> &meta);

// ---------------------------------------------------------------------------
// Extraction inputs and accounting
// ---------------------------------------------------------------------------

// Options for one extract() call, and for whatever the backend does with the
// symbols afterwards. What describes the *upstream* stream is producer-
// declared (`stream`); the rest is consumer configuration.
struct AnnInOptions
{
	// Extractor to use. Defaults to stream.producerId, so a correct
	// producer needs no configuration at all.
	QString upstreamId;

	// Producer's description of the incoming stream. Authoritative unless
	// explicitly overridden in meta.
	AnnotationStreamInfo stream;

	// Hints for backends that regenerate a signal from the symbols (the
	// sigrok-cli path does; an in-process decoder would ignore them). These
	// describe the *fabricated* waveform, not the captured one — keeping them
	// apart from `stream` is what stops a synthetic 10 MHz regen rate from
	// overwriting the upstream timeline. 0 / empty = backend default.
	double  synthSampleRate{0.0};
	quint64 synthBitrate{0};
	QString frameFormat; // UART, e.g. "8n1"
	QString direction;   // SPI: "mosi" | "miso" | "both"

	// Per-rule annotation-class whitelist overrides. Any meta key ending in
	// "Klass" lands here (e.g. "mosiKlass" -> {"mosi-data"}); which ones an
	// extractor honours is its own business. Absent or empty => its default.
	std::map<std::string, QStringList> klassOverrides;

	// Keys under "annIn." that nothing recognized. Reported rather than
	// silently dropped, so a typo is visible.
	QStringList unknownKeys;

	// Recognized non-"*Klass" keys, minus the "annIn." prefix.
	static QStringList knownScalarKeys();

	// Build from DecoderConfig::meta (keys still carrying "annIn."). Explicit
	// meta wins over the producer's declaration, so a user can still override
	// a misreporting producer. Never fails; a malformed value keeps the
	// inherited setting and is appended to `warnings`.
	static AnnInOptions fromMeta(const std::map<std::string, std::string> &meta,
	                             QStringList *warnings = nullptr);
};

// Per-call accounting. Extraction skips what it cannot read instead of
// failing the buffer, so these counters are the only way a silent mismatch
// (wrong radix, unexpected class names) becomes visible.
struct ExtractStats
{
	int total{0};   // annotations examined
	int matched{0}; // matched a rule
	int emitted{0}; // symbols produced
	int ignored{0}; // matched no rule (bits, warnings, ...)

	// Matched a payload rule, but `text` held no numeral. Expected: sigrok's
	// i2c PD reuses the "address-write" class for the R/W-bit row ("Write").
	int noPayload{0};
	// Matched a payload rule and `text` looked numeric, but not in the declared
	// radix. This one is a real problem: the declaration is wrong, so the bytes
	// that *did* parse are silently wrong too ("105" is 0x69 or 0x105).
	int radixMismatch{0};

	bool    sawRadixMismatch() const { return radixMismatch > 0; }
	QString toString() const;
};

// ---------------------------------------------------------------------------
// Extractor interface and registry
// ---------------------------------------------------------------------------

// Turns an upstream decoder's annotations into ExtractedSymbols.
// Registered under the canonical upstream decoder id ("uart", "spi", "i2c").
// Stateless: one call handles one buffer.
class SCOPY_CORE_EXPORT IAnnotationExtractor
{
public:
	virtual ~IAnnotationExtractor() = default;

	virtual QString upstreamId() const = 0;

	// Best-effort. Returns false only on a genuine configuration error
	// (err populated); unreadable individual annotations are skipped and
	// counted in `stats`, which may be null.
	virtual bool extract(const std::vector<AnnotationC> &in,
	                     const AnnInOptions &opts,
	                     std::vector<ExtractedSymbol> &out,
	                     ExtractStats *stats,
	                     QString *err) = 0;
};

// Maps upstream decoder id -> extractor. Owned by a backend factory and
// passed non-owning to backends. Register a protocol by adding its extractor.
class SCOPY_CORE_EXPORT AnnotationExtractorRegistry
{
public:
	AnnotationExtractorRegistry() = default;

	AnnotationExtractorRegistry(const AnnotationExtractorRegistry &) = delete;
	AnnotationExtractorRegistry &operator=(const AnnotationExtractorRegistry &) = delete;

	void                  registerExtractor(std::unique_ptr<IAnnotationExtractor> e);
	IAnnotationExtractor *find(const QString &upstreamId) const;
	QStringList           upstreamIds() const;

	// uart, spi and i2c.
	void registerBuiltins();

private:
	// std::unordered_map: QHash needs copyable values, unique_ptr is move-only.
	std::unordered_map<std::string, std::unique_ptr<IAnnotationExtractor>> m_extractors;
};

// Built-in extractor factories.
SCOPY_CORE_EXPORT std::unique_ptr<IAnnotationExtractor> makeUartExtractor();
SCOPY_CORE_EXPORT std::unique_ptr<IAnnotationExtractor> makeSpiExtractor();
SCOPY_CORE_EXPORT std::unique_ptr<IAnnotationExtractor> makeI2cExtractor();

} // namespace decoder
} // namespace scopy
