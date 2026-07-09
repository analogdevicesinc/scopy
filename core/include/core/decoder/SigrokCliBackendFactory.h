#pragma once

#include "scopy-core_export.h"
#include "IDecoderBackendFactory.h"
#include "SigrokCliBackend.h"

namespace scopy {
namespace decoder {

class SigrokCliCatalog;

// IDecoderBackendFactory that hands out fresh SigrokCliBackend instances.
// The catalog (non-owning) is threaded through to every created backend so
// they all share the same executable resolution / cache / override.
class SCOPY_CORE_EXPORT SigrokCliBackendFactory : public IDecoderBackendFactory
{
public:
	explicit SigrokCliBackendFactory(SigrokCliCatalog *catalog)
		: m_catalog(catalog)
	{}

	std::unique_ptr<IDecoderBackend> create() override
	{
		return std::make_unique<SigrokCliBackend>(m_catalog);
	}

private:
	SigrokCliCatalog *m_catalog{nullptr};
};

} // namespace decoder
} // namespace scopy
