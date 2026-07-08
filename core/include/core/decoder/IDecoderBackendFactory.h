#pragma once

#include "scopy-core_export.h"
#include "IDecoderBackend.h"

#include <memory>

namespace scopy {
namespace decoder {

// Constructs a fresh IDecoderBackend per active decoder instance. This is
// what DecoderManager holds instead of a concrete backend type — swapping
// backends is a matter of injecting a different factory at composition time.
//
// The manager takes ownership of the returned backend (it hands it off to
// ExternalDecoderProcessor via std::unique_ptr). Implementations must be
// cheap to invoke on the main thread; heavy per-backend setup (executable
// resolution, catalog scan) should live inside the created backend or in an
// associated IDecoderCatalog.
class SCOPY_CORE_EXPORT IDecoderBackendFactory
{
public:
	virtual ~IDecoderBackendFactory() = default;

	// Create a new backend instance. Never returns nullptr on success;
	// implementations that cannot instantiate must throw or return a
	// backend whose decode() always fails with lastError() populated.
	virtual std::unique_ptr<IDecoderBackend> create() = 0;
};

} // namespace decoder
} // namespace scopy
