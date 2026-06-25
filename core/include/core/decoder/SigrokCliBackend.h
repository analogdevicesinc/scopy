#pragma once

#include "scopy-core_export.h"
#include "IDecoderBackend.h"

#include <QString>
#include <QStringList>

namespace scopy {
namespace decoder {

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
// Threading: safe to call decode() from any single thread; no QObject
// signals are emitted, so no event loop is required.
class SCOPY_CORE_EXPORT SigrokCliBackend : public IDecoderBackend
{
public:
	SigrokCliBackend();
	~SigrokCliBackend() override;

	bool        decode(const DecoderConfig &cfg,
	                   const uint8_t *data, std::size_t nSamples,
	                   std::vector<AnnotationC> &out) override;
	std::string lastError() const override { return m_lastError; }

	// Allows demo wiring / tests to override the CLI lookup.
	void    setExecutableOverride(const QString &path);
	QString lastCommandLine() const { return m_lastCmdLine; }

private:
	QString     findCli() const;
	QString     resolveCli() const; // cached wrapper around findCli()
	QStringList buildArgs(const DecoderConfig &cfg) const;
	void        parseStdout(const QByteArray &buf,
	                        std::vector<AnnotationC> &out) const;

	std::string     m_lastError;
	QString         m_exeOverride;
	QString         m_lastCmdLine;
	mutable QString m_cachedExe;
};

} // namespace decoder
} // namespace scopy
