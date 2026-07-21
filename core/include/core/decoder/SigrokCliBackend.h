#pragma once

#include "scopy-core_export.h"
#include "IDecoderBackend.h"

#include <QString>
#include <QStringList>

namespace scopy {
namespace decoder {

class SigrokCliCatalog;
class DecoderLogger;
class AnnotationCodecRegistry;
struct ProtocolDataInput;

// Backend that drives sigrok-cli as a one-shot child process per decode().
// Stateless across calls; annotation sample indices are buffer-local.
// The catalog resolves the executable path and must outlive the backend.
// Safe to call decode() from any single thread; no event loop required.
class SCOPY_CORE_EXPORT SigrokCliBackend : public IDecoderBackend
{
public:
	explicit SigrokCliBackend(SigrokCliCatalog *catalog);
	~SigrokCliBackend() override;

	bool        decode(const DecoderConfig &cfg,
	                   const uint8_t *data, std::size_t nSamples,
	                   std::vector<AnnotationC> &out) override;

	bool        acceptsAnnotationInput(const DecoderConfig &cfg) const override;
	bool        decodeAnnotations(const DecoderConfig &cfg,
	                              const std::vector<AnnotationC> &in,
	                              std::vector<AnnotationC> &out) override;

	std::string lastError() const override { return m_lastError; }

	QString lastCommandLine() const { return m_lastCmdLine; }

	void setLogger(DecoderLogger *lg) { m_logger = lg; }
	void setCodecRegistry(const AnnotationCodecRegistry *r) { m_codecs = r; }

private:
	QStringList buildArgs(const DecoderConfig &cfg) const;
	// Overrides -I with the caller-supplied format (annotation-input path).
	// With pdIn: serializes the stack as one stacked -P (needed for sigrok
	// to wire OUTPUT_PYTHON between stages), binds root channels to the
	// virtual protocoldata names in pdIn->rootChannelOverrides and merges
	// downstreamOptionOverrides().
	QStringList buildArgsWithInput(const DecoderConfig &cfg,
	                               const QString &inputFormat,
	                               const ProtocolDataInput *pdIn = nullptr) const;
	// Per-PD hardcoded overrides for the annotation-input regen path
	// (e.g. modbus needs cschannel=RX because -I protocoldata only emits
	// the RX-equivalent wire). Empty map if none.
	static std::map<std::string, std::string>
	downstreamOptionOverrides(const std::string &decoderId);
	// Resolve stage index by matching each annotation's decoder id against
	// cfg.stack; sigrok's "-N" suffix is stripped and ignored.
	void        parseStdout(const QByteArray &buf,
	                        const std::vector<DecoderStage> &stack,
	                        std::vector<AnnotationC> &out) const;

	SigrokCliCatalog             *m_catalog{nullptr};
	DecoderLogger                *m_logger{nullptr};
	const AnnotationCodecRegistry *m_codecs{nullptr};
	std::string                   m_lastError;
	QString                       m_lastCmdLine;
};

} // namespace decoder
} // namespace scopy
