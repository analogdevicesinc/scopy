#pragma once

#include "scopy-core_export.h"
#include "IDecoderBackend.h"

#include <memory>

namespace scopy {
namespace decoder {

// Constructs a fresh IDecoderBackend per active decoder instance. Injected
// into DecoderManager so backends are swappable. Must be cheap on the main
// thread; heavy setup belongs in the created backend or its IDecoderCatalog.
class SCOPY_CORE_EXPORT IDecoderBackendFactory
{
public:
	virtual ~IDecoderBackendFactory() = default;

	// Never nullptr on success; on failure, return a backend whose
	// decode() fails with lastError() populated.
	virtual std::unique_ptr<IDecoderBackend> create() = 0;
};

} // namespace decoder
} // namespace scopy
