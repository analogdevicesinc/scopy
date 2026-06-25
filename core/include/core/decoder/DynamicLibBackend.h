#pragma once

#include "scopy-core_export.h"
#include "IDecoderBackend.h"

#include <QString>
#include <memory>

class QLibrary;

namespace scopy {
namespace decoder {

// IDecoderBackend implementation that loads a closed-source vendor decoder
// shared library exposing the C ABI defined in decoder_c_api.h.
//
// NOTE: This is a stub for this PR. The C ABI is fixed up front so the
// surface contract is settled; an actual QLibrary-based implementation will
// land in a follow-up alongside a real vendor library.
class SCOPY_CORE_EXPORT DynamicLibBackend : public IDecoderBackend
{
public:
	explicit DynamicLibBackend(const QString &libraryPath);
	~DynamicLibBackend() override;

	bool        decode(const DecoderConfig &cfg,
	                   const uint8_t *data, std::size_t nSamples,
	                   std::vector<AnnotationC> &out) override;
	std::string lastError() const override { return m_lastError; }

private:
	QString                   m_libraryPath;
	std::unique_ptr<QLibrary> m_lib;
	std::string               m_lastError;
};

} // namespace decoder
} // namespace scopy
