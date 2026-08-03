// UART protocol-data encoder: ExtractedSymbol(Data) → `-I protocoldata` input.
// Synth defaults: samplerate 10M, bitrate 115200, frameformat "8n1".
//
// Known limit — inter-byte idle is not reproducible. protocoldata emits frames
// back to back with a fixed inter-frame gap and offers no option to widen it,
// and an annotation stream carries no idle records to widen it from. Byte
// values and order survive; the silence between them does not. Stacked PDs that
// frame on *timing* rather than on content therefore disagree with a real
// capture: modbus RTU, whose message boundary is a 3.5-character idle, reads
// the whole stream as one long frame here and as several frames on the wire.
// Content-framed PDs (i2c/spi upper layers, uart-based text protocols) are
// unaffected. Use stacked decoding when the framing is temporal.

#include "ProtocolDataEncoder.h"

#include <QString>

namespace scopy {
namespace decoder {
namespace sigrok {
namespace {

class UartEncoder : public IProtocolDataEncoder
{
public:
	QString downstreamId() const override { return QStringLiteral("uart"); }

	bool encode(const std::vector<ExtractedSymbol> &sym,
	            const AnnInOptions &opts,
	            ProtocolDataInput &out,
	            QString *err) override
	{
		(void)err;

		const double  samplerate  = opts.synthSampleRate > 0.0 ? opts.synthSampleRate : 1.0e7;
		const quint64 bitrate     = opts.synthBitrate > 0 ? opts.synthBitrate : 115200;
		const QString frameformat = opts.frameFormat.isEmpty()
			? QStringLiteral("8n1") : opts.frameFormat;

		out.inputOpts = QStringLiteral(
			"protocoldata:protocol=uart:samplerate=%1:bitrate=%2:"
			"frameformat=%3:textinput=raw-bytes")
			.arg(QString::number(samplerate, 'f', 0),
			     QString::number(bitrate),
			     frameformat);

		out.stdinBytes.clear();
		out.stdinBytes.reserve(static_cast<int>(sym.size()));
		out.anchors.clear();
		out.anchors.reserve(sym.size());

		// sigrok's `-I protocoldata:protocol=uart` emits a single wire "rxtx".
		out.rootChannelOverrides["rx"] = "rxtx";
		out.rootChannelOverrides["tx"] = "rxtx";

		for(const ExtractedSymbol &s : sym) {
			if(s.tag != SymbolTag::Data) continue;
			out.stdinBytes.append(static_cast<char>(s.byte));
			out.anchors.push_back(SymbolAnchor{s.upstreamAnnIndex});
		}

		return true;
	}
};

} // namespace

std::unique_ptr<IProtocolDataEncoder> makeUartEncoder()
{
	return std::make_unique<UartEncoder>();
}

} // namespace sigrok
} // namespace decoder
} // namespace scopy
