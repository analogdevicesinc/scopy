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

	// add all devices from context — structure only, no IIOWidget creation
	uint ctx_devices_count = iio_context_get_devices_count(m_ctx);
	for(m_currentDeviceIndex = 0; m_currentDeviceIndex < ctx_devices_count; ++m_currentDeviceIndex) {
		setupCurrentDevice();

		// add all channels to current device (structure only)
		uint device_channels_count = iio_device_get_channels_count(m_currentDevice);
		for(m_currentChannelIndex = 0; m_currentChannelIndex < device_channels_count; ++m_currentChannelIndex) {
			setupCurrentChannel();
			m_currentDeviceItem->appendRow(m_currentChannelItem);
		}

		// If the device has no channel children but has attributes, add a placeholder
		// so the expand arrow is visible in the tree view.
		if(m_currentDeviceItem->rowCount() == 0) {
			uint devAttrCount = iio_device_get_attrs_count(m_currentDevice);
			uint dbgAttrCount = iio_device_get_debug_attrs_count(m_currentDevice);
			if(devAttrCount + dbgAttrCount > 0) {
				m_currentDeviceItem->appendRow(createPlaceholderItem());
			}
		}

		// add device to ctx
		m_rootItem->appendRow(m_currentDeviceItem);
	}
	generateCtxAttributes();

	m_model->appendRow(m_rootItem);

	// Pre-populate entries so the SearchBar autocomplete works immediately
	buildEntries();
}

void IIOModel::setupCtx()
{
	m_ctxList = IIOWidgetBuilder(nullptr).context(m_ctx).buildAll();
	m_rootItem = createIIOStandardItem(m_ctxList, m_rootString, "", m_rootString, IIOStandardItem::Context);
	m_rootItem->setContext(m_ctx);
	m_rootItem->setEditable(false);
}

void IIOModel::generateCtxAttributes()
{
	// Build a separate set of widgets for the attribute child items so they don't
	// share the same IIOWidget instances with the context root item.
	QList<IIOWidget *> ctxWidgetsForChildren = IIOWidgetBuilder(nullptr).context(m_ctx).buildAll();

	for(int i = 0; i < ctxWidgetsForChildren.size(); ++i) {
		IIOWidget *ctxWidget = ctxWidgetsForChildren[i];
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
	IIOStandardItem::Type devType = is_trigger ? IIOStandardItem::Trigger : IIOStandardItem::Device;
	// Create item with empty widget list — widgets are built lazily in populateChildren()
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

	// Create item with empty widget list — widgets are built lazily in populateChildren()
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

	// Add placeholder child so the expand arrow is visible
	if(iio_channel_get_attrs_count(m_currentChannel) > 0) {
		m_currentChannelItem->appendRow(createPlaceholderItem());
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
	}
}

void IIOModel::populateDeviceChildren(IIOStandardItem *item)
{
	struct iio_device *dev = item->device();
	if(!dev) {
		return;
	}

	// Build one set for the parent item's GUI view and a separate set for the attribute child items,
	// so that displaying a child attribute does not remove its widget from the parent's view.
	QList<IIOWidget *> devWidgetsForParent =
		IIOWidgetBuilder(nullptr).device(dev).includeAvailableAttributes(true).buildAll();
	QList<IIOWidget *> devWidgetsForChildren =
		IIOWidgetBuilder(nullptr).device(dev).includeAvailableAttributes(true).buildAll();

	item->setIIOWidgets(devWidgetsForParent);

	// Remove placeholder if present (it's a plain QStandardItem, not IIOStandardItem)
	if(item->rowCount() > 0 && !dynamic_cast<IIOStandardItem *>(item->child(0))) {
		item->removeRow(0);
	}

	uint debug_attr_idx = 0;
	for(int j = 0; j < devWidgetsForChildren.size(); ++j) {
		QString device_attr = iio_device_get_attr(dev, static_cast<uint>(j));
		if(device_attr.isEmpty()) {
			device_attr = iio_device_get_debug_attr(dev, debug_attr_idx++);
		}
		if(device_attr.isEmpty()) {
			continue;
		}

		m_entries.insert(device_attr);
		auto *attrItem = createIIOStandardItem(
			{devWidgetsForChildren[j]}, devWidgetsForChildren[j]->getRecipe().data, "",
			item->path() + SEPARATOR + device_attr, IIOStandardItem::DeviceAttribute);
		attrItem->setDevice(dev);
		attrItem->setEditable(false);
		item->appendRow(attrItem);
	}
}

void IIOModel::populateChannelChildren(IIOStandardItem *item)
{
	struct iio_channel *ch = item->channel();
	if(!ch) {
		return;
	}

	// Remove placeholder if present (it's a plain QStandardItem, not IIOStandardItem)
	if(item->rowCount() > 0 && !dynamic_cast<IIOStandardItem *>(item->child(0))) {
		item->removeRow(0);
	}

	// Build one set for the parent item's GUI view and a separate set for the attribute child items,
	// so that displaying a child attribute does not remove its widget from the parent's view.
	QList<IIOWidget *> chnlWidgetsForParent =
		IIOWidgetBuilder(nullptr).channel(ch).includeAvailableAttributes(true).buildAll();
	QList<IIOWidget *> chnlWidgetsForChildren =
		IIOWidgetBuilder(nullptr).channel(ch).includeAvailableAttributes(true).buildAll();

	item->setIIOWidgets(chnlWidgetsForParent);

	for(int i = 0; i < chnlWidgetsForChildren.size(); ++i) {
		QString attr_name = iio_channel_get_attr(ch, i);
		m_entries.insert(attr_name);
		QString attrName = chnlWidgetsForChildren[i]->getRecipe().data;
		auto *attr_item =
			createIIOStandardItem({chnlWidgetsForChildren[i]}, attrName, "",
					      item->path() + SEPARATOR + attrName, IIOStandardItem::ChannelAttribute);
		attr_item->setChannel(ch);
		attr_item->setEditable(false);
		item->appendRow(attr_item);
	}
}

void IIOModel::buildEntries()
{
	uint devCount = iio_context_get_devices_count(m_ctx);
	for(uint i = 0; i < devCount; ++i) {
		struct iio_device *dev = iio_context_get_device(m_ctx, i);

		uint devAttrCount = iio_device_get_attrs_count(dev);
		for(uint j = 0; j < devAttrCount; ++j) {
			const char *attr = iio_device_get_attr(dev, j);
			if(attr) {
				m_entries.insert(attr);
			}
		}

		uint dbgAttrCount = iio_device_get_debug_attrs_count(dev);
		for(uint j = 0; j < dbgAttrCount; ++j) {
			const char *attr = iio_device_get_debug_attr(dev, j);
			if(attr) {
				m_entries.insert(attr);
			}
		}

		uint chCount = iio_device_get_channels_count(dev);
		for(uint k = 0; k < chCount; ++k) {
			struct iio_channel *ch = iio_device_get_channel(dev, k);
			uint chAttrCount = iio_channel_get_attrs_count(ch);
			for(uint l = 0; l < chAttrCount; ++l) {
				const char *attr = iio_channel_get_attr(ch, l);
				if(attr) {
					m_entries.insert(attr);
				}
			}
		}
	}
}

QStandardItem *IIOModel::createPlaceholderItem()
{
	auto *placeholder = new QStandardItem();
	placeholder->setEnabled(false);
	placeholder->setEditable(false);
	return placeholder;
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

	connect(item, &IIOStandardItem::emitLog, this, &IIOModel::emitLog);

	return item;
}

#include "moc_iiomodel.cpp"
