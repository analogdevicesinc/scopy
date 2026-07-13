#pragma once

#include "scopy-core_export.h"
#include "IDecoderBackendFactory.h"
#include "SigrokCliBackend.h"

namespace scopy {
namespace decoder {

class SigrokCliCatalog;
class DecoderLogger;

// IDecoderBackendFactory that hands out fresh SigrokCliBackend instances.
// The catalog (non-owning) is threaded through to every created backend so
// they all share the same executable resolution / cache / override.
class SCOPY_CORE_EXPORT SigrokCliBackendFactory : public IDecoderBackendFactory
{
public:
	explicit SigrokCliBackendFactory(SigrokCliCatalog *catalog)
		: m_catalog(catalog)
	{}

	void setLogger(DecoderLogger *lg) { m_logger = lg; }

	std::unique_ptr<IDecoderBackend> create() override
	{
		auto b = std::make_unique<SigrokCliBackend>(m_catalog);
		b->setLogger(m_logger);
		return b;
	}

private:
	SigrokCliCatalog *m_catalog{nullptr};
	DecoderLogger    *m_logger{nullptr};
};

} // namespace decoder
} // namespace scopy
