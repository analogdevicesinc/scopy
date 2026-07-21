// I2C protocol-data encoder: emits text-format lines with `# i2c:` pseudo-
// comments (start / repeat-start / stop / addr-{read,write}=0xNN) plus bare
// "0xNN" data lines. Opts: samplerate (1M), bitrate (100k).

#include "decoder/codecs/BuiltinCodecs.h"

#include <QByteArray>
#include <QString>

namespace scopy {
namespace decoder {
namespace codecs {
namespace {

class I2cEncoder : public IProtocolDataEncoder
{
public:
	QString downstreamId() const override { return QStringLiteral("i2c"); }

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

		const QString samplerate = get("samplerate", QStringLiteral("1000000"));
		const QString bitrate    = get("bitrate",    QStringLiteral("100000"));

		out.inputOpts = QStringLiteral(
			"protocoldata:protocol=i2c:samplerate=%1:bitrate=%2:"
			"textinput=text-format")
			.arg(samplerate, bitrate);

		out.stdinBytes.clear();
		out.byteToUpstreamAnn.clear();

		// Regenerated wire channel names: scl, sda.
		out.rootChannelOverrides["scl"] = "scl";
		out.rootChannelOverrides["sda"] = "sda";

		auto appendLine = [&](const QByteArray &line, int upstream) {
			out.stdinBytes.append(line);
			for(int i = 0; i < line.size(); ++i)
				out.byteToUpstreamAnn.push_back(upstream);
		};

		const int headerAnnIdx = sym.empty() ? -1 : sym.front().upstreamAnnIndex;
		appendLine(QByteArrayLiteral("# textinput: radix=hex\n"), headerAnnIdx);

		for(const ExtractedSymbol &s : sym) {
			QByteArray line;
			if(s.tag == QStringLiteral("start")) {
				line = QByteArrayLiteral("# i2c: start\n");
			} else if(s.tag == QStringLiteral("start-repeat")) {
				line = QByteArrayLiteral("# i2c: repeat-start\n");
			} else if(s.tag == QStringLiteral("stop")) {
				line = QByteArrayLiteral("# i2c: stop\n");
			} else if(s.tag == QStringLiteral("addr-w")) {
				line = QStringLiteral("# i2c: addr-write=0x%1\n")
				               .arg(s.byte, 2, 16, QLatin1Char('0'))
				               .toUtf8();
			} else if(s.tag == QStringLiteral("addr-r")) {
				line = QStringLiteral("# i2c: addr-read=0x%1\n")
				               .arg(s.byte, 2, 16, QLatin1Char('0'))
				               .toUtf8();
			} else if(s.tag == QStringLiteral("data")) {
				line = QStringLiteral("0x%1\n")
				               .arg(s.byte, 2, 16, QLatin1Char('0'))
				               .toUtf8();
			} else {
				continue;
			}
			appendLine(line, s.upstreamAnnIndex);
		}

		return true;
	}
};

} // namespace

std::unique_ptr<IProtocolDataEncoder> makeI2cEncoder()
{
	return std::make_unique<I2cEncoder>();
}

} // namespace codecs
} // namespace decoder
} // namespace scopy
