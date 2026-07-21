// I2C annotation extractor: emits ordered framing/addr/data symbols.
// Tags: "start"/"start-repeat"/"stop"/"addr-r"/"addr-w"/"data".
// Opts: startKlass/stopKlass/addrR/addrW/dataKlass whitelists, radix.

#include "decoder/codecs/BuiltinCodecs.h"

#include <QString>
#include <QStringList>

namespace scopy {
namespace decoder {
namespace codecs {
namespace {

class I2cExtractor : public IAnnotationExtractor
{
public:
	QString upstreamId() const override { return QStringLiteral("i2c"); }

	bool extract(const std::vector<AnnotationC> &in,
	             const std::map<std::string, std::string> &opts,
	             std::vector<ExtractedSymbol> &out,
	             QString *err) override
	{
		out.clear();

		auto getList = [&](const char *key, const QStringList &def) -> QStringList {
			auto it = opts.find(key);
			if(it == opts.end() || it->second.empty()) return def;
			return QString::fromStdString(it->second).split(',');
		};

		const QStringList startWL = getList("startKlass",
			QStringList{QStringLiteral("start"), QStringLiteral("start-repeat")});
		const QStringList stopWL  = getList("stopKlass",
			QStringList{QStringLiteral("stop")});
		const QStringList addrRWL = getList("addrRKlass",
			QStringList{QStringLiteral("address-read")});
		const QStringList addrWWL = getList("addrWKlass",
			QStringList{QStringLiteral("address-write")});
		const QStringList dataWL  = getList("dataKlass",
			QStringList{QStringLiteral("data-read"),
			            QStringLiteral("data-write"),
			            QStringLiteral("data")});

		int radix = 16;
		auto itRadix = opts.find("radix");
		if(itRadix != opts.end()) {
			const QString r = QString::fromStdString(itRadix->second).toLower();
			if(r == QStringLiteral("dec"))      radix = 10;
			else if(r == QStringLiteral("hex")) radix = 16;
			else if(r == QStringLiteral("bin")) radix = 2;
		}

		for(std::size_t i = 0; i < in.size(); ++i) {
			const AnnotationC &a = in[i];
			const QString klass = QString::fromStdString(a.klass);

			ExtractedSymbol s;
			s.byte = 0;
			s.upstreamAnnIndex = static_cast<int>(i);

			if(startWL.contains(klass)) {
				s.tag = (klass == QStringLiteral("start-repeat"))
				                ? QStringLiteral("start-repeat")
				                : QStringLiteral("start");
				out.push_back(s);
				continue;
			}
			if(stopWL.contains(klass)) {
				s.tag = QStringLiteral("stop");
				out.push_back(s);
				continue;
			}

			QString tag;
			if(addrRWL.contains(klass))      tag = QStringLiteral("addr-r");
			else if(addrWWL.contains(klass)) tag = QStringLiteral("addr-w");
			else if(dataWL.contains(klass))  tag = QStringLiteral("data");
			else continue;

			const QString text = QString::fromStdString(a.text).trimmed();
			QString tok = text;
			const int sp = text.lastIndexOf(' ');
			if(sp >= 0) tok = text.mid(sp + 1);
			if(tok.startsWith(QStringLiteral("0x"), Qt::CaseInsensitive))
				tok = tok.mid(2);

			// Skip unparseable variants (short-form labels share the class).
			bool ok = false;
			const unsigned v = tok.toUInt(&ok, radix);
			if(!ok) continue;

			s.byte = static_cast<uint8_t>(v & 0xFFu);
			s.tag  = tag;
			out.push_back(s);
		}

		return true;
	}
};

} // namespace

std::unique_ptr<IAnnotationExtractor> makeI2cExtractor()
{
	return std::make_unique<I2cExtractor>();
}

} // namespace codecs
} // namespace decoder
} // namespace scopy
