/*
 * Copyright 2017 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "regmapparser.h"

#include "string.h"

RegmapParser::RegmapParser(QObject *parent, struct iio_context *context) : QObject(parent), ctx(context) {}

void RegmapParser::setIioContext(struct iio_context *ctx) { this->ctx = ctx; }

void RegmapParser::deviceXmlFileSelection(const QString *device, QString *filename, const QString source) {
	struct iio_device *dev = iio_context_find_device(ctx, device->toLatin1().data());

	filename->clear();

	if (source.contains("SPI")) {
		findDeviceXmlFile(&xmlsFolderPath, device, filename);
	} else {
		if (source.contains("AXI CORE")) {
			QString adcRegmapName, dacRegmapName;
			int pcoreMajor;

			if (pcoreGetVersion(device, &pcoreMajor) && pcoreMajor > 8) {
				adcRegmapName.append(QString("adi_regmap_adc_v%1.xml").arg(pcoreMajor));
				dacRegmapName.append(QString("adi_regmap_dac_v%1.xml").arg(pcoreMajor));
			} else {
				adcRegmapName.append("adi_regmap_adc.xml");
				dacRegmapName.append("adi_regmap_dac.xml");
			}

			/* Attempt to associate AXI Core ADC xml or AXI Core DAC
			 * xml to the device */
			if (isInputDevice(dev) && xmlFileExists(adcRegmapName.toLatin1().data())) {
				filename->append(adcRegmapName);
			} else if (isOutputDevice(dev) && xmlFileExists(dacRegmapName.toLatin1().data())) {
				filename->append(dacRegmapName);
			}
		} else {
			filename->clear();
		}
	}
}

void RegmapParser::findDeviceXmlFile(const QString *xmlsFolderPath, const QString *device, QString *filename) {
	filename->append(xmlsFolderPath->toLatin1().data());
	filename->append(device->toLatin1().data());
	filename->append(".xml");

	QFile file(filename->toLatin1().data());

	if (!file.exists()) {
		filename->clear();
	}
}

int RegmapParser::pcoreGetVersion(const QString *device, int *pcoreMajor) {
	struct iio_device *dev;
	int ret;

	dev = iio_context_find_device(ctx, device->toLatin1().data());

	uint32_t value, address = 0x80000000;
	ret = iio_device_reg_read(dev, address, &value);

	*pcoreMajor = (int)PCORE_VERSION_MAJOR(value);

	return ret;
}

bool RegmapParser::xmlFileExists(char *filename) {
	QString fileToOpen;

	fileToOpen.append(xmlsFolderPath.toLatin1());
	fileToOpen.append(filename);
	QFile file(fileToOpen);

	return file.exists();
}
int RegmapParser::deviceXmlFileLoad(QString *filename) {
	if (!file.isOpen()) {
		file.setFileName(*filename);

		if (!file.exists()) {
			qDebug() << "No file available";
		}

		/*Xml content loaded inside doc*/
		doc.clear();

		if (!file.open(QIODevice::ReadOnly) || !doc.setContent(&file)) {
			return 0;
		}
	}

	file.close();
	return 1;
}

QDomNode *RegmapParser::getRegisterNode(const QString address) {
	int i;
	bool status;
	QDomNodeList registers = doc.elementsByTagName("Register");
	uint32_t lastAddress = registers.item(registers.size() - 1)
				       .firstChildElement("Address")
				       .text()
				       .split("0x")[1]
				       .toUInt(&status, 16);
	lastNode = registers.item(registers.size() - 1);

	for (i = 0; i < registers.size(); i++) {
		node = registers.item(i);
		QDomElement addr = node.firstChildElement("Address");

		uint hexNode = addr.text().split("0x")[1].toUInt(&status, 16);
		uint hexAddress = address.split("0x")[1].toUInt(&status, 16);

		if (hexAddress > lastAddress) {
			return &lastNode;
		}

		if (addr.isNull()) {
			continue;
		}

		if (hexNode == hexAddress) {
			return &node;
		}
	}

	return nullptr;
}

uint32_t RegmapParser::getLastAddress(void) const {
	bool status;

	QDomNodeList registers = doc.elementsByTagName("Register");
	QDomNode n = registers.item(registers.size() - 1);
	QDomElement addr = n.firstChildElement("Address");
	QStringList temp = addr.text().split("0x");

	uint32_t lastAddress = addr.text().split("0x")[1].toUInt(&status, 16);

	return lastAddress;
}

bool RegmapParser::isInputDevice(const struct iio_device *dev) { return deviceTypeGet(dev, 1); }

bool RegmapParser::isOutputDevice(const iio_device *dev) { return deviceTypeGet(dev, 0); }

bool RegmapParser::deviceTypeGet(const iio_device *dev, int type) {
	struct iio_channel *ch;
	int nbChannels, i;

	if (!dev) {
		return false;
	}

	nbChannels = iio_device_get_channels_count(dev);

	for (i = 0; i < nbChannels; i++) {
		ch = iio_device_get_channel(dev, i);

		if (iio_channel_is_scan_element(ch) &&
		    (type ? !iio_channel_is_output(ch) : iio_channel_is_output(ch))) {
			return true;
		}
	}

	return false;
}

uint32_t RegmapParser::readRegister(const QString *device, const uint32_t u32Address) {
	struct iio_device *dev = iio_context_find_device(ctx, device->toLatin1().data());
	int ret;
	uint32_t i;

	ret = iio_device_reg_read(dev, u32Address, &i);

	if (ret == 0) {
		return i;
	}

	else {
		return -1;
	}
}

void RegmapParser::writeRegister(const QString *device, const uint32_t u32Address, const uint32_t value) {
	struct iio_device *dev = iio_context_find_device(ctx, device->toLatin1().data());

	iio_device_reg_write(dev, u32Address, value);
}
