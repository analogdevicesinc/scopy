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

#include "clidetailsview.h"
#include <QVBoxLayout>
#include <QtConcurrent/QtConcurrentRun>
#include <pluginbase/preferences.h>

#define ATTR_BUFFER_SIZE 16384

using namespace scopy::debugger;

CliDetailsView::CliDetailsView(QWidget *parent)
	: QWidget(parent)
	, m_textBrowser(new QTextBrowser(this))
	, m_currentItem(nullptr)
	, m_includeDebugAttrs(Preferences::get("debugger_v2_include_debugfs").toBool())
{
	setupUi();
}

void CliDetailsView::setIIOStandardItem(IIOStandardItem *item)
{
	m_currentItem = item;
	m_textBrowser->setText("Loading...");

	QFutureWatcher<QString> *watcher = new QFutureWatcher<QString>(this);
	connect(watcher, &QFutureWatcher<QString>::finished, this, [this, watcher, item]() {
		if(m_currentItem == item) {
			m_textBrowser->setText(watcher->result());
		}
		watcher->deleteLater();
	});

	QFuture<QString> future = QtConcurrent::run([this, item]() { return buildText(item); });
	watcher->setFuture(future);
}

void CliDetailsView::refreshView() { setIIOStandardItem(m_currentItem); }

QString CliDetailsView::buildText(IIOStandardItem *item)
{
	QString text;
	int globalLevel = 0;

	switch(item->type()) {
	case IIOStandardItem::Context:
		globalLevel = 0;
		appendContextInfo(item->context(), text, globalLevel);
		break;
	case IIOStandardItem::ContextAttribute: {
		struct iio_context *ctx = item->context();
		if(ctx) {
			uint count = iio_context_get_attrs_count(ctx);
			for(uint i = 0; i < count; ++i) {
				const char *name;
				const char *value;
				if(iio_context_get_attr(ctx, i, &name, &value) == 0 && item->name() == QString(name)) {
					text.append(QString("%1: %2\n").arg(name).arg(value));
					break;
				}
			}
		}
		break;
	}
	case IIOStandardItem::Device:
		/* fallthrough */
	case IIOStandardItem::Trigger:
		globalLevel = 1;
		appendDeviceInfo(item->device(), text, globalLevel);
		break;
	case IIOStandardItem::DeviceAttribute: {
		struct iio_device *dev = item->device();
		if(dev) {
			QString val = readDeviceAttr(dev, item->name().toStdString().c_str(), false);
			if(val.isEmpty()) {
				val = readDeviceAttr(dev, item->name().toStdString().c_str(), true);
			}
			text.append(QString("attr 0: %1 value: %2\n").arg(item->name()).arg(val));
		}
		break;
	}
	case IIOStandardItem::Channel:
		globalLevel = 3;
		appendChannelInfo(item->channel(), text, globalLevel);
		break;
	case IIOStandardItem::ChannelAttribute: {
		struct iio_channel *ch = item->channel();
		if(ch) {
			QString val = readChannelAttr(ch, item->name().toStdString().c_str());
			text.append(QString("attr 0: %1 value: %2\n").arg(item->name()).arg(val));
		}
		break;
	}
	default:
		break;
	}

	return text;
}

void CliDetailsView::setupUi()
{
	setContentsMargins(0, 0, 0, 0);
	setLayout(new QVBoxLayout(this));
	layout()->addWidget(m_textBrowser);
	layout()->setContentsMargins(0, 0, 0, 0);
	m_textBrowser->setTabStopDistance(30);
	QFont mono("Monospace");
	mono.setStyleHint(QFont::Monospace);
	m_textBrowser->setFont(mono);
}

void CliDetailsView::appendContextInfo(struct iio_context *ctx, QString &text, int globalLevel)
{
	if(!ctx) {
		return;
	}

	uint ctxAttrCount = iio_context_get_attrs_count(ctx);
	text.append(QString("IIO context has %1 attributes:\n").arg(ctxAttrCount));
	for(uint i = 0; i < ctxAttrCount; ++i) {
		const char *name;
		const char *value;
		if(iio_context_get_attr(ctx, i, &name, &value) == 0) {
			text.append(tabs(1, globalLevel) + QString("%1: %2\n").arg(name).arg(value));
		}
	}

	uint devCount = iio_context_get_devices_count(ctx);
	text.append(QString("IIO context has %1 devices:\n").arg(devCount));
	for(uint i = 0; i < devCount; ++i) {
		struct iio_device *dev = iio_context_get_device(ctx, i);
		appendDeviceInfo(dev, text, globalLevel);
	}
}

void CliDetailsView::appendDeviceInfo(struct iio_device *dev, QString &text, int globalLevel)
{
	if(!dev) {
		return;
	}

	QString devName = iio_device_get_name(dev);
	QString devLabel = iio_device_get_label(dev);
	QString devId = iio_device_get_id(dev);
	if(!devLabel.isEmpty()) {
		devName = devLabel;
	}
	if(devName.isEmpty()) {
		devName = devId;
	}
	QString displayName = devId + ": " + devName;

	bool bufferCapable = false;
	uint chCount = iio_device_get_channels_count(dev);
	for(uint i = 0; i < chCount; ++i) {
		if(iio_channel_is_scan_element(iio_device_get_channel(dev, i))) {
			bufferCapable = true;
			break;
		}
	}

	text.append(tabs(1, globalLevel) + displayName + (bufferCapable ? " (buffer capable)" : "") + "\n");

	// Channels
	text.append(tabs(2, globalLevel) + QString("%1 channels found:\n").arg(chCount));
	for(uint i = 0; i < chCount; ++i) {
		struct iio_channel *ch = iio_device_get_channel(dev, i);
		appendChannelInfo(ch, text, globalLevel);
	}

	// Device attributes
	uint devAttrCount = iio_device_get_attrs_count(dev);
	if(devAttrCount > 0) {
		text.append(tabs(2, globalLevel) + QString("%1 device-specific attributes found:\n").arg(devAttrCount));
		for(uint i = 0; i < devAttrCount; ++i) {
			const char *attr = iio_device_get_attr(dev, i);
			if(!attr) {
				continue;
			}
			QString val = readDeviceAttr(dev, attr, false);
			text.append(tabs(4, globalLevel) +
				    QString("attr %1: %2 value: %3\n").arg(i).arg(attr).arg(val));
		}
	}

	// Debug attributes
	if(m_includeDebugAttrs) {
		uint dbgAttrCount = iio_device_get_debug_attrs_count(dev);
		if(dbgAttrCount > 0) {
			text.append(tabs(2, globalLevel) + QString("%1 debug attributes found:\n").arg(dbgAttrCount));
			for(uint i = 0; i < dbgAttrCount; ++i) {
				const char *attr = iio_device_get_debug_attr(dev, i);
				if(!attr) {
					continue;
				}
				QString val = readDeviceAttr(dev, attr, true);
				text.append(tabs(4, globalLevel) +
					    QString("attr %1: %2 value: %3\n").arg(i).arg(attr).arg(val));
			}
		}
	}

	// Trigger status
	text.append(tabs(2, globalLevel) + triggerStatusString(dev) + "\n");
}

void CliDetailsView::appendChannelInfo(struct iio_channel *ch, QString &text, int globalLevel)
{
	if(!ch) {
		return;
	}

	QString chId = iio_channel_get_id(ch);
	const char *chNameCStr = iio_channel_get_name(ch);
	QString chName = chNameCStr ? chNameCStr : "";
	QString display = !chName.isEmpty() ? (chId + ": " + chName) : chId;

	bool isOutput = iio_channel_is_output(ch);
	bool isScanElement = iio_channel_is_scan_element(ch);

	text.append(tabs(3, globalLevel) + display + ": (" + (isOutput ? "output" : "input"));
	if(isScanElement) {
		text.append(
			QString(", index: %1, format: %2").arg(iio_channel_get_index(ch)).arg(channelFormatString(ch)));
	}
	text.append(")\n");

	// Channel attributes
	uint chAttrCount = iio_channel_get_attrs_count(ch);
	if(chAttrCount > 0) {
		text.append(tabs(3, globalLevel) + QString("%1 channel-specific attributes found:\n").arg(chAttrCount));
		for(uint i = 0; i < chAttrCount; ++i) {
			const char *attr = iio_channel_get_attr(ch, i);
			if(!attr) {
				continue;
			}
			QString val = readChannelAttr(ch, attr);
			text.append(tabs(4, globalLevel) +
				    QString("attr %1: %2 value: %3\n").arg(i).arg(attr).arg(val));
		}
	}
}

QString CliDetailsView::readDeviceAttr(struct iio_device *dev, const char *attr, bool isDebug)
{
	char buf[ATTR_BUFFER_SIZE] = {0};
	ssize_t ret;
	if(isDebug) {
		ret = iio_device_debug_attr_read(dev, attr, buf, sizeof(buf));
	} else {
		ret = iio_device_attr_read(dev, attr, buf, sizeof(buf));
	}
	return (ret >= 0) ? QString(buf) : QString("ERROR (%1)").arg(ret);
}

QString CliDetailsView::readChannelAttr(struct iio_channel *ch, const char *attr)
{
	char buf[ATTR_BUFFER_SIZE] = {0};
	ssize_t ret = iio_channel_attr_read(ch, attr, buf, sizeof(buf));
	return (ret >= 0) ? QString(buf) : QString("ERROR (%1)").arg(ret);
}

QString CliDetailsView::channelFormatString(struct iio_channel *ch)
{
	const struct iio_data_format *format = iio_channel_get_data_format(ch);
	char sign = format->is_signed ? 's' : 'u';
	char repeat[12];
	repeat[0] = '\0';

	if(format->is_fully_defined) {
		sign += 'A' - 'a';
	}

	if(format->repeat > 1) {
		snprintf(repeat, sizeof(repeat), "X%u", format->repeat);
	}

	return QString("%1e:%2%3/%4%5>>%6")
		.arg(format->is_be ? 'b' : 'l')
		.arg(sign)
		.arg(format->bits)
		.arg(format->length)
		.arg(repeat)
		.arg(format->shift);
}

QString CliDetailsView::triggerStatusString(struct iio_device *dev)
{
	const struct iio_device *trig;
	int ret = iio_device_get_trigger(dev, &trig);
	if(ret == 0) {
		if(trig) {
			QString trigName = iio_device_get_name(trig);
			QString trigId = iio_device_get_id(trig);
			return QString("Current trigger: %1(%2)").arg(trigId).arg(trigName);
		}
		return "No trigger on this device";
	} else if(ret == -ENODEV) {
		return "No trigger assigned on this device";
	} else if(ret == -ENOENT) {
		return "No trigger on this device";
	}
	return "Unable to get trigger";
}

QString CliDetailsView::tabs(int level, int globalLevel)
{
	int effective = level - globalLevel;
	QString result;
	for(int i = 0; i < effective; ++i) {
		result.append("\t");
	}
	return result;
}

#include "moc_clidetailsview.cpp"
