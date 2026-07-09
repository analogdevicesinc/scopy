#pragma once

#include "scopy-core_export.h"
#include "IDecoderBackend.h"

#include <QString>
#include <QStringList>

namespace scopy {
namespace decoder {

class SigrokCliCatalog;

// IDecoderBackend implementation that drives sigrok-cli as a one-shot
// child process. Each decode() call:
//   1. spawns sigrok-cli with the configured protocol decoder
//   2. writes the entire packed sample buffer to stdin
//   3. closes stdin (EOF flushes the decoder)
//   4. waits for the process to finish
//   5. parses stdout into annotations
//   6. tears the process down
//
// No state is preserved across decode() calls. Annotation sample indices
// are buffer-local (0 .. nSamples-1).
//
// The executable path is resolved by the catalog (non-owning), which
// owns the override / cache. The catalog must outlive the backend.
//
// Threading: safe to call decode() from any single thread; no QObject
// signals are emitted, so no event loop is required.
class SCOPY_CORE_EXPORT SigrokCliBackend : public IDecoderBackend
{
public:
	explicit SigrokCliBackend(SigrokCliCatalog *catalog);
	~SigrokCliBackend() override;

	bool        decode(const DecoderConfig &cfg,
	                   const uint8_t *data, std::size_t nSamples,
	                   std::vector<AnnotationC> &out) override;
	std::string lastError() const override { return m_lastError; }

	QString lastCommandLine() const { return m_lastCmdLine; }

private:
	QStringList buildArgs(const DecoderConfig &cfg) const;
	void        parseStdout(const QByteArray &buf,
	                        std::vector<AnnotationC> &out) const;

	SigrokCliCatalog *m_catalog{nullptr};
	std::string       m_lastError;
	QString           m_lastCmdLine;
};

} // namespace decoder
} // namespace scopy
