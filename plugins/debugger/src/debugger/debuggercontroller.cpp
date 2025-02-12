/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "debuggercontroller.h"

#include <iioutil/iiocpp/iioscan.h>
#include <iioutil/iiocpp/iioresult.h>
#include <iioutil/iiocpp/iiocontext.h>
#include <iioutil/iiocpp/iiodevice.h>
#include <iioutil/iiocpp/iiochannel.h>
#include <iioutil/iiocpp/iioattribute.h>

static const size_t maxAttrSize = 512;

using namespace scopy;
using namespace scopy::debugger;

DebuggerController::DebuggerController(QObject *parent)
	: QObject(parent)
{
}

DebuggerController::~DebuggerController() {}

void DebuggerController::scanDevices(void)
{
	size_t Size;
	iio_device *device;

	IIOResult<struct iio_scan *> scanRes = IIOScan::scan(nullptr, nullptr);
	if(!scanRes.ok()) {
		qDebug() << "Error: " << scanRes.error();
		return;
	}
	struct iio_scan *scan = scanRes.data();

	Size = IIOScan::get_results_count(scan);

	if(Size) {
		connected = true;

		/*Display context info*/
		for(size_t i = 0; i < (size_t)Size; i++) {
			qDebug("\t %d: %s [%s]\n\r", (int)i, IIOScan::get_description(scan, i),
			       IIOScan::get_uri(scan, i));
		}

		/*Create context*/
		IIOResult<iio_context *> ctxRes = IIOContext::create_context(nullptr, IIOScan::get_uri(scan, 0));
		if(!ctxRes.ok()) {
			qDebug() << "Error creating ctx from scan: " << ctxRes.error();
			return;
		}
		ctx = ctxRes.data();

		Size = IIOContext::get_devices_count(ctx);

		for(size_t i = 0; i < (size_t)Size; i++) {
			device = IIOContext::get_device(ctx, i).expect(QString("Error getting device %1").arg(i));

			deviceList.append(IIODevice::get_name(device));
		}
	} else {
		connected = false;
	}
}

void DebuggerController::setIioContext(struct iio_context *ctx)
{
	iio_device *device;
	int Size;

	this->ctx = ctx;
	connected = true;

	Size = IIOContext::get_devices_count(ctx);

	for(size_t i = 0; i < (size_t)Size; i++) {
		device = IIOContext::get_device(ctx, i).expect(QString("Error getting device %1").arg(i));

		deviceList.append(IIODevice::get_name(device));
	}
}

QStringList DebuggerController::getDeviceList() const { return deviceList; }

QStringList DebuggerController::getChannelList() const { return channelList; }

QStringList DebuggerController::getAttributeList() const { return attributeList; }

QStringList DebuggerController::getFileName() const { return filename; }

QVector<QString> DebuggerController::getAttributeVector() const { return attributeAvailable; }

void DebuggerController::scanChannels(const QString &devName)
{
	int nb_channels;
	iio_device *device;
	const iio_channel *ch;
	QString type;

	if(connected) {
		device = IIOContext::find_device(ctx, devName.toLatin1().data())
				 .expect(QString("Expected valid device %1").arg(devName));
		nb_channels = IIODevice::get_channels_count(device);

		channelList.clear();

		for(int j = 0; j < nb_channels; j++) {
			/*get channel*/
			ch = IIODevice::get_channel(device, j).expect(QString("Expected channel with id %1").arg(j));

			if(IIOChannel::is_output(ch)) {
				type = "output ";
			} else {
				type = "input ";
			}

			type.append(QString(IIOChannel::get_id(ch)));
			channelList << type;
		}

		channelList << QString("Global");
		Q_EMIT channelsChanged(channelList);
	}
}

void DebuggerController::scanChannelAttributes(QString devName, QString &channel)
{
	iio_device *device;
	iio_channel *ch = nullptr;
	const iio_attr *attr;
	const char *attrName;
	unsigned int nb_attrs;
	bool isOutput;

	if(connected) { // check if M2K is connected
		isOutput = channel.contains("output", Qt::CaseInsensitive);

		if(isOutput) {
			channel.remove("output ", Qt::CaseInsensitive);
		} else {
			channel.remove("input ", Qt::CaseInsensitive);
		}

		device = IIOContext::find_device(ctx, devName.toLatin1().data())
				 .expect(QString("Expected device %1").arg(devName));
		attributeList.clear();
		attributeAvailable.clear();
		filename.clear();

		const bool isGlobal = (channel == QString("Global"));
		if(isGlobal) {
			nb_attrs = IIODevice::get_attrs_count(device);
		} else {
			ch = IIODevice::find_channel(device, channel.toLatin1().data(), isOutput)
				     .expect(QString("Expected channel %1").arg(channel));
			nb_attrs = IIOChannel::get_attrs_count(ch);
		}

		for(unsigned int k = 0; k < nb_attrs; k++) {
			if(isGlobal) {
				attr = IIODevice::get_attr(device, k).expect(
					QString("Expected dev attribute %1").arg(k));
			} else {
				attr = IIOChannel::get_attr(ch, k).expect(
					QString("Expected channel attribute %1").arg(k));
			}
			attrName = IIOAttribute::get_name(attr);

			if(QString(attrName).endsWith("_available", Qt::CaseInsensitive)) {
				QString tmp_attr = QString(attrName).remove("_available");
				attributeAvailable.append(tmp_attr);
			} else {
				if(isGlobal) {
					filename << attrName;
				} else {
					filename << QString(IIOAttribute::get_filename(attr));
				}
				attributeList << attrName;
			}
		}
	}
}

QStringList DebuggerController::getAvailableValues(const QString &devName, QString &channel, QString &attribute) const
{
	iio_device *device;
	iio_channel *ch;
	char value[maxAttrSize] = "";
	QStringList values;
	bool isOutput;

	attribute.append("_available");

	if(channel.contains("Global", Qt::CaseInsensitive)) {
		channel.clear();
	}

	if(connected) {

		device = IIOContext::find_device(ctx, devName.toLatin1().data())
				 .expect(QString("Expected device %1").arg(devName));

		if(channel.isNull()) {
			IIOResult<const iio_attr *> res = IIODevice::find_attr(device, attribute.toLatin1().data());
			if(!res.ok()) {
				qDebug() << "Error: " << res.error();
				return values;
			}
			const iio_attr *attr = res.data();
			IIOAttribute::read_raw(attr, value, maxAttrSize);
		} else {
			isOutput = channel.contains("output", Qt::CaseInsensitive);

			if(isOutput) {
				channel.remove("output ", Qt::CaseInsensitive);
			} else {
				channel.remove("input ", Qt::CaseInsensitive);
			}

			ch = IIODevice::find_channel(device, channel.toLatin1().data(), isOutput)
				     .expect(QString("Expected channel %1").arg(channel));
			const iio_attr *res = IIOChannel::find_attr(ch, attribute.toLatin1().data())
						      .expect(QString("Expected channel attribute %1").arg(attribute));
			IIOAttribute::read_raw(res, value, maxAttrSize);
		}

		values = QString(value).split(' ');
		return values;
	} else {
		values.clear();
		return values;
	}
}

QString DebuggerController::readAttribute(const QString &devName, QString &channel, const QString &attribute)
{
	struct iio_device *device;
	struct iio_channel *ch;
	char value[maxAttrSize] = "";
	bool isOutput;

	if(connected) {
		device = IIOContext::find_device(ctx, devName.toLatin1().data())
				 .expect(QString("Expected device %1").arg(devName));

		if(channel.isNull()) {
			const iio_attr *attr = IIODevice::find_attr(device, attribute.toLatin1().data())
						       .expect(QString("Expected attribute %1").arg(attribute));
			IIOAttribute::read_raw(attr, value, maxAttrSize);
		} else {
			isOutput = channel.contains("output", Qt::CaseInsensitive);

			if(isOutput) {
				channel.remove("output ", Qt::CaseInsensitive);
			} else {
				channel.remove("input ", Qt::CaseInsensitive);
			}

			ch = IIODevice::find_channel(device, channel.toLatin1().data(), isOutput)
				     .expect(QString("Expected channel %1").arg(channel));
			const iio_attr *attr = IIOChannel::find_attr(ch, attribute.toLatin1().data())
						       .expect(QString("Expected channel attribute %1").arg(attribute));
			IIOAttribute::read_raw(attr, value, maxAttrSize);
		}
	}

	return QString(value);
}

QString DebuggerController::writeAttribute(const QString &devName, QString &channel, const QString &attribute,
					   const QString &value)
{
	struct iio_device *device;
	struct iio_channel *ch;
	bool isOutput;
	char feedbackValue[maxAttrSize] = "";

	if(connected) {
		device = IIOContext::find_device(ctx, devName.toLatin1().data())
				 .expect(QString("Expected device %1").arg(devName));

		if(channel.isNull()) {
			const iio_attr *attr = IIODevice::find_attr(device, attribute.toLatin1().data())
						       .expect(QString("Expected attribute %1").arg(attribute));
			IIOAttribute::write_raw(attr, value.toLatin1().data(), value.size());
			IIOAttribute::read_raw(attr, feedbackValue, maxAttrSize);

		} else {
			isOutput = channel.contains("output", Qt::CaseInsensitive);

			if(isOutput) {
				channel.remove("output ", Qt::CaseInsensitive);
			} else {
				channel.remove("input ", Qt::CaseInsensitive);
			}

			ch = IIODevice::find_channel(device, channel.toLatin1().data(), isOutput)
				     .expect(QString("Expected channel %1").arg(channel));
			const iio_attr *attr = IIOChannel::find_attr(ch, attribute.toLatin1().data())
						       .expect(QString("Expected channel attribute %1").arg(attribute));
			IIOAttribute::write_raw(attr, value.toLatin1().data(), value.size());
			IIOAttribute::read_raw(attr, feedbackValue, maxAttrSize);
		}
	}
	return QString(feedbackValue);
}

struct iio_context *DebuggerController::getIioContext(void) { return ctx; }
