#pragma once

#include "scopy-core_export.h"
#include "AnnotationSymbols.h"
#include "IDecoderBackendFactory.h"
#include "SigrokCliBackend.h"

namespace scopy {
namespace decoder {

class SigrokCliCatalog;
class DecoderLogger;

// Factory for SigrokCliBackend. Threads a shared catalog and annotation
// extractor registry (built-in uart/spi/i2c) to every created backend.
// Additional extractors can be registered before calling create().
class SCOPY_CORE_EXPORT SigrokCliBackendFactory : public IDecoderBackendFactory
{
public:
	explicit SigrokCliBackendFactory(SigrokCliCatalog *catalog)
		: m_catalog(catalog)
	{
		m_extractors.registerBuiltins();
	}

	void setLogger(DecoderLogger *lg) { m_logger = lg; }

	AnnotationExtractorRegistry       &extractorRegistry()       { return m_extractors; }
	const AnnotationExtractorRegistry &extractorRegistry() const { return m_extractors; }

	std::unique_ptr<IDecoderBackend> create() override
	{
		auto b = std::make_unique<SigrokCliBackend>(m_catalog);
		b->setLogger(m_logger);
		b->setExtractorRegistry(&m_extractors);
		return b;
	}

private:
	SigrokCliCatalog           *m_catalog{nullptr};
	DecoderLogger              *m_logger{nullptr};
	AnnotationExtractorRegistry m_extractors;
};

} // namespace decoder
} // namespace scopy
