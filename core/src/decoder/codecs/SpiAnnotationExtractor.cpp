// SPI annotation extractor: emits one ExtractedSymbol per word ann.
// Opts: mosiKlass/misoKlass/dataKlass whitelists, radix ("hex"/"dec"/"bin").
// Output tag = "mosi" or "miso"; single-direction "data" maps to mosi.

#include "decoder/codecs/BuiltinCodecs.h"

#include <QString>
#include <QStringList>

namespace scopy {
namespace decoder {
namespace codecs {
namespace {

class SpiExtractor : public IAnnotationExtractor
{
public:
	QString upstreamId() const override { return QStringLiteral("spi"); }

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

		const QStringList mosiWL = getList("mosiKlass",
			QStringList{QStringLiteral("mosi-data"), QStringLiteral("mosi")});
		const QStringList misoWL = getList("misoKlass",
			QStringList{QStringLiteral("miso-data"), QStringLiteral("miso")});
		const QStringList dataWL = getList("dataKlass",
			QStringList{QStringLiteral("data")});

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

			QString tag;
			if(mosiWL.contains(klass))       tag = QStringLiteral("mosi");
			else if(misoWL.contains(klass))  tag = QStringLiteral("miso");
			else if(dataWL.contains(klass))  tag = QStringLiteral("mosi");
			else continue;

			const QString text = QString::fromStdString(a.text).trimmed();
			QString tok = text;
			const int sp = text.lastIndexOf(' ');
			if(sp >= 0) tok = text.mid(sp + 1);
			if(tok.startsWith(QStringLiteral("0x"), Qt::CaseInsensitive))
				tok = tok.mid(2);

			bool ok = false;
			const unsigned v = tok.toUInt(&ok, radix);
			if(!ok) {
				if(err) *err = QStringLiteral(
					"SpiExtractor: cannot parse '%1' as radix %2")
					.arg(text).arg(radix);
				return false;
			}

			ExtractedSymbol s;
			s.byte = static_cast<uint8_t>(v & 0xFFu);
			s.tag  = tag;
			s.upstreamAnnIndex = static_cast<int>(i);
			out.push_back(s);
		}

		return true;
	}
};

} // namespace

std::unique_ptr<IAnnotationExtractor> makeSpiExtractor()
{
	return std::make_unique<SpiExtractor>();
}

} // namespace codecs
} // namespace decoder
} // namespace scopy
