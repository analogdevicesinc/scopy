#pragma once

#include "iioutil/iattrops.h"

namespace scopy::iio {

// In v1, AttrHandle.ptr stores const iio_attr* directly — no wrapper struct needed.
class V1AttrOps : public IAttrOps
{
public:
	AttrHandle contextAttr(ContextHandle ctx, const QString &name) override;
    AttrHandle deviceAttr(DeviceHandle dev, const QString &name) override;
	AttrHandle debugAttr(DeviceHandle dev, const QString &name) override;
	AttrHandle bufferAttr(DeviceHandle dev, unsigned int bufferIdx, const QString &name) override;
	AttrHandle channelAttr(ChannelHandle ch, const QString &name) override;
	void releaseAttr(AttrHandle attr) override; // no-op: iio_attr* lifetime is tied to context

	Result<QByteArray> read(AttrHandle attr) override;
	Result<void> write(AttrHandle attr, const QString &value) override;

	Result<void> getRange(AttrHandle attr, double &min, double &step, double &max) const override;
	Result<void> getAvailable(AttrHandle attr, QStringList &values) const override;
};

} // namespace scopy::iio
