#pragma once

#include "iioutil/handles.h"
#include "core/result.h"
#include <QStringList>

namespace scopy::iio {

class IAttrOps {
public:
	virtual ~IAttrOps() = default;

	// Handle lifecycle — called during buildTree()
	virtual AttrHandle contextAttr(ContextHandle ctx, const QString &name) = 0;
	virtual AttrHandle deviceAttr(DeviceHandle dev, const QString &name) = 0;
	virtual AttrHandle debugAttr(DeviceHandle dev, const QString &name) = 0;
	virtual AttrHandle bufferAttr(DeviceHandle dev, unsigned int bufferIdx, const QString &name) = 0;
	virtual AttrHandle channelAttr(ChannelHandle ch, const QString &name) = 0;
	virtual void releaseAttr(AttrHandle attr) = 0;

	// I/O — returns a typed Result; called from Command::execute() on worker thread.
	// The read payload is the raw attr bytes; errors travel in the Unexpected<Error> channel.
	virtual Result<QByteArray> read(AttrHandle attr) = 0;
	virtual Result<void> write(AttrHandle attr, const QString &value) = 0;

	// Range / available — convenience parsers for _available attributes.
	// v1: thin wrappers around iio_attr_get_range / iio_attr_get_available.
	// v0: backend reads the attr string and parses it manually.
	virtual Result<void> getRange(AttrHandle attr, double &min, double &step, double &max) const = 0;
	virtual Result<void> getAvailable(AttrHandle attr, QStringList &values) const = 0;
};

} // namespace scopy::iio
