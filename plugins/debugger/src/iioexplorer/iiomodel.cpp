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

#include "iiomodel.h"
#include <QLoggingCategory>

#define BUFFER_SIZE 256
#define SEPARATOR "/"

using namespace scopy::debugger;

IIOModel::IIOModel(struct iio_context *context, QString uri, QWidget *parent)
	: QObject(parent)
	, m_ctx(context)
	, m_uri(uri)
	, m_parent(parent)
{
	m_model = new QStandardItemModel(this);
	iioTreeSetup();
}

QStandardItemModel *IIOModel::getModel() { return m_model; }

QSet<QString> IIOModel::getEntries() { return m_entries; }

void IIOModel::iioTreeSetup()
{
	m_rootString = m_uri;
	setupCtx();

	// add all devices from context, dfs
	uint ctx_devices_count = iio_context_get_devices_count(m_ctx);
	for(m_currentDeviceIndex = 0; m_currentDeviceIndex < ctx_devices_count; ++m_currentDeviceIndex) {
		setupCurrentDevice();

		// add all channels to current device
		uint device_channels_count = iio_device_get_channels_count(m_currentDevice);
		for(m_currentChannelIndex = 0; m_currentChannelIndex < device_channels_count; ++m_currentChannelIndex) {
			setupCurrentChannel();
			generateChannelAttributes();

			// add channel to device
			m_currentDeviceItem->appendRow(m_currentChannelItem);
		}
		generateDeviceAttributes();

		// add device to ctx
		m_rootItem->appendRow(m_currentDeviceItem);
	}
	generateCtxAttributes();

	m_model->appendRow(m_rootItem);
}

void IIOModel::setupCtx()
{
	m_ctxList = IIOWidgetBuilder(m_parent).context(m_ctx).buildAll();
	m_rootItem = createIIOStandardItem(m_ctxList, m_rootString, "", m_rootString, IIOStandardItem::Context);
	// m_rootItem = new IIOStandardItem(m_ctxList, m_rootString, m_rootString, IIOStandardItem::Context);
	m_rootItem->setEditable(false);
}

void IIOModel::generateCtxAttributes()
{
	// add attrs from context
	for(IIOWidget *ctxWidget : qAsConst(m_ctxList)) {
		m_entries.insert(ctxWidget->getRecipe().data);
		auto *attrItem = createIIOStandardItem({ctxWidget}, ctxWidget->getRecipe().data, "",
						       m_rootString + SEPARATOR + ctxWidget->getRecipe().data,
						       IIOStandardItem::ContextAttribute);
		attrItem->setEditable(false);
		m_rootItem->appendRow(attrItem);
	}
}

void IIOModel::setupCurrentDevice()
{
	m_currentDevice = iio_context_get_device(m_ctx, static_cast<uint>(m_currentDeviceIndex));
	m_devList = IIOWidgetBuilder(m_parent).device(m_currentDevice).includeAvailableAttributes(true).buildAll();
	m_currentDeviceName = iio_device_get_name(m_currentDevice);
	QString lbl = iio_device_get_label(m_currentDevice);
	if(!lbl.isEmpty()) {
		m_currentDeviceName = lbl;
	}
	QString currentDeviceId = iio_device_get_id(m_currentDevice);
	if(m_currentDeviceName.isEmpty()) {
		m_currentDeviceName = currentDeviceId;
	}
	bool is_trigger = iio_device_is_trigger(m_currentDevice);
	if(is_trigger) {
		m_currentDeviceItem =
			createIIOStandardItem({}, m_currentDeviceName, currentDeviceId,
					      m_rootString + SEPARATOR + m_currentDeviceName, IIOStandardItem::Trigger);
	} else {
		m_currentDeviceItem =
			createIIOStandardItem(m_devList, m_currentDeviceName, currentDeviceId,
					      m_rootString + SEPARATOR + m_currentDeviceName, IIOStandardItem::Device);
	}
	m_currentDeviceItem->setDevice(m_currentDevice);
	m_currentDeviceItem->setEditable(false);
	m_entries.insert(m_currentDeviceName);
}

void IIOModel::generateDeviceAttributes()
{
	// Debug attr idx
	uint debug_attr_idx = 0;

	// add all attrs to current device
	for(int j = 0; j < m_devList.size(); ++j) {
		QString device_attr = iio_device_get_attr(m_currentDevice, static_cast<uint>(j));
		if(device_attr.isEmpty()) {
			// Probably a debug attribute
			device_attr = iio_device_get_debug_attr(m_currentDevice, debug_attr_idx++);
		}

		if(device_attr.isEmpty()) {
			// Skip empty attributes
			continue;
		}

		m_entries.insert(device_attr);
		auto *attrItem =
			createIIOStandardItem({m_devList[j]}, m_devList[j]->getRecipe().data, "",
					      m_rootString + SEPARATOR + m_currentDeviceName + SEPARATOR + device_attr,
					      IIOStandardItem::DeviceAttribute);
		attrItem->setDevice(m_currentDevice);
		attrItem->setEditable(false);
		m_currentDeviceItem->appendRow(attrItem);
	}
}

void IIOModel::setupCurrentChannel()
{
	m_currentChannel = iio_device_get_channel(m_currentDevice, static_cast<uint>(m_currentChannelIndex));
	m_chnlList = IIOWidgetBuilder(m_parent).channel(m_currentChannel).includeAvailableAttributes(true).buildAll();
	m_currentChannelName = iio_channel_get_id(m_currentChannel);
	QString currentChannelId = iio_channel_get_name(m_currentChannel);

	m_currentChannelItem =
		createIIOStandardItem(m_chnlList, currentChannelId, m_currentChannelName,
				      m_rootString + SEPARATOR + m_currentDeviceName + SEPARATOR + m_currentChannelName,
				      IIOStandardItem::Channel);
	m_currentChannelItem->setChannel(m_currentChannel);

	if(m_currentChannelItem->isScanElement()) {
		m_currentDeviceItem->setBufferCapable(true);
	}

	m_currentChannelItem->setEditable(false);
	m_entries.insert(m_currentChannelName);
}

void IIOModel::generateChannelAttributes()
{
	// add all attrs from channel
	for(int i = 0; i < m_chnlList.size(); ++i) {
		QString attr_name = iio_channel_get_attr(m_currentChannel, i);

		m_entries.insert(attr_name);
		QString attrName = m_chnlList[i]->getRecipe().data;
		auto *attr_item = createIIOStandardItem({m_chnlList[i]}, attrName, "",
							m_rootString + SEPARATOR + m_currentDeviceName + SEPARATOR +
								m_currentChannelName + SEPARATOR + attrName,
							IIOStandardItem::ChannelAttribute);
		attr_item->setChannel(m_currentChannel);
		attr_item->setEditable(false);
		m_currentChannelItem->appendRow(attr_item);
	}
}

IIOStandardItem *IIOModel::createIIOStandardItem(QList<IIOWidget *> widgets, QString name, QString id, QString path,
						 IIOStandardItem::Type type)
{
	IIOStandardItem *item;
	if(id.isEmpty()) {
		item = new IIOStandardItem(widgets, name, path, type);
	} else {
		item = new IIOStandardItem(widgets, name, id, path, type);
	}

	/* Hide all widgets */
	for(IIOWidget *widget : qAsConst(widgets)) {
		widget->hide();
	}

	connect(item, &IIOStandardItem::emitLog, this, &IIOModel::emitLog);

	return item;
}

#include "moc_iiomodel.cpp"
