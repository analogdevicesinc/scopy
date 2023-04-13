#include "devicefactory.h"

using namespace scopy;


DeviceImpl *DeviceFactory::build(QString param, PluginManager *pm, QString category, QObject *parent)
{
	return new DeviceImpl(param, pm, category, parent);
}
