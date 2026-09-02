#pragma once

#include "scopy-core_export.h"
#include "IDecoderBackend.h"
// AnnInOptions and IAnnotationExtractor appear by value/reference in private
// member signatures, so a forward declaration will not do.
#include "AnnotationSymbols.h"

#include <QString>
#include <QStringList>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace scopy {
namespace decoder {

class SigrokCliCatalog;
class DecoderLogger;

namespace sigrok {
struct ProtocolDataInput;
class ProtocolDataEncoderRegistry;
} // namespace sigrok

// Backend that drives sigrok-cli as a one-shot child process per decode().
// Stateless across calls; annotation sample indices are buffer-local.
// The catalog resolves the executable path and must outlive the backend.
// Safe to call decode() from any single thread; no event loop required.
class SCOPY_CORE_EXPORT SigrokCliBackend : public IDecoderBackend
{
public:
	explicit SigrokCliBackend(SigrokCliCatalog *catalog);
	~SigrokCliBackend() override;

	bool decode(const DecoderConfig &cfg,
	            const uint8_t *data, std::size_t nSamples,
	            std::vector<AnnotationC> &out) override;

	bool acceptsAnnotationInput(const DecoderConfig &cfg) const override;
	bool decodeAnnotations(const DecoderConfig &cfg,
	                       const std::vector<AnnotationC> &in,
	                       std::vector<AnnotationC> &out) override;

	std::string lastError() const override { return m_lastError; }

	QString lastCommandLine() const { return m_lastCmdLine; }

	void setLogger(DecoderLogger *lg) { m_logger = lg; }

	// Extractors are shared and backend-neutral, so they are injected. The
	// matching protocoldata encoders are a private implementation detail of
	// this backend and are built in the constructor.
	void setExtractorRegistry(const AnnotationExtractorRegistry *r) { m_extractors = r; }

private:
	QStringList buildArgs(const DecoderConfig &cfg) const;
	// Overrides -I with the caller-supplied format (annotation-input path).
	// With pdIn: serializes the stack as one stacked -P (needed for sigrok
	// to wire OUTPUT_PYTHON between stages), binds root channels to the
	// virtual protocoldata names in pdIn->rootChannelOverrides and merges
	// sigrok::downstreamOptionOverrides().
	QStringList buildArgsWithInput(const DecoderConfig &cfg,
	                               const QString &inputFormat,
	                               const sigrok::ProtocolDataInput *pdIn = nullptr) const;
	// Resolves each annotation's stage index from sigrok's "<id>-<n>" instance
	// suffix, so a stack containing the same decoder twice does not collapse
	// both instances onto the first stage.
	void parseStdout(const QByteArray &buf,
	                 const std::vector<DecoderStage> &stack,
	                 std::vector<AnnotationC> &out) const;
	// Moves annotations decoded off the regenerated waveform back onto the
	// upstream timeline. `rootAnns` is the synthetic root stage's own output:
	// re-extracting it with `ext` says where each symbol actually landed on the
	// fabricated wire, which is what pdIn's anchors are matched against.
	// Consumes syntheticOut; appends to out.
	void remapToUpstream(const sigrok::ProtocolDataInput &pdIn,
	                     IAnnotationExtractor *ext,
	                     const AnnInOptions &opts,
	                     const std::vector<AnnotationC> &rootAnns,
	                     const std::vector<AnnotationC> &in,
	                     std::vector<AnnotationC> &syntheticOut,
	                     std::vector<AnnotationC> &out) const;

	const char                        *m_kBackendId;
	SigrokCliCatalog                  *m_catalog{nullptr};
	DecoderLogger                     *m_logger{nullptr};
	const AnnotationExtractorRegistry *m_extractors{nullptr};
	// unique_ptr so the private encoder types stay out of this header.
	std::unique_ptr<sigrok::ProtocolDataEncoderRegistry> m_encoders;
	std::string                        m_lastError;
	QString                            m_lastCmdLine;
};

} // namespace decoder
} // namespace scopy
