// UART annotation extractor: sigrok "uart" PD data anns → one byte each.
// Opts: klass (default "data,rx-data,tx-data"), radix ("hex"|"dec"|"bin").

#include "decoder/codecs/BuiltinCodecs.h"

#include <QString>
#include <QStringList>

namespace scopy {
namespace decoder {
namespace codecs {
namespace {

class UartExtractor : public IAnnotationExtractor
{
public:
	QString upstreamId() const override { return QStringLiteral("uart"); }

	bool extract(const std::vector<AnnotationC> &in,
	             const std::map<std::string, std::string> &opts,
	             std::vector<ExtractedSymbol> &out,
	             QString *err) override
	{
		out.clear();

		QStringList whitelist;
		auto itKlass = opts.find("klass");
		if(itKlass != opts.end() && !itKlass->second.empty()) {
			whitelist = QString::fromStdString(itKlass->second).split(',');
		} else {
			whitelist = QStringList{QStringLiteral("data"),
			                        QStringLiteral("rx-data"),
			                        QStringLiteral("tx-data")};
		}

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
			if(!whitelist.contains(klass)) continue;

			// Parse last whitespace token as integer in the given radix.
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
					"UartExtractor: cannot parse '%1' as radix %2")
					.arg(text).arg(radix);
				return false;
			}

			ExtractedSymbol s;
			s.byte = static_cast<uint8_t>(v & 0xFFu);
			s.tag  = QStringLiteral("data");
			s.upstreamAnnIndex = static_cast<int>(i);
			out.push_back(s);
		}

		return true;
	}
};

} // namespace

std::unique_ptr<IAnnotationExtractor> makeUartExtractor()
{
	return std::make_unique<UartExtractor>();
}

} // namespace codecs
} // namespace decoder
} // namespace scopy
