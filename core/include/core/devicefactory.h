#ifndef DEVICEFACTORY_H
#define DEVICEFACTORY_H

#include "deviceimpl.h"

#include <QObject>

namespace scopy {

class DeviceFactory : public QObject
{
	Q_OBJECT
public:
	static DeviceImpl *build(QString param, PluginManager *pm, QString category = "", QObject *parent = nullptr);
};
} // namespace scopy
#endif // DEVICEFACTORY_H
