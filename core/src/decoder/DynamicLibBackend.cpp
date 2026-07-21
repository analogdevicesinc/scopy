#include "decoder/DynamicLibBackend.h"

#include "decoder/DecoderLogger.h"
#include "decoder/decoder_c_api.h"

#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLibrary>

#include <vector>

namespace scopy {
namespace decoder {

static constexpr const char *kDynLibId = "dynamic-lib-backend";

namespace {

// Serializes cfg per the JSON schema in decoder_c_api.h.
QByteArray serializeConfig(const DecoderConfig &cfg)
{
	QJsonObject root;
	if(!cfg.stack.empty())
		root["decoderId"] = QString::fromStdString(cfg.stack[0].decoderId);
	root["sampleRate"]  = cfg.sampleRate;
	root["numChannels"] = cfg.numChannels;
	root["rootInput"]   = (cfg.rootInput == RootInput::Annotations)
		? QStringLiteral("annotations")
		: QStringLiteral("samples");

	QJsonArray stack;
	for(const DecoderStage &s : cfg.stack) {
		QJsonObject j;
		j["decoderId"] = QString::fromStdString(s.decoderId);
		QJsonObject opts;
		for(const auto &kv : s.options)
			opts[QString::fromStdString(kv.first)] =
				QString::fromStdString(kv.second);
		j["options"] = opts;
		QJsonArray channels;
		for(const ChannelMap &c : s.channels) {
			QJsonObject cj;
			cj["role"]     = QString::fromStdString(c.role);
			cj["bitIndex"] = c.bitIndex;
			channels.append(cj);
		}
		j["channels"] = channels;
		stack.append(j);
	}
	root["stack"] = stack;

	QJsonObject meta;
	for(const auto &kv : cfg.meta)
		meta[QString::fromStdString(kv.first)] =
			QString::fromStdString(kv.second);
	root["meta"] = meta;

	if(cfg.rootInput == RootInput::Annotations) {
		QJsonObject annIn;
		annIn["sourceUid"]  = QString::fromStdString(cfg.annotationInput.sourceUid);
		annIn["stageIndex"] = cfg.annotationInput.sourceStageIndex;
		root["annotationInput"] = annIn;
	}

	return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

// scopy_decoder_ann_cb trampoline; 'user' is a std::vector<AnnotationC>*.
void annotationTrampoline(const scopy_decoder_annotation_t *ann, void *user)
{
	auto *out = static_cast<std::vector<AnnotationC> *>(user);
	if(!ann || !out) return;
	AnnotationC a;
	a.start      = ann->start;
	a.end        = ann->end;
	a.decoder    = ann->decoder ? std::string(ann->decoder) : std::string();
	a.klass      = ann->klass   ? std::string(ann->klass)   : std::string();
	a.text       = ann->text    ? std::string(ann->text)    : std::string();
	a.severity   = ann->severity;
	a.stageIndex = 0;
	out->push_back(std::move(a));
}

} // namespace

DynamicLibBackend::DynamicLibBackend(const QString &libraryPath)
	: m_libraryPath(libraryPath)
{}

DynamicLibBackend::~DynamicLibBackend() = default;

bool DynamicLibBackend::ensureLoaded() const
{
	if(m_loadAttempted) return m_symDecode != nullptr;
	m_loadAttempted = true;

	if(m_libraryPath.isEmpty()) {
		m_lastError = "DynamicLibBackend: empty library path";
		return false;
	}

	m_lib = std::make_unique<QLibrary>(m_libraryPath);
	if(!m_lib->load()) {
		m_lastError = "DynamicLibBackend: failed to load "
		              + m_libraryPath.toStdString() + ": "
		              + m_lib->errorString().toStdString();
		m_lib.reset();
		return false;
	}

	m_symDecode      = reinterpret_cast<void *>(m_lib->resolve("scopy_decoder_decode"));
	m_symAcceptsAnn  = reinterpret_cast<void *>(m_lib->resolve("scopy_decoder_accepts_annotations"));
	m_symDecodeAnn   = reinterpret_cast<void *>(m_lib->resolve("scopy_decoder_decode_ann"));

	if(!m_symDecode) {
		m_lastError = "DynamicLibBackend: mandatory symbol "
		              "scopy_decoder_decode not exported by "
		              + m_libraryPath.toStdString();
		m_lib->unload();
		m_lib.reset();
		return false;
	}
	return true;
}

bool DynamicLibBackend::decode(const DecoderConfig &cfg,
                               const uint8_t *data, std::size_t nSamples,
                               std::vector<AnnotationC> &out)
{
	out.clear();
	if(!ensureLoaded()) {
		if(m_logger) m_logger->warning(kDynLibId,
			QString::fromStdString(m_lastError));
		return false;
	}

	using CDecodeFn = int (*)(const char *, const uint8_t *, size_t,
	                          scopy_decoder_ann_cb, void *,
	                          char *, size_t);
	auto fn = reinterpret_cast<CDecodeFn>(m_symDecode);

	const QByteArray json = serializeConfig(cfg);
	char errbuf[512] = {0};
	const int rc = fn(json.constData(), data, nSamples,
	                  &annotationTrampoline, &out,
	                  errbuf, sizeof(errbuf));
	if(rc != 0) {
		m_lastError = errbuf[0] ? std::string(errbuf)
		                        : std::string("decode() returned non-zero");
		if(m_logger) m_logger->warning(kDynLibId,
			QString::fromStdString(m_lastError));
		return false;
	}
	return true;
}

bool DynamicLibBackend::acceptsAnnotationInput(const DecoderConfig & /*cfg*/) const
{
	if(!ensureLoaded()) return false;
	if(!m_symAcceptsAnn || !m_symDecodeAnn) return false;

	using CAcceptsFn = int (*)(void);
	auto fn = reinterpret_cast<CAcceptsFn>(m_symAcceptsAnn);
	return fn() != 0;
}

bool DynamicLibBackend::decodeAnnotations(const DecoderConfig &cfg,
                                          const std::vector<AnnotationC> &in,
                                          std::vector<AnnotationC> &out)
{
	out.clear();
	if(!ensureLoaded()) return false;
	if(!m_symDecodeAnn) {
		m_lastError = "DynamicLibBackend: library does not export "
		              "scopy_decoder_decode_ann";
		if(m_logger) m_logger->warning(kDynLibId,
			QString::fromStdString(m_lastError));
		return false;
	}

	// Marshal AnnotationC -> POD; source strings in 'in' back the c_str() ptrs.
	std::vector<scopy_decoder_annotation_t> pod;
	pod.reserve(in.size());
	for(const AnnotationC &a : in) {
		scopy_decoder_annotation_t p;
		p.start    = a.start;
		p.end      = a.end;
		p.decoder  = a.decoder.c_str();
		p.klass    = a.klass.c_str();
		p.text     = a.text.c_str();
		p.severity = a.severity;
		pod.push_back(p);
	}

	using CDecodeAnnFn = int (*)(const char *,
	                             const scopy_decoder_annotation_t *, size_t,
	                             scopy_decoder_ann_cb, void *,
	                             char *, size_t);
	auto fn = reinterpret_cast<CDecodeAnnFn>(m_symDecodeAnn);

	const QByteArray json = serializeConfig(cfg);
	char errbuf[512] = {0};
	const int rc = fn(json.constData(), pod.data(), pod.size(),
	                  &annotationTrampoline, &out,
	                  errbuf, sizeof(errbuf));
	if(rc != 0) {
		m_lastError = errbuf[0] ? std::string(errbuf)
		                        : std::string("decode_ann() returned non-zero");
		if(m_logger) m_logger->warning(kDynLibId,
			QString::fromStdString(m_lastError));
		return false;
	}
	return true;
}

} // namespace decoder
} // namespace scopy
