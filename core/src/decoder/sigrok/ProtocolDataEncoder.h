#pragma once

// PRIVATE to SigrokCliBackend. Not installed, not exported.
//
// Feeding annotations to a sigrok PD means re-synthesizing a waveform for
// libsigrok's `protocoldata` input module and piping it to sigrok-cli's stdin.
// That is one backend's implementation strategy, not part of the decoder
// abstraction: another backend consuming ExtractedSymbols would hand them to
// its own library directly. So this header stays under src/.

#include <core/decoder/AnnotationSymbols.h>

#include <QByteArray>
#include <QString>
#include <QStringList>

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace scopy {
namespace decoder {
namespace sigrok {

// One payload slot written to `stdinBytes`, in emission order, and the
// upstream annotation it came from. "Slot" is one line of protocoldata text,
// which is one wire word: SPI puts a MOSI and a MISO byte on the same line and
// so contributes one anchor, not two. The backend re-reads the regenerated wire
// through the root PD and pairs those readings with this list by rank, which is
// how a downstream sample range gets back to a real upstream one.
//
// Framing (SPI cs-assert/release, I2C start/stop) is deliberately not anchored:
// it emits no bytes, so it has no slot, and a downstream range that extends
// over framing therefore snaps to the nearest payload word. Bit-level
// annotations collapse to their containing byte for the same reason — an
// annotation stream records where each byte was, never where a bit sat inside
// it. Both are limits of the input, not approximations we chose.
struct SymbolAnchor
{
	int upstreamAnnIndex{-1};
};

// Input for one `sigrok-cli -i - -I <inputOpts>` invocation.
// rootChannelOverrides binds root-stage roles to sigrok's virtual
// protocoldata channel names (uart-rx0, i2c-scl0, spi-clk0, ...) instead of
// raw DIO bit indices.
struct ProtocolDataInput
{
	QByteArray                         stdinBytes;
	QString                            inputOpts;
	std::vector<SymbolAnchor>          anchors;
	std::map<std::string, std::string> rootChannelOverrides;
};

// Turns ExtractedSymbols into a ProtocolDataInput for one downstream root
// protocol. Registered by downstream root decoder id. Stateless.
class IProtocolDataEncoder
{
public:
	virtual ~IProtocolDataEncoder() = default;

	virtual QString downstreamId() const = 0;

	virtual bool encode(const std::vector<ExtractedSymbol> &sym,
	                    const AnnInOptions &opts,
	                    ProtocolDataInput &out,
	                    QString *err) = 0;
};

// Maps downstream root decoder id -> encoder.
class ProtocolDataEncoderRegistry
{
public:
	ProtocolDataEncoderRegistry() = default;

	ProtocolDataEncoderRegistry(const ProtocolDataEncoderRegistry &) = delete;
	ProtocolDataEncoderRegistry &operator=(const ProtocolDataEncoderRegistry &) = delete;

	void registerEncoder(std::unique_ptr<IProtocolDataEncoder> e);
	IProtocolDataEncoder *find(const QString &downstreamId) const;
	QStringList           downstreamIds() const;

	// uart, spi and i2c.
	void registerBuiltins();

private:
	std::unordered_map<std::string, std::unique_ptr<IProtocolDataEncoder>> m_encoders;
};

std::unique_ptr<IProtocolDataEncoder> makeUartEncoder();
std::unique_ptr<IProtocolDataEncoder> makeSpiEncoder();
std::unique_ptr<IProtocolDataEncoder> makeI2cEncoder();

// Root-stage decoder options the synthetic waveform forces. protocoldata
// emits an idealized signal (one sample per bit, fixed framing), so the root
// PD must be told to read it that way regardless of how the user configured
// the *upstream* capture. Empty for decoders with nothing to force.
std::map<std::string, std::string> downstreamOptionOverrides(const QString &downstreamId);

} // namespace sigrok
} // namespace decoder
} // namespace scopy
