#include "decoder/DynamicLibBackend.h"

#include <QLibrary>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_DYNLIB_BACKEND, "DynamicLibBackend")

namespace scopy {
namespace decoder {

DynamicLibBackend::DynamicLibBackend(const QString &libraryPath)
	: m_libraryPath(libraryPath)
{}

DynamicLibBackend::~DynamicLibBackend() = default;

bool DynamicLibBackend::decode(const DecoderConfig & /*cfg*/,
                               const uint8_t * /*data*/, std::size_t /*nSamples*/,
                               std::vector<AnnotationC> & /*out*/)
{
	m_lastError = "DynamicLibBackend not implemented in this build";
	qCWarning(CAT_DYNLIB_BACKEND)
		<< "decode() called on stub backend; library:" << m_libraryPath;
	return false;
}

} // namespace decoder
} // namespace scopy
