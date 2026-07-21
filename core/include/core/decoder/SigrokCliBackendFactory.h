#pragma once

#include "scopy-core_export.h"
#include "AnnotationCodecRegistry.h"
#include "IDecoderBackendFactory.h"
#include "SigrokCliBackend.h"

namespace scopy {
namespace decoder {

class SigrokCliCatalog;
class DecoderLogger;

// Factory for SigrokCliBackend. Threads a shared catalog and codec
// registry (built-in uart/spi/i2c) to every created backend. Additional
// codecs can be injected before calling create().
class SCOPY_CORE_EXPORT SigrokCliBackendFactory : public IDecoderBackendFactory
{
public:
	explicit SigrokCliBackendFactory(SigrokCliCatalog *catalog)
		: m_catalog(catalog)
	{
		m_codecs.registerBuiltinCodecs();
	}

	void setLogger(DecoderLogger *lg) { m_logger = lg; }

	AnnotationCodecRegistry       &codecRegistry()       { return m_codecs; }
	const AnnotationCodecRegistry &codecRegistry() const { return m_codecs; }

	std::unique_ptr<IDecoderBackend> create() override
	{
		auto b = std::make_unique<SigrokCliBackend>(m_catalog);
		b->setLogger(m_logger);
		b->setCodecRegistry(&m_codecs);
		return b;
	}

private:
	SigrokCliCatalog       *m_catalog{nullptr};
	DecoderLogger          *m_logger{nullptr};
	AnnotationCodecRegistry m_codecs;
};

} // namespace decoder
} // namespace scopy
