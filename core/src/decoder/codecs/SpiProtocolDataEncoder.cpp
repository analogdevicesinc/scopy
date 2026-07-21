// SPI protocol-data encoder: emits text-format lines "0xMM 0xSS" wrapped by
// cs-assert/cs-release markers (raw-bytes yields no SCK edges).
// Opts: samplerate (10M), bitrate (1M), direction ("mosi"/"miso"/"both").

#include "decoder/codecs/BuiltinCodecs.h"

#include <QByteArray>
#include <QString>

namespace scopy {
namespace decoder {
namespace codecs {
namespace {

class SpiEncoder : public IProtocolDataEncoder
{
public:
	QString downstreamId() const override { return QStringLiteral("spi"); }

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

		const QString samplerate = get("samplerate", QStringLiteral("10000000"));
		const QString bitrate    = get("bitrate",    QStringLiteral("1000000"));
		const QString direction  = get("direction",  QStringLiteral("both")).toLower();

		const bool useMosi = (direction != QStringLiteral("miso"));
		const bool useMiso = (direction != QStringLiteral("mosi"));

		out.inputOpts = QStringLiteral(
			"protocoldata:protocol=spi:samplerate=%1:bitrate=%2:"
			"textinput=text-format")
			.arg(samplerate, bitrate);

		out.stdinBytes.clear();
		out.byteToUpstreamAnn.clear();

		// Regenerated wire channel names: sck/miso/mosi/cs (clk -> sck).
		out.rootChannelOverrides["clk"]  = "sck";
		out.rootChannelOverrides["miso"] = "miso";
		out.rootChannelOverrides["mosi"] = "mosi";
		out.rootChannelOverrides["cs"]   = "cs";

		// Pair MOSI/MISO in arrival order; missing side is zero-filled.
		std::size_t iMosi = 0, iMiso = 0;
		const auto nextIdx = [&](const std::vector<ExtractedSymbol> &s,
		                         std::size_t &cursor,
		                         const QString &tag) -> int {
			for(; cursor < s.size(); ++cursor) {
				if(s[cursor].tag == tag) return static_cast<int>(cursor++);
			}
			return -1;
		};

		QByteArray body;
		body.append("# textinput: radix=hex\n");
		body.append("# spi: cs-assert\n");

		auto appendLine = [&](uint8_t mosi, uint8_t miso, int annIdx) {
			const QByteArray line = QByteArray()
				.append("0x")
				.append(QByteArray::number(mosi, 16).rightJustified(2, '0'))
				.append(' ')
				.append("0x")
				.append(QByteArray::number(miso, 16).rightJustified(2, '0'))
				.append('\n');
			body.append(line);
			for(int i = 0; i < line.size(); ++i)
				out.byteToUpstreamAnn.push_back(annIdx);
		};

		// Header lines anchored to the first upstream annotation.
		const int headerAnnIdx = sym.empty() ? -1 : sym.front().upstreamAnnIndex;
		for(int i = 0; i < body.size(); ++i)
			out.byteToUpstreamAnn.push_back(headerAnnIdx);

		while(true) {
			int mosiI = useMosi ? nextIdx(sym, iMosi, QStringLiteral("mosi")) : -1;
			int misoI = useMiso ? nextIdx(sym, iMiso, QStringLiteral("miso")) : -1;
			if(mosiI < 0 && misoI < 0) break;

			const uint8_t mosiByte = (mosiI >= 0) ? sym[mosiI].byte : 0;
			const uint8_t misoByte = (misoI >= 0) ? sym[misoI].byte : 0;
			const int annIdx = (mosiI >= 0) ? sym[mosiI].upstreamAnnIndex
			                                : sym[misoI].upstreamAnnIndex;
			appendLine(mosiByte, misoByte, annIdx);
		}

		const QByteArray tail("# spi: cs-release\n");
		body.append(tail);
		const int tailAnnIdx = sym.empty() ? -1 : sym.back().upstreamAnnIndex;
		for(int i = 0; i < tail.size(); ++i)
			out.byteToUpstreamAnn.push_back(tailAnnIdx);

		out.stdinBytes = body;
		return true;
	}
};

} // namespace

std::unique_ptr<IProtocolDataEncoder> makeSpiEncoder()
{
	return std::make_unique<SpiEncoder>();
}

} // namespace codecs
} // namespace decoder
} // namespace scopy
