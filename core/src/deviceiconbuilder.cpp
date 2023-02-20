#include "deviceiconbuilder.h"
#include "deviceiconimpl.h"

using namespace adiscope;
DeviceIconBuilder::DeviceIconBuilder(QObject *parent) // creates name, uri , icon, forgetbtn, connection icon, etc
	: QObject{parent}
{

}
DeviceIconBuilder::~DeviceIconBuilder()
{

}
DeviceIcon* DeviceIconBuilder::build(QString name, QString id, QWidget *icon, QWidget *parent)
{
	return new DeviceIconImpl(name, id, icon, parent);
}
