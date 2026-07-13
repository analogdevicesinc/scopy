#include "decoder/DynamicLibBackend.h"

#include "decoder/DecoderLogger.h"

#include <QLibrary>

namespace scopy {
namespace decoder {

static constexpr const char *kDynLibId = "dynamic-lib-backend";

DynamicLibBackend::DynamicLibBackend(const QString &libraryPath)
	: m_libraryPath(libraryPath)
{}

DynamicLibBackend::~DynamicLibBackend() = default;

bool DynamicLibBackend::decode(const DecoderConfig & /*cfg*/,
                               const uint8_t * /*data*/, std::size_t /*nSamples*/,
                               std::vector<AnnotationC> & /*out*/)
{
	m_lastError = "DynamicLibBackend not implemented in this build";
	if(m_logger)
		m_logger->warning(kDynLibId,
			QStringLiteral("decode() called on stub backend; library: ") + m_libraryPath);
	return false;
}

} // namespace decoder
} // namespace scopy
