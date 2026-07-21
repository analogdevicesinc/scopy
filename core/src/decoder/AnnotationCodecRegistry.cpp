#include "decoder/AnnotationCodecRegistry.h"

#include "decoder/codecs/BuiltinCodecs.h"

namespace scopy {
namespace decoder {

void AnnotationCodecRegistry::registerExtractor(std::unique_ptr<IAnnotationExtractor> e)
{
	if(!e) return;
	const std::string key = e->upstreamId().toStdString();
	m_extractors[key] = std::move(e);
}

void AnnotationCodecRegistry::registerEncoder(std::unique_ptr<IProtocolDataEncoder> e)
{
	if(!e) return;
	const std::string key = e->downstreamId().toStdString();
	m_encoders[key] = std::move(e);
}

IAnnotationExtractor *AnnotationCodecRegistry::findExtractor(const QString &upstreamId) const
{
	auto it = m_extractors.find(upstreamId.toStdString());
	return (it == m_extractors.end()) ? nullptr : it->second.get();
}

IProtocolDataEncoder *AnnotationCodecRegistry::findEncoder(const QString &downstreamId) const
{
	auto it = m_encoders.find(downstreamId.toStdString());
	return (it == m_encoders.end()) ? nullptr : it->second.get();
}

bool AnnotationCodecRegistry::supports(const QString &upstreamId,
                                       const QString &downstreamId) const
{
	return findExtractor(upstreamId) != nullptr &&
	       findEncoder(downstreamId) != nullptr;
}

void AnnotationCodecRegistry::registerBuiltinCodecs()
{
	registerExtractor(codecs::makeUartExtractor());
	registerExtractor(codecs::makeSpiExtractor());
	registerExtractor(codecs::makeI2cExtractor());

	registerEncoder(codecs::makeUartEncoder());
	registerEncoder(codecs::makeSpiEncoder());
	registerEncoder(codecs::makeI2cEncoder());
}

} // namespace decoder
} // namespace scopy
