#pragma once

#include "iioutil/handles.h"
#include <QString>

namespace scopy::iio {

class IContextOps {
public:
	virtual ~IContextOps() = default;

	virtual ContextHandle createContext(const QString &uri, const ContextParams &params = {}) = 0;
	virtual void destroyContext(ContextHandle ctx) = 0;
	virtual QString name(ContextHandle ctx) const = 0;
	virtual QString description(ContextHandle ctx) const = 0;
	virtual void version(ContextHandle ctx, unsigned int &major, unsigned int &minor, QString &tag) const = 0;
	virtual unsigned int devicesCount(ContextHandle ctx) const = 0;
	virtual DeviceHandle getDevice(ContextHandle ctx, unsigned int index) const = 0;
	virtual DeviceHandle findDevice(ContextHandle ctx, const QString &name) const = 0;
	virtual unsigned int attrsCount(ContextHandle ctx) const = 0;
	virtual QString attrName(ContextHandle ctx, unsigned int index) const = 0;

	// Returns true if the context is still reachable, false if the connection is lost.
	virtual bool ping(ContextHandle ctx) const = 0;
};

} // namespace scopy::iio
