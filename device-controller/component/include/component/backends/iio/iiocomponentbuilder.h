#pragma once

#include "component/componentbuilder.h"
#include "iioutil/handles.h"

#include <QString>

namespace scopy::iio {
class IBackend;
class IAttrOps;
}

namespace scopy::component {
class Attribute;
}

namespace scopy::component::iio {

class IIOContext;
class IIODevice;

// Builds the IIO component tree from a Context whose IIOContext handle is already
// created and whose executor is set. Written as many small single-purpose methods.
class IIOComponentBuilder : public ComponentBuilder
{
public:
	explicit IIOComponentBuilder(scopy::iio::IBackend *backend)
		: m_backend(backend)
	{
	}

	bool build(Context *ctx, ICmdExecutor *executor) override;

private:
	void setContextIdentity(IIOContext *iioCtx);
	void buildContextAttributes(IIOContext *iioCtx, ICmdExecutor *executor);
	void buildDevices(IIOContext *iioCtx, ICmdExecutor *executor);
	void buildOneDevice(IIOContext *iioCtx, scopy::iio::DeviceHandle dh, ICmdExecutor *executor);
	void buildDeviceAttributes(IIODevice *dev, ICmdExecutor *executor);
	void buildDebugAttributes(IIODevice *dev, ICmdExecutor *executor);
	void buildChannels(IIODevice *dev, ICmdExecutor *executor);

	scopy::component::Attribute *makeAttribute(QObject *parent, const QString &attrName,
						   scopy::iio::AttrHandle handle, ICmdExecutor *executor,
						   bool forceReadOnly = false);

	static bool isReadOnlyAttr(const QString &attrName);

	scopy::iio::IBackend *m_backend;
	scopy::iio::IAttrOps *m_attrOps = nullptr;
};

} // namespace scopy::component::iio
