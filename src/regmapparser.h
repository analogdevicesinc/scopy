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

#ifndef REGMAPPARSER_H
#define REGMAPPARSER_H

#include <iio.h>

#include <QDebug>
#include <QDomDocument>
#include <QFile>
#include <QObject>

#define PCORE_VERSION_MAJOR(version) (version >> 16)

class RegmapParser : public QObject {
	Q_OBJECT

public:
	explicit RegmapParser(QObject *parent = nullptr,
			      struct iio_context *context = nullptr);
	void deviceXmlFileSelection(const QString *device, QString *filename,
				    const QString source);
	int deviceXmlFileLoad(QString *filename);
	void regMapChooserInit(QString *device);
	QDomNode *getRegisterNode(const QString address);
	void setIioContext(struct iio_context *ctx);
	uint32_t readRegister(const QString *device, const uint32_t u8Address);
	void writeRegister(const QString *device, const uint32_t u8Address,
			   const uint32_t value);
	uint32_t getLastAddress(void) const;

private:
	void findDeviceXmlFile(const QString *xmlsFolderPath,
			       const QString *device, QString *filename);
	int pcoreGetVersion(const QString *device, int *pcoreMajor);
	bool xmlFileExists(char *filename);
	bool isInputDevice(const struct iio_device *dev);
	bool isOutputDevice(const struct iio_device *dev);
	bool deviceTypeGet(const struct iio_device *dev, int type);

private:
	struct iio_context *ctx;
	QString xmlsFolderPath;
	QFile file;
	QDomDocument doc;
	QDomNode node;
	QDomNode lastNode;
};

#endif // REGMAPPARSER_H
