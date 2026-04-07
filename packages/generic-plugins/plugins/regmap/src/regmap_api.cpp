/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "regmap_api.h"

using namespace scopy::regmap;

Q_LOGGING_CATEGORY(CAT_REGMAP_API, "RegMap_API")

RegMap_API::RegMap_API(RegmapPlugin *regMapPlugin)
	: ApiObject()
	, m_regMapPlugin(regMapPlugin)
{}

RegMap_API::~RegMap_API() {}

void RegMap_API::write(const QString &addr, const QString &val)
{
	DeviceRegisterMap *devRegMap = getActiveDevRegMap();
	if(!devRegMap) {
		qWarning(CAT_REGMAP_API) << "No active device register map";
		return;
	}
	uint32_t address = Utils::convertQStringToUint32(addr);
	uint32_t value = Utils::convertQStringToUint32(val);
	Q_EMIT devRegMap->registerMapValues->requestWrite(address, value);
}

void RegMap_API::writeBitField(const QString &addr, const QString &val)
{
	DeviceRegisterMap *devRegMap = getActiveDevRegMap();
	if(!devRegMap) {
		qWarning(CAT_REGMAP_API) << "No active device register map";
		return;
	}
	uint32_t address = Utils::convertQStringToUint32(addr);
	devRegMap->registerController->registerChanged(address);
	Q_EMIT devRegMap->registerDetailedWidget->bitFieldValueChanged(val);
	write(addr, devRegMap->registerController->regValue->text());
}

QStringList RegMap_API::getAvailableDevicesName()
{
	if(!m_regMapPlugin || !m_regMapPlugin->registerMapTool || !m_regMapPlugin->registerMapTool->deviceList) {
		qWarning(CAT_REGMAP_API) << "Register map tool not initialized";
		return {};
	}
	auto devices = m_regMapPlugin->registerMapTool->deviceList;
	return devices->keys();
}

bool RegMap_API::setDevice(const QString &device)
{
	if(!m_regMapPlugin || !m_regMapPlugin->registerMapTool) {
		qWarning(CAT_REGMAP_API) << "Register map tool not initialized";
		return false;
	}
	m_regMapPlugin->registerMapTool->updateActiveRegisterMap(device);
	QString currentRegMap = m_regMapPlugin->registerMapTool->activeRegisterMap;
	if(currentRegMap == device) {
		return true;
	}
	qWarning(CAT_REGMAP_API) << "Device " << device << " was not set";
	return false;
}

QList<QString> RegMap_API::search(const QString &searchParam)
{
	QList<uint32_t> resultIndexes;
	DeviceRegisterMap *devRegMap = getActiveDevRegMap();
	if(!devRegMap || !devRegMap->registerMapTemplate) {
		qWarning(CAT_REGMAP_API) << "No active device register map";
		return {};
	}
	resultIndexes = Search::searchForRegisters(devRegMap->registerMapTemplate->getRegisterList(), searchParam);
	QList<QString> resultList;

	for(int i = 0; i < resultIndexes.size(); i++) {
		resultList.append(QString::number(resultIndexes[i]));
	}
	return resultList;
}

void RegMap_API::readInterval(const QString &startAddr, const QString &stopAddr)
{
	DeviceRegisterMap *devRegMap = getActiveDevRegMap();
	if(!devRegMap) {
		qWarning(CAT_REGMAP_API) << "No active device register map";
		return;
	}
	uint32_t start = Utils::convertQStringToUint32(startAddr);
	uint32_t stop = Utils::convertQStringToUint32(stopAddr);
	for(int i = start; i <= stop; i++) {
		Q_EMIT devRegMap->requestRead(i);
	}
}

bool RegMap_API::enableAutoread(bool enable)
{
	DeviceRegisterMap *devRegMap = getActiveDevRegMap();
	if(!devRegMap) {
		qWarning(CAT_REGMAP_API) << "No active device register map";
		return false;
	}
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
	if(!devRegMap) {
		qWarning(CAT_REGMAP_API) << "No active device register map";
		return false;
	}
	return devRegMap->getAutoread();
}

void RegMap_API::registerDump(const QString &filePath)
{
	DeviceRegisterMap *devRegMap = getActiveDevRegMap();
	if(!devRegMap || !devRegMap->registerMapValues) {
		qWarning(CAT_REGMAP_API) << "No active device register map";
		return;
	}
	Q_EMIT devRegMap->registerMapValues->registerDump(filePath);
}

void RegMap_API::setPath(const QString &filePath)
{
	if(!m_regMapPlugin || !m_regMapPlugin->registerMapTool || !m_regMapPlugin->registerMapTool->settings ||
	   !m_regMapPlugin->registerMapTool->settings->fileBrowser) {
		qWarning(CAT_REGMAP_API) << "Register map settings not initialized";
		return;
	}
	m_regMapPlugin->registerMapTool->settings->fileBrowser->lineEdit()->setText(filePath);
}

void RegMap_API::writeFromFile(const QString &filePath)
{
	if(!m_regMapPlugin || !m_regMapPlugin->registerMapTool || !m_regMapPlugin->registerMapTool->settings ||
	   !m_regMapPlugin->registerMapTool->settings->writeListOfValuesButton) {
		qWarning(CAT_REGMAP_API) << "Register map settings not initialized";
		return;
	}
	setPath(filePath);
	Q_EMIT m_regMapPlugin->registerMapTool->settings->writeListOfValuesButton->clicked();
}

QString RegMap_API::readRegister(const QString &addr)
{
	DeviceRegisterMap *devRegMap = getActiveDevRegMap();
	if(!devRegMap || !devRegMap->registerController || !devRegMap->registerController->regValue) {
		qWarning(CAT_REGMAP_API) << "No active device register map";
		return "";
	}
	Q_EMIT devRegMap->requestRead(Utils::convertQStringToUint32(addr));
	return devRegMap->registerController->regValue->text();
}

QString RegMap_API::getValueOfRegister(const QString &addr)
{
	DeviceRegisterMap *devRegMap = getActiveDevRegMap();
	if(!devRegMap || !devRegMap->registerMapValues) {
		qWarning(CAT_REGMAP_API) << "No active device register map";
		return "";
	}
	uint32_t address = Utils::convertQStringToUint32(addr);
	QMap<uint32_t, uint32_t> *values = devRegMap->registerMapValues->getRegisterReadValues();
	if(values->keys().contains(address)) {
		return Utils::convertToHexa(values->value(address), 16);
	}
	qWarning(CAT_REGMAP_API) << "Value not read";
	return "";
}

DeviceRegisterMap *RegMap_API::getActiveDevRegMap()
{
	if(!m_regMapPlugin || !m_regMapPlugin->registerMapTool || !m_regMapPlugin->registerMapTool->deviceList) {
		return nullptr;
	}
	return m_regMapPlugin->registerMapTool->deviceList->value(m_regMapPlugin->registerMapTool->activeRegisterMap);
}

QStringList RegMap_API::getRegisterInfo(const QString &addr)
{
	DeviceRegisterMap *devRegMap = getActiveDevRegMap();
	if(!devRegMap || !devRegMap->registerMapTableWidget || !devRegMap->registerMapTableWidget->registerModels) {
		qWarning(CAT_REGMAP_API) << "No active device register map";
		return {};
	}
	QStringList regInfo;
	RegisterModel *regModel =
		devRegMap->registerMapTableWidget->registerModels->value(Utils::convertQStringToUint32(addr));
	if(regModel != nullptr) {
		regInfo.append("Name:" + regModel->getName());
		regInfo.append("Address:" + Utils::convertToHexa(regModel->getAddress(), 16));
		regInfo.append("Description:" + regModel->getDescription());
		regInfo.append("Notes:" + regModel->getNotes());
	}
	return regInfo;
}

QStringList RegMap_API::getRegisterBitFieldsInfo(const QString &addr)
{
	DeviceRegisterMap *devRegMap = getActiveDevRegMap();
	if(!devRegMap || !devRegMap->registerMapTableWidget || !devRegMap->registerMapTableWidget->registerModels) {
		qWarning(CAT_REGMAP_API) << "No active device register map";
		return {};
	}
	QStringList bitFieldsDetails;
	RegisterModel *regModel =
		devRegMap->registerMapTableWidget->registerModels->value(Utils::convertQStringToUint32(addr));
	if(!regModel) {
		qWarning(CAT_REGMAP_API) << "Register model not found for address " << addr;
		return {};
	}
	QVector<BitFieldModel *> *bitField = regModel->getBitFields();
	if(!bitField) {
		return {};
	}

	for(BitFieldModel *bf : *bitField) {
		bitFieldsDetails.append("Name:" + bf->getName());
		bitFieldsDetails.append("Description:" + bf->getDescription());
		bitFieldsDetails.append("Notes:" + bf->getNotes());
		bitFieldsDetails.append("Default value:" + Utils::convertToHexa(bf->getDefaultValue(), 16));
		bitFieldsDetails.append(";");
	}
	return bitFieldsDetails;
}

QStringList RegMap_API::getBitFieldInfo(const QString &addr, const QString &bitName)
{
	DeviceRegisterMap *devRegMap = getActiveDevRegMap();
	if(!devRegMap || !devRegMap->registerMapTableWidget || !devRegMap->registerMapTableWidget->registerModels) {
		qWarning(CAT_REGMAP_API) << "No active device register map";
		return {};
	}
	QStringList bitDetails;
	RegisterModel *regModel =
		devRegMap->registerMapTableWidget->registerModels->value(Utils::convertQStringToUint32(addr));
	if(!regModel) {
		qWarning(CAT_REGMAP_API) << "Register model not found for address " << addr;
		return {};
	}
	QVector<BitFieldModel *> *bitField = regModel->getBitFields();
	if(!bitField) {
		return {};
	}

	for(BitFieldModel *bf : *bitField) {
		if(bf->getName() == bitName) {
			bitDetails.append("Name:" + bf->getName());
			bitDetails.append("Description:" + bf->getDescription());
			bitDetails.append("Notes:" + bf->getNotes());
			bitDetails.append("Default value:" + Utils::convertToHexa(bf->getDefaultValue(), 16));
			break;
		}
	}
	return bitDetails;
}
