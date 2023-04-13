#ifndef DEVICEFACTORY_H
#define DEVICEFACTORY_H

#include <QObject>
#include "deviceimpl.h"

namespace scopy {


class DeviceFactory : public QObject {
	Q_OBJECT
public:
	static DeviceImpl* build(QString param, PluginManager *pm, QString category = "",QObject *parent = nullptr);

};
}
#endif // DEVICEFACTORY_H
