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

#include "debug.h"

static const size_t maxAttrSize = 512;

using namespace adiscope;

Debug::Debug(QObject *parent) : QObject(parent) {}

Debug::~Debug() {}

void Debug::scanDevices(void) {
	size_t Size;
	struct iio_context_info **info;
	struct iio_scan_context *scan_ctx;
	struct iio_device *device;

	scan_ctx = iio_create_scan_context(NULL, 0);
	Size = iio_scan_context_get_info_list(scan_ctx, &info);

	if (Size) {
		connected = true;

		/*Display context info*/
		for (size_t i = 0; i < (size_t)Size; i++) {
			qDebug("\t %d: %s [%s]\n\r", (int)i, iio_context_info_get_description(info[i]),
			       iio_context_info_get_uri(info[i]));
		}

		/*Create context*/
		ctx = iio_create_context_from_uri(iio_context_info_get_uri(info[0]));

		Size = iio_context_get_devices_count(ctx);

		for (size_t i = 0; i < (size_t)Size; i++) {
			device = iio_context_get_device(ctx, i);

			deviceList.append(iio_device_get_name(device));
		}
	} else {
		connected = false;
	}
}

void Debug::setIioContext(struct iio_context *ctx) {
	struct iio_device *device;
	int Size;

	this->ctx = ctx;
	connected = true;

	Size = iio_context_get_devices_count(ctx);

	for (size_t i = 0; i < (size_t)Size; i++) {
		device = iio_context_get_device(ctx, i);

		deviceList.append(iio_device_get_name(device));
	}
}

QStringList Debug::getDeviceList() const { return deviceList; }

QStringList Debug::getChannelList() const { return channelList; }

QStringList Debug::getAttributeList() const { return attributeList; }

QStringList Debug::getFileName() const { return filename; }

QVector<QString> Debug::getAttributeVector() const { return attributeAvailable; }

void Debug::scanChannels(const QString &devName) {
	int nb_channels;
	struct iio_device *device;
	struct iio_channel *ch;
	QString type;

	if (connected) {
		device = iio_context_find_device(ctx, devName.toLatin1().data());
		nb_channels = iio_device_get_channels_count(device);

		channelList.clear();

		for (int j = 0; j < nb_channels; j++) {
			/*get channel*/
			ch = iio_device_get_channel(device, j);

			if (iio_channel_is_output(ch)) {
				type = "output ";
			} else {
				type = "input ";
			}

			type.append(QString(iio_channel_get_id(ch)));
			channelList << type;
		}

		channelList << QString("Global");
		Q_EMIT channelsChanged(channelList);
	}
}

void Debug::scanChannelAttributes(QString devName, QString &channel) {
	struct iio_device *device;
	struct iio_channel *ch;
	const char *attr;
	unsigned int nb_attrs;
	bool isOutput;

	if (connected) { // check if M2K is connected
		isOutput = channel.contains("output", Qt::CaseInsensitive);

		if (isOutput) {
			channel.remove("output ", Qt::CaseInsensitive);
		} else {
			channel.remove("input ", Qt::CaseInsensitive);
		}

		device = iio_context_find_device(ctx, devName.toLatin1().data());
		attributeList.clear();
		attributeAvailable.clear();
		filename.clear();

		if (channel != QString("Global")) {
			ch = iio_device_find_channel(device, channel.toLatin1().data(), isOutput);

			nb_attrs = iio_channel_get_attrs_count(ch);

			for (unsigned int k = 0; k < nb_attrs; k++) {
				attr = iio_channel_get_attr(ch, k);

				if (QString(attr).contains("available", Qt::CaseInsensitive)) {
					attributeAvailable.append(QString(attr));
				} else {
					filename << QString(iio_channel_attr_get_filename(ch, attr));
					attributeList << QString(attr);
				}
			}
		} else {
			nb_attrs = iio_device_get_attrs_count(device);

			for (unsigned int k = 0; k < nb_attrs; k++) {
				attr = iio_device_get_attr(device, k);

				if (QString(attr).contains("available", Qt::CaseInsensitive)) {
					attributeAvailable.append(QString(attr));
				} else {
					filename << QString(attr);
					attributeList << QString(attr);
				}
			}
		}
	}
}

QStringList Debug::getAvailableValues(const QString &devName, QString &channel, QString &attribute) const {
	struct iio_device *device;
	struct iio_channel *ch;
	char value[maxAttrSize];
	QStringList values;
	bool isOutput;

	attribute.append("_available");

	if (channel.contains("Global", Qt::CaseInsensitive)) {
		channel.clear();
	}

	if (connected) {

		device = iio_context_find_device(ctx, devName.toLatin1().data());

		if (channel.isNull()) {
			iio_device_attr_read(device, attribute.toLatin1().data(), value, maxAttrSize);
		} else {
			isOutput = channel.contains("output", Qt::CaseInsensitive);

			if (isOutput) {
				channel.remove("output ", Qt::CaseInsensitive);
			} else {
				channel.remove("input ", Qt::CaseInsensitive);
			}

			ch = iio_device_find_channel(device, channel.toLatin1().data(), isOutput);
			iio_channel_attr_read(ch, attribute.toLatin1().data(), value, maxAttrSize);
		}

		values = QString(value).split(' ');

		return values;
	} else {
		values.clear();
		return values;
	}
}

QString Debug::readAttribute(const QString &devName, QString &channel, const QString &attribute) {
	struct iio_device *device;
	struct iio_channel *ch;
	char value[maxAttrSize];
	bool isOutput;

	if (connected) {
		device = iio_context_find_device(ctx, devName.toLatin1().data());

		if (channel.isNull()) {
			iio_device_attr_read(device, attribute.toLatin1().data(), value, maxAttrSize);
		} else {
			isOutput = channel.contains("output", Qt::CaseInsensitive);

			if (isOutput) {
				channel.remove("output ", Qt::CaseInsensitive);
			} else {
				channel.remove("input ", Qt::CaseInsensitive);
			}

			ch = iio_device_find_channel(device, channel.toLatin1().data(), isOutput);
			iio_channel_attr_read(ch, attribute.toLatin1().data(), value, maxAttrSize);
		}
	}

	return QString(value);
}

void Debug::writeAttribute(const QString &devName, QString &channel, const QString &attribute, const QString &value) {
	struct iio_device *device;
	struct iio_channel *ch;
	bool isOutput;

	if (connected) {
		device = iio_context_find_device(ctx, devName.toLatin1().data());

		if (channel.isNull()) {
			iio_device_attr_write(device, attribute.toLatin1().data(), value.toLatin1().data());

		} else {
			isOutput = channel.contains("output", Qt::CaseInsensitive);

			if (isOutput) {
				channel.remove("output ", Qt::CaseInsensitive);
			} else {
				channel.remove("input ", Qt::CaseInsensitive);
			}

			ch = iio_device_find_channel(device, channel.toLatin1().data(), isOutput);
			iio_channel_attr_write(ch, attribute.toLatin1().data(), value.toLatin1().data());
		}
	}
}

struct iio_context *Debug::getIioContext(void) {
	return ctx;
}
