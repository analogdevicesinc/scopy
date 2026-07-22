#pragma once

#include "iioutil/icontextops.h"

namespace scopy::iio {

class V0ContextOps : public IContextOps {
public:
	ContextHandle createContext(const QString &uri, const ContextParams &params = {}) override;
	void destroyContext(ContextHandle ctx) override;
	QString name(ContextHandle ctx) const override;
	QString description(ContextHandle ctx) const override;
	void version(ContextHandle ctx, unsigned int &major, unsigned int &minor, QString &tag) const override;
	unsigned int devicesCount(ContextHandle ctx) const override;
	DeviceHandle getDevice(ContextHandle ctx, unsigned int index) const override;
	DeviceHandle findDevice(ContextHandle ctx, const QString &name) const override;
	unsigned int attrsCount(ContextHandle ctx) const override;
	QString attrName(ContextHandle ctx, unsigned int index) const override;

	bool ping(ContextHandle ctx) const override;
};

} // namespace scopy::iio
