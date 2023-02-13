#include "deviceiconbuilder.h"
#include "deviceicon.h"

using namespace adiscope;
DeviceIconBuilder::DeviceIconBuilder(QObject *parent) // creates name, uri , icon, forgetbtn, connection icon, etc
	: QObject{parent}
{

}
DeviceIconBuilder::~DeviceIconBuilder()
{

}
QAbstractButton* DeviceIconBuilder::build(QString name, QString id, QWidget *icon, QWidget *parent)
{
	return new DeviceIcon(name, id, icon, parent);
}
