// Table-driven annotation extractor: one implementation for every upstream
// protocol whose extraction differs only in {klass whitelist -> SymbolTag}
// rules and whether each class carries a payload byte or is a pure framing
// marker. Per-protocol behaviour lives in the profile tables at the bottom.
//
// A new (non-sigrok) decoder that wants to consume annotations does not have
// to reuse this: it implements IAnnotationExtractor however it likes. This is
// just the shape that covers class-whitelist protocols with no code.

#include "decoder/AnnotationSymbols.h"

#include "AnnotationTextParse.h"

#include <QString>
#include <QStringList>

#include <utility>
#include <vector>

namespace scopy {
namespace decoder {
namespace {

// One class-whitelist rule: if an incoming annotation's klass is in
// `defaultKlasses` (overridable via AnnInOptions::klassOverrides[optsKey]),
// emit an ExtractedSymbol tagged `outTag`.
//
// symbolTagCarriesByte(outTag) decides whether a payload is read: framing
// tags emit byte=0 with no parse. `tagFromKlass` lets one rule pick its tag
// from the matched class instead (start vs repeat-start, ack vs nack) — the
// whitelist and the tag set then stay in one place.
struct KlassRule
{
	const char *optsKey;        // klassOverrides key (nullptr = not overridable)
	QStringList defaultKlasses; // default whitelist
	SymbolTag   outTag;         // tag to emit, unless tagFromKlass
	bool        tagFromKlass{false};
};

struct ExtractorProfile
{
	QString                upstreamId;
	std::vector<KlassRule> rules;
};

// Class-id spelling is the one thing that must match sigrok exactly, and a
// mismatch is invisible: the class simply never matches and its symbols
// vanish. Deriving the tag from the id via the same table used to whitelist
// it means one name, checked once.
SymbolTag tagFromKlassName(const QString &klass)
{
	if(klass == QStringLiteral("start"))         return SymbolTag::Start;
	if(klass == QStringLiteral("repeat-start"))  return SymbolTag::RepeatStart;
	if(klass == QStringLiteral("stop"))          return SymbolTag::Stop;
	if(klass == QStringLiteral("ack"))           return SymbolTag::Ack;
	if(klass == QStringLiteral("nack"))          return SymbolTag::Nack;
	return SymbolTag::Unknown;
}

class TableDrivenExtractor : public IAnnotationExtractor
{
public:
	explicit TableDrivenExtractor(ExtractorProfile profile)
		: m_profile(std::move(profile))
	{}

	QString upstreamId() const override { return m_profile.upstreamId; }

	// Best-effort by design. An unreadable annotation used to fail the whole
	// buffer, which turns one bad byte (or one wrong radix) into "the decoder
	// produced nothing" with no indication why. Now it is skipped and counted;
	// the caller logs the counters.
	bool extract(const std::vector<AnnotationC> &in,
	             const AnnInOptions &opts,
	             std::vector<ExtractedSymbol> &out,
	             ExtractStats *stats,
	             QString *err) override
	{
		(void)err;
		out.clear();
		out.reserve(in.size());

		ExtractStats st;

		// Resolve whitelists once per call.
		std::vector<const QStringList *> whitelists;
		whitelists.reserve(m_profile.rules.size());
		for(const KlassRule &r : m_profile.rules) {
			const QStringList *list = &r.defaultKlasses;
			if(r.optsKey) {
				auto it = opts.klassOverrides.find(r.optsKey);
				if(it != opts.klassOverrides.end() && !it->second.isEmpty())
					list = &it->second;
			}
			whitelists.push_back(list);
		}

		for(std::size_t i = 0; i < in.size(); ++i) {
			const AnnotationC &a     = in[i];
			const QString      klass = QString::fromStdString(a.klass);
			++st.total;

			// First matching rule wins.
			int matched = -1;
			for(std::size_t r = 0; r < m_profile.rules.size(); ++r) {
				if(whitelists[r]->contains(klass)) {
					matched = static_cast<int>(r);
					break;
				}
			}
			if(matched < 0) {
				++st.ignored;
				continue;
			}
			++st.matched;

			const KlassRule &rule = m_profile.rules[matched];

			ExtractedSymbol s;
			s.upstreamAnnIndex = static_cast<int>(i);
			s.tag = rule.tagFromKlass ? tagFromKlassName(klass) : rule.outTag;
			if(s.tag == SymbolTag::Unknown) {
				// Whitelisted by an override we have no tag for.
				++st.ignored;
				continue;
			}

			if(symbolTagCarriesByte(s.tag)) {
				uint8_t byte = 0;
				if(a.hasValue) {
					// Producer handed us the number; no radix guessing.
					byte = static_cast<uint8_t>(a.value & 0xFFu);
				} else {
					const ByteParse r = parseAnnotationByteEx(
						QString::fromStdString(a.text),
						opts.stream.textRadix, byte);
					if(r != ByteParse::Ok) {
						// A payload class with no numeral is routine (sigrok
						// repeats the address class for the R/W bit); a numeral
						// that will not read in the declared radix is not.
						if(r == ByteParse::RadixMismatch)
							++st.radixMismatch;
						else
							++st.noPayload;
						continue;
					}
				}
				s.byte = byte;
			}

			out.push_back(s);
			++st.emitted;
		}

		if(stats) *stats = st;
		return true;
	}

private:
	ExtractorProfile m_profile;
};

// ---- Per-protocol profiles -------------------------------------------------

ExtractorProfile uartProfile()
{
	ExtractorProfile p;
	p.upstreamId = QStringLiteral("uart");
	p.rules = {
		KlassRule{
			"klass",
			QStringList{QStringLiteral("data"),
			            QStringLiteral("rx-data"),
			            QStringLiteral("tx-data")},
			SymbolTag::Data,
		},
	};
	return p;
}

ExtractorProfile spiProfile()
{
	ExtractorProfile p;
	p.upstreamId = QStringLiteral("spi");
	p.rules = {
		KlassRule{
			"mosiKlass",
			QStringList{QStringLiteral("mosi-data"), QStringLiteral("mosi")},
			SymbolTag::Mosi,
		},
		KlassRule{
			"misoKlass",
			QStringList{QStringLiteral("miso-data"), QStringLiteral("miso")},
			SymbolTag::Miso,
		},
		// Single-direction "data" class maps to MOSI.
		KlassRule{
			"dataKlass",
			QStringList{QStringLiteral("data")},
			SymbolTag::Mosi,
		},
		// CS-block delimiter. The spi PD emits one *-transfer annotation per
		// CS-asserted block, after that block's data words, so this marks the
		// end of a transfer rather than the start of one. Backends turn it into
		// a CS boundary — without it every word lands in a single transfer and
		// CS-aware upper layers (spiflash) misparse.
		KlassRule{
			"transferKlass",
			QStringList{QStringLiteral("mosi-transfer"),
			            QStringLiteral("miso-transfer")},
			SymbolTag::TransferEnd,
		},
	};
	return p;
}

ExtractorProfile i2cProfile()
{
	ExtractorProfile p;
	p.upstreamId = QStringLiteral("i2c");
	p.rules = {
		// Framing: tag comes from the class id, so "start" and "repeat-start"
		// stay distinguishable. Losing that distinction reads a read-address
		// as a data write and inverts direction for the rest of the transaction.
		KlassRule{
			"startKlass",
			QStringList{QStringLiteral("start"),
			            QStringLiteral("repeat-start")},
			SymbolTag::Start,
			/*tagFromKlass=*/true,
		},
		KlassRule{
			"stopKlass",
			QStringList{QStringLiteral("stop")},
			SymbolTag::Stop,
		},
		// Framing: the i2c PD reports ack/nack *after* the byte it belongs to.
		// Kept because a regenerating backend must state each byte's
		// acknowledge explicitly, and dropping these NACKs the whole capture.
		KlassRule{
			"ackKlass",
			QStringList{QStringLiteral("ack"), QStringLiteral("nack")},
			SymbolTag::Ack,
			/*tagFromKlass=*/true,
		},
		KlassRule{
			"addrRKlass",
			QStringList{QStringLiteral("address-read")},
			SymbolTag::AddrRead,
		},
		KlassRule{
			"addrWKlass",
			QStringList{QStringLiteral("address-write")},
			SymbolTag::AddrWrite,
		},
		KlassRule{
			"dataKlass",
			QStringList{QStringLiteral("data-read"),
			            QStringLiteral("data-write"),
			            QStringLiteral("data")},
			SymbolTag::Data,
		},
	};
	return p;
}

} // namespace

std::unique_ptr<IAnnotationExtractor> makeUartExtractor()
{
	return std::make_unique<TableDrivenExtractor>(uartProfile());
}

std::unique_ptr<IAnnotationExtractor> makeSpiExtractor()
{
	return std::make_unique<TableDrivenExtractor>(spiProfile());
}

std::unique_ptr<IAnnotationExtractor> makeI2cExtractor()
{
	return std::make_unique<TableDrivenExtractor>(i2cProfile());
}

} // namespace decoder
} // namespace scopy
