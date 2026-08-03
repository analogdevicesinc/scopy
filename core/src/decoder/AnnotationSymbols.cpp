// Backend-neutral half of the annotation-input feature: the symbol
// vocabulary, the meta <-> AnnotationStreamInfo round-trip, and the extractor
// registry. Nothing here knows about sigrok.

#include "decoder/AnnotationSymbols.h"

#include <QStringList>

namespace scopy {
namespace decoder {

namespace {

const std::string kPrefix       = "annIn.";
const std::string kStreamPrefix = "annIn.stream.";

// Value for `key` (already including the "annIn." prefix), or nullopt.
std::optional<std::string> metaGet(const std::map<std::string, std::string> &meta,
                                   const std::string &key)
{
	auto it = meta.find(key);
	if(it == meta.end() || it->second.empty())
		return std::nullopt;
	return it->second;
}

} // namespace

const char *symbolTagName(SymbolTag tag)
{
	switch(tag) {
	case SymbolTag::Unknown:     return "unknown";
	case SymbolTag::Data:        return "data";
	case SymbolTag::Mosi:        return "mosi";
	case SymbolTag::Miso:        return "miso";
	case SymbolTag::AddrRead:    return "addr-read";
	case SymbolTag::AddrWrite:   return "addr-write";
	case SymbolTag::Start:       return "start";
	case SymbolTag::RepeatStart: return "repeat-start";
	case SymbolTag::Stop:        return "stop";
	case SymbolTag::Ack:         return "ack";
	case SymbolTag::Nack:        return "nack";
	case SymbolTag::TransferEnd: return "transfer-end";
	}
	return "?";
}

bool symbolTagCarriesByte(SymbolTag tag)
{
	switch(tag) {
	case SymbolTag::Data:
	case SymbolTag::Mosi:
	case SymbolTag::Miso:
	case SymbolTag::AddrRead:
	case SymbolTag::AddrWrite:
		return true;
	default:
		return false;
	}
}

// --- AnnotationStreamInfo <-> meta -----------------------------------------

void streamInfoToMeta(const AnnotationStreamInfo &info,
                      std::map<std::string, std::string> &meta)
{
	meta[kStreamPrefix + "producerId"] = info.producerId.toStdString();
	meta[kStreamPrefix + "textRadix"]  = textRadixName(info.textRadix);
	meta[kStreamPrefix + "samplerate"] =
		QString::number(info.sampleRate, 'f', 0).toStdString();
	meta[kStreamPrefix + "bitrate"] =
		QString::number(info.bitrate).toStdString();
}

AnnotationStreamInfo streamInfoFromMeta(const std::map<std::string, std::string> &meta)
{
	AnnotationStreamInfo info;
	if(auto v = metaGet(meta, kStreamPrefix + "producerId"))
		info.producerId = QString::fromStdString(*v);
	if(auto v = metaGet(meta, kStreamPrefix + "textRadix"))
		(void)acq::textRadixFromString(QString::fromStdString(*v), info.textRadix);
	if(auto v = metaGet(meta, kStreamPrefix + "samplerate"))
		info.sampleRate = QString::fromStdString(*v).toDouble();
	if(auto v = metaGet(meta, kStreamPrefix + "bitrate"))
		info.bitrate = QString::fromStdString(*v).toULongLong();
	return info;
}

// --- AnnInOptions ----------------------------------------------------------

QStringList AnnInOptions::knownScalarKeys()
{
	return {
		QStringLiteral("upstreamId"),
		QStringLiteral("samplerate"),
		QStringLiteral("bitrate"),
		QStringLiteral("frameformat"),
		QStringLiteral("direction"),
		QStringLiteral("radix"),
		QStringLiteral("stream.producerId"),
		QStringLiteral("stream.textRadix"),
		QStringLiteral("stream.samplerate"),
		QStringLiteral("stream.bitrate"),
	};
}

AnnInOptions AnnInOptions::fromMeta(const std::map<std::string, std::string> &meta,
                                    QStringList *warnings)
{
	AnnInOptions o;
	o.stream = streamInfoFromMeta(meta);

	auto warn = [&](const QString &msg) {
		if(warnings) warnings->append(msg);
	};

	const QStringList known = knownScalarKeys();

	for(const auto &kv : meta) {
		if(kv.first.compare(0, kPrefix.size(), kPrefix) != 0)
			continue;
		const std::string bare = kv.first.substr(kPrefix.size());
		const QString     qkey = QString::fromStdString(bare);
		const QString     qval = QString::fromStdString(kv.second);

		// "*Klass" keys are extractor-defined, so accept any of them.
		if(qkey.endsWith(QStringLiteral("Klass"))) {
			if(!qval.isEmpty())
				o.klassOverrides[bare] =
					qval.split(QLatin1Char(','), Qt::SkipEmptyParts);
			continue;
		}

		if(qkey == QStringLiteral("upstreamId")) {
			o.upstreamId = qval;
		} else if(qkey == QStringLiteral("samplerate")) {
			bool ok = false;
			const double d = qval.toDouble(&ok);
			if(ok && d > 0.0) o.synthSampleRate = d;
			else warn(QStringLiteral("annIn.samplerate: not a positive number ('%1'), "
			                         "using backend default").arg(qval));
		} else if(qkey == QStringLiteral("bitrate")) {
			bool ok = false;
			const quint64 b = qval.toULongLong(&ok);
			if(ok && b > 0) o.synthBitrate = b;
			else warn(QStringLiteral("annIn.bitrate: not a positive integer ('%1'), "
			                         "using backend default").arg(qval));
		} else if(qkey == QStringLiteral("frameformat")) {
			o.frameFormat = qval;
		} else if(qkey == QStringLiteral("direction")) {
			const QString d = qval.toLower();
			if(d == QStringLiteral("mosi") || d == QStringLiteral("miso") ||
			   d == QStringLiteral("both"))
				o.direction = d;
			else
				warn(QStringLiteral("annIn.direction: expected mosi|miso|both, "
				                    "got '%1'").arg(qval));
		} else if(qkey == QStringLiteral("radix")) {
			// Consumer override of the producer's declaration. Present so a
			// misreporting producer can still be worked around by hand.
			TextRadix r{};
			if(acq::textRadixFromString(qval, r))
				o.stream.textRadix = r;
			else
				warn(QStringLiteral("annIn.radix: unknown radix '%1', keeping %2")
					.arg(qval, QLatin1String(textRadixName(o.stream.textRadix))));
		} else if(!known.contains(qkey)) {
			o.unknownKeys.append(qkey);
		}
	}

	if(o.upstreamId.isEmpty())
		o.upstreamId = o.stream.producerId;

	return o;
}

QString ExtractStats::toString() const
{
	return QStringLiteral("total=%1 matched=%2 emitted=%3 ignored=%4 "
	                      "noPayload=%5 radixMismatch=%6")
		.arg(total).arg(matched).arg(emitted).arg(ignored)
		.arg(noPayload).arg(radixMismatch);
}

// --- AnnotationExtractorRegistry -------------------------------------------

void AnnotationExtractorRegistry::registerExtractor(std::unique_ptr<IAnnotationExtractor> e)
{
	if(!e) return;
	m_extractors[e->upstreamId().toStdString()] = std::move(e);
}

IAnnotationExtractor *AnnotationExtractorRegistry::find(const QString &upstreamId) const
{
	auto it = m_extractors.find(upstreamId.toStdString());
	return (it == m_extractors.end()) ? nullptr : it->second.get();
}

QStringList AnnotationExtractorRegistry::upstreamIds() const
{
	QStringList ids;
	ids.reserve(static_cast<int>(m_extractors.size()));
	for(const auto &kv : m_extractors)
		ids.append(QString::fromStdString(kv.first));
	ids.sort();
	return ids;
}

void AnnotationExtractorRegistry::registerBuiltins()
{
	registerExtractor(makeUartExtractor());
	registerExtractor(makeSpiExtractor());
	registerExtractor(makeI2cExtractor());
}

} // namespace decoder
} // namespace scopy
