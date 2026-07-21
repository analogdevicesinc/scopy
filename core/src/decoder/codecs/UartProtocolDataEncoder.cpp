// UART protocol-data encoder: ExtractedSymbol(data) → `-I protocoldata` input.
// Opts: samplerate (10M), bitrate (115200), frameformat ("8n1").

#include "decoder/codecs/BuiltinCodecs.h"

#include <QString>

namespace scopy {
namespace decoder {
namespace codecs {
namespace {

class UartEncoder : public IProtocolDataEncoder
{
public:
	QString downstreamId() const override { return QStringLiteral("uart"); }

	bool encode(const std::vector<ExtractedSymbol> &sym,
	            const std::map<std::string, std::string> &opts,
	            ProtocolDataInput &out,
	            QString *err) override
	{
		(void)err;

		auto get = [&](const char *k, const QString &def) -> QString {
			auto it = opts.find(k);
			return (it == opts.end() || it->second.empty())
			               ? def
			               : QString::fromStdString(it->second);
		};

		const QString samplerate  = get("samplerate",  QStringLiteral("10000000"));
		const QString bitrate     = get("bitrate",     QStringLiteral("115200"));
		const QString frameformat = get("frameformat", QStringLiteral("8n1"));

		out.inputOpts = QStringLiteral(
			"protocoldata:protocol=uart:samplerate=%1:bitrate=%2:"
			"frameformat=%3:textinput=raw-bytes")
			.arg(samplerate, bitrate, frameformat);

		out.stdinBytes.clear();
		out.stdinBytes.reserve(static_cast<int>(sym.size()));
		out.byteToUpstreamAnn.clear();
		out.byteToUpstreamAnn.reserve(sym.size());

		// sigrok's `-I protocoldata:protocol=uart` emits a single wire "rxtx".
		out.rootChannelOverrides["rx"] = "rxtx";
		out.rootChannelOverrides["tx"] = "rxtx";

		for(const ExtractedSymbol &s : sym) {
			if(s.tag != QStringLiteral("data")) continue;
			out.stdinBytes.append(static_cast<char>(s.byte));
			out.byteToUpstreamAnn.push_back(s.upstreamAnnIndex);
		}

		return true;
	}
};

} // namespace

std::unique_ptr<IProtocolDataEncoder> makeUartEncoder()
{
	return std::make_unique<UartEncoder>();
}

} // namespace codecs
} // namespace decoder
} // namespace scopy
