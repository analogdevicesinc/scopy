#include "devicefactory.h"

using namespace adiscope;


DeviceImpl *DeviceFactory::build(QString param, PluginManager *pm, QString category, QObject *parent)
{
	return new DeviceImpl(param, pm, category, parent);
}
