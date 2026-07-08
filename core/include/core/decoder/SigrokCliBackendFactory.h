#pragma once

#include "scopy-core_export.h"
#include "IDecoderBackendFactory.h"
#include "SigrokCliBackend.h"

#include <QString>

namespace scopy {
namespace decoder {

// IDecoderBackendFactory that hands out fresh SigrokCliBackend instances.
// The optional executable override is forwarded to every created backend so
// tests / demo wiring can pin a specific sigrok-cli path.
class SCOPY_CORE_EXPORT SigrokCliBackendFactory : public IDecoderBackendFactory
{
public:
	SigrokCliBackendFactory() = default;

	void setExecutableOverride(const QString &path) { m_exeOverride = path; }

	std::unique_ptr<IDecoderBackend> create() override
	{
		auto b = std::make_unique<SigrokCliBackend>();
		if(!m_exeOverride.isEmpty())
			b->setExecutableOverride(m_exeOverride);
		return b;
	}

private:
	QString m_exeOverride;
};

} // namespace decoder
} // namespace scopy
