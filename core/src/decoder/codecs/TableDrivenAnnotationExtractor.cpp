// Table-driven annotation extractor: one implementation for every upstream
// protocol whose codec differs only in {klass whitelist -> output tag} rules
// and whether each class carries a parseable byte (data/addr) or is a pure
// framing marker (i2c start/stop). Per-protocol behaviour lives in the
// factory tables at the bottom of this file.

#include "decoder/codecs/BuiltinCodecs.h"
#include "decoder/codecs/AnnotationTextParse.h"

#include <QString>
#include <QStringList>

#include <utility>
#include <vector>

namespace scopy {
namespace decoder {
namespace codecs {
namespace {

// One class-whitelist rule: if an incoming annotation's klass is in
// `defaultKlasses` (overridable via `optsKey` in the opts map), emit an
// ExtractedSymbol tagged `outTag`. `parseByte` selects the data/framing
// modes:
//   Framing (parseByte=false): emit with byte=0, no text parse.
//   Data    (parseByte=true) : parse text->byte; behaviour on failure is
//                              driven by `skipUnparseable`.
// `outTagOverrideFromKlass` (i2c only) lets one rule remap its own tag
// based on the matched klass (start vs start-repeat).
struct KlassRule
{
	const char *optsKey;                 // opts override key (nullptr = no override)
	QStringList defaultKlasses;          // default whitelist
	QString     outTag;                  // symbol tag to emit
	bool        parseByte;               // true = parse data byte; false = framing
	bool        skipUnparseable;         // parseByte=true only; skip vs. hard-error
	bool        outTagOverrideFromKlass; // set outTag from matched klass literally
};

struct ExtractorProfile
{
	QString              upstreamId;
	std::vector<KlassRule> rules;
	QString              fallbackErrorLabel; // for hard-error messages
};

class TableDrivenExtractor : public IAnnotationExtractor
{
public:
	explicit TableDrivenExtractor(ExtractorProfile profile)
		: m_profile(std::move(profile))
	{}

	QString upstreamId() const override { return m_profile.upstreamId; }

	bool extract(const std::vector<AnnotationC> &in,
	             const std::map<std::string, std::string> &opts,
	             std::vector<ExtractedSymbol> &out,
	             QString *err) override
	{
		out.clear();

		auto getList = [&](const char *key,
		                   const QStringList &def) -> QStringList {
			if(!key) return def;
			auto it = opts.find(key);
			if(it == opts.end() || it->second.empty()) return def;
			return QString::fromStdString(it->second).split(QLatin1Char(','));
		};

		// Resolve whitelists once.
		std::vector<QStringList> whitelists;
		whitelists.reserve(m_profile.rules.size());
		for(const KlassRule &r : m_profile.rules)
			whitelists.push_back(getList(r.optsKey, r.defaultKlasses));

		int radix = 16;
		auto itRadix = opts.find("radix");
		if(itRadix != opts.end())
			radix = radixFromString(QString::fromStdString(itRadix->second), 16);

		for(std::size_t i = 0; i < in.size(); ++i) {
			const AnnotationC &a = in[i];
			const QString klass = QString::fromStdString(a.klass);

			// First matching rule wins.
			int matched = -1;
			for(std::size_t r = 0; r < m_profile.rules.size(); ++r) {
				if(whitelists[r].contains(klass)) {
					matched = static_cast<int>(r);
					break;
				}
			}
			if(matched < 0) continue;

			const KlassRule &rule = m_profile.rules[matched];

			ExtractedSymbol s;
			s.byte             = 0;
			s.upstreamAnnIndex = static_cast<int>(i);
			s.tag              = rule.outTagOverrideFromKlass
			                             ? klass
			                             : rule.outTag;

			if(!rule.parseByte) {
				out.push_back(s);
				continue;
			}

			uint8_t byte = 0;
			if(!parseAnnotationByte(QString::fromStdString(a.text),
			                        radix, byte)) {
				if(rule.skipUnparseable) continue;
				if(err) *err = QStringLiteral(
					"%1: cannot parse '%2' as radix %3")
					.arg(m_profile.fallbackErrorLabel,
					     QString::fromStdString(a.text))
					.arg(radix);
				return false;
			}
			s.byte = byte;
			out.push_back(s);
		}

		return true;
	}

private:
	ExtractorProfile m_profile;
};

// ---- Per-protocol profiles -------------------------------------------------

ExtractorProfile uartProfile()
{
	ExtractorProfile p;
	p.upstreamId         = QStringLiteral("uart");
	p.fallbackErrorLabel = QStringLiteral("UartExtractor");
	p.rules = {
		// One "data" rule; UART hard-fails on unparseable payload.
		KlassRule{
			"klass",
			QStringList{QStringLiteral("data"),
			            QStringLiteral("rx-data"),
			            QStringLiteral("tx-data")},
			QStringLiteral("data"),
			/*parseByte=*/true,
			/*skipUnparseable=*/false,
			/*outTagOverrideFromKlass=*/false,
		},
	};
	return p;
}

ExtractorProfile spiProfile()
{
	ExtractorProfile p;
	p.upstreamId         = QStringLiteral("spi");
	p.fallbackErrorLabel = QStringLiteral("SpiExtractor");
	p.rules = {
		KlassRule{
			"mosiKlass",
			QStringList{QStringLiteral("mosi-data"), QStringLiteral("mosi")},
			QStringLiteral("mosi"),
			true, false, false,
		},
		KlassRule{
			"misoKlass",
			QStringList{QStringLiteral("miso-data"), QStringLiteral("miso")},
			QStringLiteral("miso"),
			true, false, false,
		},
		// Single-direction "data" class maps to mosi (matches legacy SpiExtractor).
		KlassRule{
			"dataKlass",
			QStringList{QStringLiteral("data")},
			QStringLiteral("mosi"),
			true, false, false,
		},
	};
	return p;
}

ExtractorProfile i2cProfile()
{
	ExtractorProfile p;
	p.upstreamId         = QStringLiteral("i2c");
	p.fallbackErrorLabel = QStringLiteral("I2cExtractor");
	p.rules = {
		// Framing: emit klass verbatim (start vs start-repeat), no parse.
		KlassRule{
			"startKlass",
			QStringList{QStringLiteral("start"),
			            QStringLiteral("start-repeat")},
			QStringLiteral("start"),
			/*parseByte=*/false,
			/*skipUnparseable=*/false,
			/*outTagOverrideFromKlass=*/true,
		},
		KlassRule{
			"stopKlass",
			QStringList{QStringLiteral("stop")},
			QStringLiteral("stop"),
			false, false, false,
		},
		// Data/addr: legacy I2C silently skips unparseable variants
		// (short-form labels share the same annotation class).
		KlassRule{
			"addrRKlass",
			QStringList{QStringLiteral("address-read")},
			QStringLiteral("addr-r"),
			true, /*skipUnparseable=*/true, false,
		},
		KlassRule{
			"addrWKlass",
			QStringList{QStringLiteral("address-write")},
			QStringLiteral("addr-w"),
			true, /*skipUnparseable=*/true, false,
		},
		KlassRule{
			"dataKlass",
			QStringList{QStringLiteral("data-read"),
			            QStringLiteral("data-write"),
			            QStringLiteral("data")},
			QStringLiteral("data"),
			true, /*skipUnparseable=*/true, false,
		},
	};
	return p;
}

} // namespace

std::unique_ptr<IAnnotationExtractor> makeUartExtractor()
{
	return std::unique_ptr<IAnnotationExtractor>(
		new TableDrivenExtractor(uartProfile()));
}

std::unique_ptr<IAnnotationExtractor> makeSpiExtractor()
{
	return std::unique_ptr<IAnnotationExtractor>(
		new TableDrivenExtractor(spiProfile()));
}

std::unique_ptr<IAnnotationExtractor> makeI2cExtractor()
{
	return std::unique_ptr<IAnnotationExtractor>(
		new TableDrivenExtractor(i2cProfile()));
}

} // namespace codecs
} // namespace decoder
} // namespace scopy
