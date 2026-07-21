#pragma once

#include "scopy-core_export.h"
#include "IAnnotationExtractor.h"

#include <QByteArray>
#include <QString>

#include <map>
#include <string>
#include <vector>

namespace scopy {
namespace decoder {

// Input for one `sigrok-cli -i - -I <inputOpts>` invocation.
// byteToUpstreamAnn maps each stdin byte/symbol back to its source
// annotation index, so the backend can remap synthetic sample indices
// onto the upstream timeline (size may be 0 if unsupported).
// rootChannelOverrides binds root-stage roles to sigrok's virtual
// protocoldata channel names (uart-rx0, i2c-scl0, spi-clk0, ...) instead
// of raw DIO bit indices.
struct ProtocolDataInput
{
	QByteArray                         stdinBytes;
	QString                            inputOpts;
	std::vector<int>                   byteToUpstreamAnn;
	std::map<std::string, std::string> rootChannelOverrides;
};

// Turns ExtractedSymbols into a ProtocolDataInput for one downstream root
// protocol. Registered by downstream root decoder id. Stateless.
class SCOPY_CORE_EXPORT IProtocolDataEncoder
{
public:
	virtual ~IProtocolDataEncoder() = default;

	virtual QString downstreamId() const = 0;

	// opts = flat "annIn.*" section of DecoderConfig::meta (prefix stripped).
	virtual bool encode(const std::vector<ExtractedSymbol> &sym,
	                    const std::map<std::string, std::string> &opts,
	                    ProtocolDataInput &out,
	                    QString *err) = 0;
};

} // namespace decoder
} // namespace scopy
