#pragma once

#include "iioutil/iattrops.h"

namespace scopy::iio {

// v0 attribute info — wraps the scope-specific details needed for read/write
struct V0AttrInfo
{
    enum Scope
    {
        Context,
        Device,
        Channel,
        Debug,
        Buffer
    };
	Scope scope;
	void *parent; // iio_context*, iio_device*, or iio_channel*
    QByteArray name;
};

class V0AttrOps : public IAttrOps
{
public:
	AttrHandle contextAttr(ContextHandle ctx, const QString &name) override;
	AttrHandle deviceAttr(DeviceHandle dev, const QString &name) override;
	AttrHandle debugAttr(DeviceHandle dev, const QString &name) override;
	AttrHandle bufferAttr(DeviceHandle dev, unsigned int bufferIdx, const QString &name) override;
	AttrHandle channelAttr(ChannelHandle ch, const QString &name) override;
	void releaseAttr(AttrHandle attr) override;

	Result<QByteArray> read(AttrHandle attr) override;
	Result<void> write(AttrHandle attr, const QString &value) override;

	Result<void> getRange(AttrHandle attr, double &min, double &step, double &max) const override;
	Result<void> getAvailable(AttrHandle attr, QStringList &values) const override;

private:
	AttrHandle makeHandle(V0AttrInfo::Scope scope, void *parent, const QString &name);
	Result<QByteArray> readAvailableAttr(const V0AttrInfo *info) const;
};

} // namespace scopy::iio
