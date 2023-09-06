#include "tabinfo.hpp"


TabInfo::TabInfo(iio_device *dev, QString deviceName, QString xmlPath, QObject *parent)
: QObject{parent},
    dev(dev),
    deviceName(deviceName),
    xmlPath(xmlPath)
{

}

iio_device *TabInfo::getDev() const
{
    return dev;
}

QString TabInfo::getDeviceName() const
{
    return deviceName;
}

QString TabInfo::getXmlPath() const
{
    return xmlPath;
}
