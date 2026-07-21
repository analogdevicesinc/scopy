#pragma once

#include "scopy-core_export.h"
#include "IAnnotationExtractor.h"
#include "IProtocolDataEncoder.h"

#include <QString>

#include <memory>
#include <string>
#include <unordered_map>

namespace scopy {
namespace decoder {

// Registry mapping upstream decoder id -> extractor and downstream root
// decoder id -> encoder. Owned by the backend factory; passed non-owning
// to backends. Add a protocol by registering its extractor/encoder here.
class SCOPY_CORE_EXPORT AnnotationCodecRegistry
{
public:
	AnnotationCodecRegistry() = default;
	~AnnotationCodecRegistry() = default;

	AnnotationCodecRegistry(const AnnotationCodecRegistry &) = delete;
	AnnotationCodecRegistry &operator=(const AnnotationCodecRegistry &) = delete;

	void registerExtractor(std::unique_ptr<IAnnotationExtractor> e);
	void registerEncoder(std::unique_ptr<IProtocolDataEncoder> e);

	IAnnotationExtractor *findExtractor(const QString &upstreamId) const;
	IProtocolDataEncoder *findEncoder(const QString &downstreamId) const;

	bool supports(const QString &upstreamId, const QString &downstreamId) const;

	// Register uart, spi and i2c codecs (extractor + encoder).
	void registerBuiltinCodecs();

private:
	// std::unordered_map — QHash requires copyable values; unique_ptr is move-only.
	std::unordered_map<std::string, std::unique_ptr<IAnnotationExtractor>> m_extractors;
	std::unordered_map<std::string, std::unique_ptr<IProtocolDataEncoder>> m_encoders;
};

} // namespace decoder
} // namespace scopy
