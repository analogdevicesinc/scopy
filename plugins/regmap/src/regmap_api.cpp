#include "regmap_api.h"

using namespace scopy::regmap;

Q_LOGGING_CATEGORY(CAT_REGMAP_API, "RegMap_API")

RegMap_API::RegMap_API(RegmapPlugin *regMapPlugin)
	: ApiObject()
	, m_regMapPlugin(regMapPlugin)
{}

RegMap_API::~RegMap_API() {}

void RegMap_API::write(QString addr, QString val)
{
	DeviceRegisterMap *devRegMap = getActiveDevRegMap();
	Q_EMIT devRegMap->registerMapValues->requestWrite(addr.toUInt(nullptr, 16), val.toUInt(nullptr, 16));
}

QStringList RegMap_API::getAvailableDevicesName()
{
	QStringList devicesName;
	if(m_regMapPlugin->m_deviceList->size() != 0) {
		for(const auto &dev : *(m_regMapPlugin->m_deviceList)) {
			devicesName.append(iio_device_get_name(dev));
		}
	} else {
		qWarning(CAT_REGMAP_API) << "No devices available";
	}
	return devicesName;
}

bool RegMap_API::setDevice(QString device)
{
	m_regMapPlugin->registerMapTool->updateActiveRegisterMap(device);
	QString currentRegMap = m_regMapPlugin->registerMapTool->activeRegisterMap;
	if(currentRegMap == device) {
		return true;
	}
	qWarning(CAT_REGMAP_API) << "Device " << device << " was not set";
	return false;
}

QStringList RegMap_API::search(QString searchParam)
{
	QList<uint32_t> resultIndexes;
	DeviceRegisterMap *devRegMap = getActiveDevRegMap();
	resultIndexes = Search::searchForRegisters(devRegMap->registerMapTemplate->getRegisterList(), searchParam);

	QStringList resultList;
	for(int i = 0; i < resultIndexes.size(); i++) {
		RegisterModel *model = devRegMap->registerMapTemplate->getRegisterTemplate(resultIndexes[i]);
		resultList.append(model->getName());
	}
	return resultList;
}

QStringList RegMap_API::readInterval(QString startAddr, QString stopAddr)
{
	DeviceRegisterMap *devRegMap = getActiveDevRegMap();
	uint32_t start = startAddr.toUInt(nullptr, 16);
	uint32_t stop = stopAddr.toUInt(nullptr, 16);
	QStringList readValues;
	for(int i = start; i < stop; i++) {
		Q_EMIT devRegMap->registerMapValues->requestRead(i);
	}
	return readValues;
}

bool RegMap_API::enableAutoread(bool enable)
{
	DeviceRegisterMap *devRegMap = getActiveDevRegMap();
	devRegMap->toggleAutoread(enable);
	if(devRegMap->getAutoread() == enable) {
		return true;
	}
	qWarning(CAT_REGMAP_API) << "Autoread was not changed to " << enable;
	return false;
}

bool RegMap_API::isAutoreadEnabled()
{
	DeviceRegisterMap *devRegMap = getActiveDevRegMap();
	return devRegMap->getAutoread();
}

void RegMap_API::registerDump(QString filePath)
{
	DeviceRegisterMap *devRegMap = getActiveDevRegMap();
	Q_EMIT devRegMap->registerMapValues->registerDump(filePath);
}

void RegMap_API::writeFromFile(QString filePath)
{

}

DeviceRegisterMap *RegMap_API::getActiveDevRegMap()
{
	return m_regMapPlugin->registerMapTool->deviceList->value(m_regMapPlugin->registerMapTool->activeRegisterMap);
}
