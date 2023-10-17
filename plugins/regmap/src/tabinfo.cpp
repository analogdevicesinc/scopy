#include "tabinfo.hpp"

TabInfo::TabInfo(iio_device *dev, QString deviceName, QString xmlPath, bool isAxi, QObject *parent)
	: QObject{parent}
	, dev(dev)
	, deviceName(deviceName)
	, xmlPath(xmlPath)
	, isAxi(isAxi)
{}

iio_device *TabInfo::getDev() const { return dev; }

QString TabInfo::getDeviceName() const { return deviceName; }

QString TabInfo::getXmlPath() const { return xmlPath; }

bool TabInfo::getIsAxi() const { return isAxi; }
