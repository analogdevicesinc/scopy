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
#include <pluginbase/preferences.h>

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

	// add all devices from context, dfs — structure only, no IIOWidget creation
	uint ctx_devices_count = iio_context_get_devices_count(m_ctx);
	for(m_currentDeviceIndex = 0; m_currentDeviceIndex < ctx_devices_count; ++m_currentDeviceIndex) {
		setupCurrentDevice();

		// add all channels to current device (structure only)
		uint device_channels_count = iio_device_get_channels_count(m_currentDevice);
		for(m_currentChannelIndex = 0; m_currentChannelIndex < device_channels_count; ++m_currentChannelIndex) {
			setupCurrentChannel();
			generateChannelAttributes();
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
	m_rootItem->setContext(m_ctx);
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
		attrItem->setContext(m_ctx);
		attrItem->setEditable(false);
		m_rootItem->appendRow(attrItem);
	}
}

void IIOModel::setupCurrentDevice()
{
	m_currentDevice = iio_context_get_device(m_ctx, static_cast<uint>(m_currentDeviceIndex));
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
	// Create item with empty widget list — widgets are built lazily in populateChildren()
	IIOStandardItem::Type devType = is_trigger ? IIOStandardItem::Trigger : IIOStandardItem::Device;
	m_currentDeviceItem = createIIOStandardItem({}, m_currentDeviceName, currentDeviceId,
						    m_rootString + SEPARATOR + m_currentDeviceName, devType);
	m_currentDeviceItem->setDevice(m_currentDevice);
	m_currentDeviceItem->setEditable(false);
	m_entries.insert(m_currentDeviceName);
}

void IIOModel::setupCurrentChannel()
{
	m_currentChannel = iio_device_get_channel(m_currentDevice, static_cast<uint>(m_currentChannelIndex));
	m_currentChannelName = iio_channel_get_id(m_currentChannel);
	QString currentChannelId = iio_channel_get_name(m_currentChannel);

	// Create item with empty widget list — widgets built lazily in populateChildren()
	m_currentChannelItem =
		createIIOStandardItem({}, currentChannelId, m_currentChannelName,
				      m_rootString + SEPARATOR + m_currentDeviceName + SEPARATOR + m_currentChannelName,
				      IIOStandardItem::Channel);
	m_currentChannelItem->setChannel(m_currentChannel);

	if(m_currentChannelItem->isScanElement()) {
		m_currentDeviceItem->setBufferCapable(true);
	}

	m_currentChannelItem->setEditable(false);
	m_entries.insert(m_currentChannelName);
}

void IIOModel::generateDeviceAttributes()
{
	uint attrCount = iio_device_get_attrs_count(m_currentDevice);
	for(uint j = 0; j < attrCount; ++j) {
		const char *attrName = iio_device_get_attr(m_currentDevice, j);
		if(!attrName) {
			continue;
		}
		m_entries.insert(attrName);
		auto *attrItem = createIIOStandardItem(
			{}, attrName, "", m_rootString + SEPARATOR + m_currentDeviceName + SEPARATOR + attrName,
			IIOStandardItem::DeviceAttribute);
		attrItem->setDevice(m_currentDevice);
		attrItem->setEditable(false);
		m_currentDeviceItem->appendRow(attrItem);
	}

	if(Preferences::get("debugger_v2_include_debugfs").toBool()) {
		uint dbgAttrCount = iio_device_get_debug_attrs_count(m_currentDevice);
		for(uint j = 0; j < dbgAttrCount; ++j) {
			const char *attrName = iio_device_get_debug_attr(m_currentDevice, j);
			if(!attrName) {
				continue;
			}
			m_entries.insert(attrName);
			auto *attrItem = createIIOStandardItem(
				{}, attrName, "", m_rootString + SEPARATOR + m_currentDeviceName + SEPARATOR + attrName,
				IIOStandardItem::DeviceAttribute);
			attrItem->setDevice(m_currentDevice);
			attrItem->setEditable(false);
			m_currentDeviceItem->appendRow(attrItem);
		}
	}
}

void IIOModel::generateChannelAttributes()
{
	uint attrCount = iio_channel_get_attrs_count(m_currentChannel);
	for(uint i = 0; i < attrCount; ++i) {
		const char *attrName = iio_channel_get_attr(m_currentChannel, i);
		if(!attrName) {
			continue;
		}
		m_entries.insert(attrName);
		auto *attrItem = createIIOStandardItem({}, attrName, "",
						       m_rootString + SEPARATOR + m_currentDeviceName + SEPARATOR +
							       m_currentChannelName + SEPARATOR + attrName,
						       IIOStandardItem::ChannelAttribute);
		attrItem->setChannel(m_currentChannel);
		attrItem->setEditable(false);
		m_currentChannelItem->appendRow(attrItem);
	}
}

void IIOModel::populateChildren(IIOStandardItem *item)
{
	if(!item || item->childrenLoaded()) {
		return;
	}
	item->setChildrenLoaded(true);

	IIOStandardItem::Type type = item->type();
	if(type == IIOStandardItem::Device || type == IIOStandardItem::Trigger) {
		populateDeviceChildren(item);
	} else if(type == IIOStandardItem::Channel) {
		populateChannelChildren(item);
	} else if(type == IIOStandardItem::DeviceAttribute || type == IIOStandardItem::ChannelAttribute) {
		auto *parentItem = dynamic_cast<IIOStandardItem *>(item->QStandardItem::parent());
		if(parentItem) {
			populateChildren(parentItem);
		}
	}
}

void IIOModel::populateDeviceChildren(IIOStandardItem *item)
{
	struct iio_device *dev = item->device();
	if(!dev) {
		return;
	}

	QList<IIOWidget *> devWidgets =
		IIOWidgetBuilder(m_parent).device(dev).includeAvailableAttributes(true).buildAll();
	item->setIIOWidgets(devWidgets);

	// Assign widgets to existing attribute children (created eagerly in generateDeviceAttributes)
	int widgetIdx = 0;
	for(int i = 0; i < item->rowCount(); ++i) {
		auto *child = dynamic_cast<IIOStandardItem *>(item->child(i));
		if(!child || child->type() == IIOStandardItem::Channel) {
			continue;
		}
		if(widgetIdx < devWidgets.size()) {
			child->setIIOWidgets({devWidgets[widgetIdx]});
			++widgetIdx;
		}
	}
}

void IIOModel::populateChannelChildren(IIOStandardItem *item)
{
	struct iio_channel *ch = item->channel();
	if(!ch) {
		return;
	}

	QList<IIOWidget *> chnlWidgets =
		IIOWidgetBuilder(m_parent).channel(ch).includeAvailableAttributes(true).buildAll();
	item->setIIOWidgets(chnlWidgets);

	// Assign widgets to existing attribute children (created eagerly in generateChannelAttributes)
	for(int i = 0; i < item->rowCount() && i < chnlWidgets.size(); ++i) {
		auto *child = dynamic_cast<IIOStandardItem *>(item->child(i));
		if(child) {
			child->setIIOWidgets({chnlWidgets[i]});
		}
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
