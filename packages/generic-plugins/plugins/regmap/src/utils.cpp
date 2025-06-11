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

#include "utils.hpp"

#include "jsonformatedelement.hpp"
#include "logging_categories.h"
#include "scopy-regmap_config.h"

#include <QApplication>
#include <QDir>
#include <QLayout>
#include <QPushButton>
#include <qjsonarray.h>
#include <qjsondocument.h>
#include <qjsonobject.h>

#include <pluginbase/preferences.h>

using namespace scopy::regmap;

QMap<QString, JsonFormatedElement *> *Utils::spiJson{new QMap<QString, JsonFormatedElement *>()};
QMap<QString, JsonFormatedElement *> *Utils::axiJson{new QMap<QString, JsonFormatedElement *>()};

Utils::Utils(QObject *parent)
	: QObject{parent}
{}

QString Utils::convertToHexa(uint32_t value, int size)
{
	return QStringLiteral("0x%1").arg(value, (size / 4), 16, QLatin1Char('0'));
}

uint32_t Utils::convertQStringToUint32(QString value)
{
	bool ok;
	uint32_t convertedValue = static_cast<uint32_t>(value.toLongLong(&ok, 16));
	return convertedValue;
}

uint32_t Utils::getBitMask(int offset, int width)
{
	if(width == 32) {
		return static_cast<uint32_t>(0xffffffff);
	}

	return (1 << (offset + width)) - 1;
}

void Utils::removeLayoutMargins(QLayout *layout)
{
	layout->setMargin(0);
	layout->setContentsMargins(0, 0, 0, 0);
}

QDir Utils::setXmlPath()
{
	QDir xmlsPath(REGMAP_XML_BUILD_PATH);
	if(xmlsPath.entryList().empty()) {
#ifdef Q_OS_WINDOWS
		xmlsPath.setPath(REGMAP_XML_PATH_LOCAL);
#elif defined __APPLE__
		xmlsPath.setPath(QCoreApplication::applicationDirPath() + "/plugins/xmls");
#elif defined(__appimage__)
		xmlsPath.setPath(QCoreApplication::applicationDirPath() + "/../lib/scopy/plugins/xmls");
#else
		xmlsPath.setPath(REGMAP_XML_SYSTEM_PATH);
#endif
	}

	qDebug(CAT_REGMAP) << "XML folder found: " << xmlsPath;
	if(!xmlsPath.entryList().empty()) {
		return xmlsPath;
	}

	qDebug(CAT_REGMAP) << "No XML folder found";
	return QDir("");
}

int Utils::getBitsPerRowDetailed() { return bitsPerRowDetailed; }

JsonFormatedElement *Utils::getJsonTemplate(QString xml)
{
	if(spiJson->contains(xml)) {
		return spiJson->value(xml);
	}

	if(axiJson->contains(xml)) {
		return axiJson->value(xml);
	}

	return nullptr;
}

JsonFormatedElement *Utils::getTemplate(QString devName)
{
	// search for SPI template
	foreach(QString key, spiJson->keys()) {
		for(int i = 0; i < spiJson->value(key)->getCompatibleDevices()->size(); i++) {
			if(spiJson->value(key)->getCompatibleDevices()->at(i).contains(devName)) {
				return spiJson->value(key);
			}
		}
	}
	// search for AXI template
	foreach(QString key, axiJson->keys()) {
		for(int i = 0; i < axiJson->value(key)->getCompatibleDevices()->size(); i++) {
			if(axiJson->value(key)->getCompatibleDevices()->at(i).contains(devName)) {
				return axiJson->value(key);
			}
		}
	}

	return nullptr;
}

void Utils::applyJsonConfig()
{
	QDir xmlsPath = Utils::setXmlPath();
	QString filePath = xmlsPath.filePath("regmap-config.json");
	generateJsonTemplate(filePath);
}

void Utils::getConfigurationFromJson(QString filePath)
{
	QString val;
	QFile file;
	file.setFileName(filePath);
	file.open(QIODevice::ReadOnly | QIODevice::Text);
	val = file.readAll();
	file.close();

	QJsonDocument d = QJsonDocument::fromJson(val.toUtf8());

	QJsonObject obj = d.object();

	QJsonArray jsonSpiArray = obj.value(QString("spi")).toArray();
	populateJsonTemplateMap(jsonSpiArray, false);
	QJsonArray jsonAxiArray = obj.value(QString("axi")).toArray();
	populateJsonTemplateMap(jsonAxiArray, true);
}

void Utils::populateJsonTemplateMap(QJsonArray jsonArray, bool isAxi)
{
	for(auto object : jsonArray) {

		QString fileName = object.toObject().value(QString("file_name")).toString();
		bool useRegisterDescriptionAsName =
			object.toObject().value(QString("use_register_description_as_name")).toBool();
		bool useBifieldDescriptionAsName =
			object.toObject().value(QString("use_bitfield_description_as_name")).toBool();
		int bitsPerRow = object.toObject().value(QString("bits_per_row")).toInt();

		if(bitsPerRow == 0) {
			bitsPerRow = 8;
		}

		QList<QString> *compatibleDevicesList = new QList<QString>();
		QJsonArray compatibleDevices = object.toObject().value(QString("compatible_drivers")).toArray();

		qDebug(CAT_REGMAP) << "fileName : " << fileName;
		qDebug(CAT_REGMAP) << "useRegisterDescriptionAsName : " << useRegisterDescriptionAsName;
		qDebug(CAT_REGMAP) << "useBifieldDescriptionAsName : " << useBifieldDescriptionAsName;
		qDebug(CAT_REGMAP) << "bitsPerRow : " << bitsPerRow;

		if(!compatibleDevices.isEmpty()) {
			for(auto device : compatibleDevices) {
				compatibleDevicesList->push_back(device.toString());
				qDebug(CAT_REGMAP) << "compatible device : " << device.toString();
			}
		}
		spiJson->insert(fileName,
				new JsonFormatedElement(fileName, compatibleDevicesList, isAxi,
							useRegisterDescriptionAsName, useBifieldDescriptionAsName,
							bitsPerRow));
	}
}

void Utils::generateJsonTemplate(QString filePath)
{
	getConfigurationFromJson(filePath);

	foreach(const QString &xmlName, Utils::setXmlPath().entryList()) {
		if(xmlName.contains(".xml")) {
			auto deviceName = xmlName.toLower();
			deviceName.chop(4);
			if(spiJson->contains(xmlName)) {
				spiJson->value(xmlName)->addCompatibleDevice(deviceName);
			} else {
				JsonFormatedElement *jsonFormatedElement = new JsonFormatedElement(xmlName);
				jsonFormatedElement->addCompatibleDevice(deviceName);
				spiJson->insert(xmlName, jsonFormatedElement);
			}
		}
	}
}
