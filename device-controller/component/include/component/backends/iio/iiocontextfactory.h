#pragma once

#include "component/contextfactory.h"
#include "iioutil/handles.h"

namespace scopy::component::iio {

// Builds a libiio-backed Context: loads the backend plugin for its fixed
// LibiioVersion, opens the context handle, attaches a serial executor, and
// runs the IIO component builder. One factory instance per version.
class IIOContextFactory : public ContextFactory
{
public:
	explicit IIOContextFactory(scopy::iio::LibiioVersion version)
		: m_version(version)
	{
	}

	Context *create(const QString &uri) override;

private:
	scopy::iio::LibiioVersion m_version;
};

} // namespace scopy::component::iio
