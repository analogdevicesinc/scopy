// I2C protocol-data encoder: emits text-format lines with `# i2c:` pseudo-
// comments (start / repeat-start / stop / addr-{read,write}=0xNN) plus bare
// "0xNN" data lines. Synth defaults: samplerate 1M, bitrate 100k.
//
// ACK handling: the input module NACKs every byte unless a preceding
// `# i2c: ack-next=<n>` says otherwise, while the upstream i2c PD reports
// ack/nack *after* the byte it acknowledges. So each payload symbol looks
// ahead for its acknowledge symbol and emits ack-next=1 before the byte.
// Skipping this turns every ACK in the capture into a NACK downstream.

#include "ProtocolDataEncoder.h"

#include <QByteArray>
#include <QString>

namespace scopy {
namespace decoder {
namespace sigrok {
namespace {

class I2cEncoder : public IProtocolDataEncoder
{
public:
	QString downstreamId() const override { return QStringLiteral("i2c"); }

	bool encode(const std::vector<ExtractedSymbol> &sym,
	            const AnnInOptions &opts,
	            ProtocolDataInput &out,
	            QString *err) override
	{
		(void)err;

		const double  samplerate = opts.synthSampleRate > 0.0 ? opts.synthSampleRate : 1.0e6;
		const quint64 bitrate    = opts.synthBitrate > 0 ? opts.synthBitrate : 100000;

		out.inputOpts = QStringLiteral(
			"protocoldata:protocol=i2c:samplerate=%1:bitrate=%2:"
			"textinput=text-format")
			.arg(QString::number(samplerate, 'f', 0), QString::number(bitrate));

		out.stdinBytes.clear();
		out.anchors.clear();

		// Regenerated wire channel names: scl, sda.
		out.rootChannelOverrides["scl"] = "scl";
		out.rootChannelOverrides["sda"] = "sda";

		// Only payload lines are anchored: START/STOP/ack-next produce framing
		// on the wire that no single upstream annotation owns, and anchoring
		// them would let a downstream range snap to the wrong record.
		auto appendRaw = [&](const QByteArray &line) { out.stdinBytes.append(line); };
		auto appendPayload = [&](const QByteArray &line, int annIdx) {
			out.stdinBytes.append(line);
			out.anchors.push_back(SymbolAnchor{annIdx});
		};

		appendRaw(QByteArrayLiteral("# textinput: radix=hex\n"));

		// True if the next acknowledge symbol after `i` is an ACK. A payload
		// symbol in between means this byte's acknowledge was never reported,
		// so assume ACK (the common case) rather than inventing a NACK.
		auto ackFollows = [&](std::size_t i) -> bool {
			for(std::size_t j = i + 1; j < sym.size(); ++j) {
				switch(sym[j].tag) {
				case SymbolTag::Ack:  return true;
				case SymbolTag::Nack: return false;
				case SymbolTag::Data:
				case SymbolTag::AddrRead:
				case SymbolTag::AddrWrite:
					return true;
				default:
					break;
				}
			}
			return true;
		};

		for(std::size_t i = 0; i < sym.size(); ++i) {
			const ExtractedSymbol &s = sym[i];
			QByteArray             line;
			bool                   isPayload = false;

			switch(s.tag) {
			case SymbolTag::Start:
				line = QByteArrayLiteral("# i2c: start\n");
				break;
			case SymbolTag::RepeatStart:
				line = QByteArrayLiteral("# i2c: repeat-start\n");
				break;
			case SymbolTag::Stop:
				line = QByteArrayLiteral("# i2c: stop\n");
				break;
			case SymbolTag::AddrWrite:
				line = QStringLiteral("# i2c: addr-write=0x%1\n")
				               .arg(s.byte, 2, 16, QLatin1Char('0'))
				               .toUtf8();
				isPayload = true;
				break;
			case SymbolTag::AddrRead:
				line = QStringLiteral("# i2c: addr-read=0x%1\n")
				               .arg(s.byte, 2, 16, QLatin1Char('0'))
				               .toUtf8();
				isPayload = true;
				break;
			case SymbolTag::Data:
				line = QStringLiteral("0x%1\n")
				               .arg(s.byte, 2, 16, QLatin1Char('0'))
				               .toUtf8();
				isPayload = true;
				break;
			default:
				// Ack/Nack are consumed by ackFollows() above.
				continue;
			}

			if(isPayload) {
				if(ackFollows(i))
					appendRaw(QByteArrayLiteral("# i2c: ack-next=1\n"));
				appendPayload(line, s.upstreamAnnIndex);
			} else {
				appendRaw(line);
			}
		}

		return true;
	}
};

} // namespace

std::unique_ptr<IProtocolDataEncoder> makeI2cEncoder()
{
	return std::make_unique<I2cEncoder>();
}

} // namespace sigrok
} // namespace decoder
} // namespace scopy
