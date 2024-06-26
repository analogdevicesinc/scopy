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
 */

#include "iiowidgetbuilder.h"
#include "guistrategy/editableguistrategy.h"
#include "guistrategy/switchguistrategy.h"
#include "datastrategy/channelattrdatastrategy.h"
#include "datastrategy/triggerdatastrategy.h"
#include "datastrategy/deviceattrdatastrategy.h"
#include "datastrategy/contextattrdatastrategy.h"
#include "datastrategy/cmdqchannelattrdatastrategy.h"
#include "datastrategy/cmdqdeviceattrdatastrategy.h"
#include "guistrategy/comboguistrategy.h"
#include "guistrategy/rangeguistrategy.h"
#include <iioutil/connectionprovider.h>
#include <QLoggingCategory>

#define ATTR_BUFFER_SIZE 16384
using namespace scopy;
Q_LOGGING_CATEGORY(CAT_ATTRFACTORY, "AttrFactory")

IIOWidgetBuilder::IIOWidgetBuilder(QObject *parent)
	: QObject(parent)
	, m_connection(nullptr)
	, m_isCompact(false)
	, m_context(nullptr)
	, m_device(nullptr)
	, m_channel(nullptr)
	, m_attribute("")
	, m_optionsAttribute("")
	, m_optionsValues("")
	, m_dataStrategy(DS::NoDataStrategy)
	, m_uiStrategy(UIS::NoUIStrategy)
	, m_widgetParent(nullptr)
{}

IIOWidgetBuilder::~IIOWidgetBuilder() {}

IIOWidget *IIOWidgetBuilder::buildSingle()
{
	DataStrategyInterface *ds = nullptr;
	GuiStrategyInterface *ui = nullptr;

	if(!m_context && !m_device && !m_channel) {
		qWarning(CAT_ATTRFACTORY) << "No channel/device/context set.";
		return nullptr;
	}

	if(m_attribute.isEmpty()) {
		qWarning(CAT_ATTRFACTORY) << "No attribute name set.";
		return nullptr;
	}

	m_generatedRecipe = {
		.connection = m_connection,
		.context = m_context,
		.device = m_device,
		.channel = m_channel,
		.data = m_attribute,
		.iioDataOptions = m_optionsAttribute,
		.constDataOptions = m_optionsValues,
	};

	ds = createDS();
	ui = createUIS();

	IIOWidget *widget = new IIOWidget(ui, ds, m_widgetParent);
	widget->setRecipe(m_generatedRecipe);
	return widget;
}

QList<IIOWidget *> IIOWidgetBuilder::buildAll()
{
	QList<IIOWidget *> result;
	ssize_t attrCount = 0;
	const char *attrName = nullptr;
	const char *availableAttr = nullptr;

	if(m_channel) {
		attrCount = iio_channel_get_attrs_count(m_channel);
		for(ssize_t i = 0; i < attrCount; ++i) {
			attrName = iio_channel_get_attr(m_channel, i);
			if(!attrName) {
				qWarning(CAT_ATTRFACTORY)
					<< "Could not read the channel attribute name with index" << i;
				continue;
			}

			m_attribute = attrName;
			if(QString(attrName).endsWith("_available")) {
				continue;
			}

			availableAttr = iio_channel_find_attr(m_channel,
							      (QString(attrName) + "_available").toStdString().c_str());
			if(availableAttr) {
				m_optionsAttribute = availableAttr;
			}

			result.append(buildSingle());
		}
	} else if(m_device) {
		attrCount = iio_device_get_attrs_count(m_device);
		for(ssize_t i = 0; i < attrCount; ++i) {
			attrName = iio_device_get_attr(m_device, i);
			if(!attrName) {
				qWarning(CAT_ATTRFACTORY) << "Could not read the device attribute name with index" << i;
				continue;
			}

			m_attribute = attrName;
			if(QString(attrName).endsWith("_available")) {
				continue;
			}

			availableAttr = iio_device_find_attr(m_device,
							     (QString(attrName) + "_available").toStdString().c_str());
			if(availableAttr) {
				m_optionsAttribute = availableAttr;
			}

			result.append(buildSingle());
		}
	} else if(m_context) {
		attrCount = iio_context_get_devices_count(m_context);
		for(ssize_t i = 0; i < attrCount; ++i) {
			const char *name;
			const char *value;
			int res = iio_context_get_attr(m_context, i, &name, &value);

			if(res < 0) {
				qWarning(CAT_ATTRFACTORY) << "Coutd not get the context attribute with index" << i;
				continue;
			}

			m_attribute = name;
			result.append(buildSingle());
		}
	} else {
		qWarning(CAT_ATTRFACTORY) << "Not enough information to build IIOWidgets";
		return {};
	}

	// clear the temp attributes that were used here
	m_attribute.clear();
	m_optionsAttribute.clear();

	return result;
}

void IIOWidgetBuilder::clear()
{
	m_connection = nullptr;
	m_isCompact = false;
	m_context = nullptr;
	m_device = nullptr;
	m_channel = nullptr;
	m_attribute = "";
	m_optionsAttribute = "";
	m_optionsValues = "";
	m_dataStrategy = DS::NoDataStrategy;
	m_uiStrategy = UIS::NoUIStrategy;
	m_widgetParent = nullptr;
}

IIOWidgetBuilder &IIOWidgetBuilder::connection(Connection *connection)
{
	m_connection = connection;
	return *this;
}

IIOWidgetBuilder &IIOWidgetBuilder::compactMode(bool isCompact)
{
	m_isCompact = isCompact;
	return *this;
}

IIOWidgetBuilder &IIOWidgetBuilder::context(iio_context *context)
{
	m_context = context;
	return *this;
}

IIOWidgetBuilder &IIOWidgetBuilder::device(iio_device *device)
{
	m_device = device;
	return *this;
}

IIOWidgetBuilder &IIOWidgetBuilder::channel(iio_channel *channel)
{
	m_channel = channel;
	return *this;
}

IIOWidgetBuilder &IIOWidgetBuilder::attribute(QString attribute)
{
	m_attribute = attribute;
	return *this;
}

IIOWidgetBuilder &IIOWidgetBuilder::optionsAttribute(QString optionsAttribute)
{
	m_optionsAttribute = optionsAttribute;
	return *this;
}

IIOWidgetBuilder &IIOWidgetBuilder::optionsValues(QString optionsValues)
{
	m_optionsValues = optionsValues;
	return *this;
}

IIOWidgetBuilder &IIOWidgetBuilder::dataStrategy(DS dataStrategy)
{
	m_dataStrategy = dataStrategy;
	return *this;
}

IIOWidgetBuilder &IIOWidgetBuilder::uiStrategy(UIS uiStrategy)
{
	m_uiStrategy = uiStrategy;
	return *this;
}

IIOWidgetBuilder &IIOWidgetBuilder::parent(QWidget *parent)
{
	m_widgetParent = parent;
	return *this;
}

DataStrategyInterface *IIOWidgetBuilder::createDS()
{
	DataStrategyInterface *ds = nullptr;

	// we do this so the m_dataStrategy will not be altered by the function
	DS strategy = m_dataStrategy;

	// determine the DS
	if(strategy == DS::NoDataStrategy) {
		if(m_channel) {
			strategy = DS::AttrData;
		} else if(m_device) {
			strategy = DS::DeviceAttrData;
		} else if(m_context) {
			strategy = DS::ContextAttrData;
		}
	}

	switch(strategy) {
	case DS::NoDataStrategy:
		qWarning(CAT_ATTRFACTORY) << "Could not determine the Data Strategy";
		break;
	case DS::AttrData:
		if(m_connection)
			ds = new CmdQChannelAttrDataStrategy(m_generatedRecipe, m_widgetParent);
		else
			ds = new ChannelAttrDataStrategy(m_generatedRecipe, m_widgetParent);
		break;
	case DS::DeviceAttrData:
		if(m_connection)
			ds = new CmdQDeviceAttrDataStrategy(m_generatedRecipe, m_widgetParent);
		else
			ds = new DeviceAttrDataStrategy(m_generatedRecipe, m_widgetParent);
		break;
	case DS::ContextAttrData:
		ds = new ContextAttrDataStrategy(m_generatedRecipe, m_widgetParent);
		break;
	case DS::TriggerData:
		ds = new TriggerDataStrategy(m_generatedRecipe, m_widgetParent);
		break;
	default:
		qWarning(CAT_ATTRFACTORY) << "No valid Data Strategy was provided.";
		break;
	}

	return ds;
}

GuiStrategyInterface *IIOWidgetBuilder::createUIS()
{
	// once here, it should be guaranteed that we can create a UIS
	GuiStrategyInterface *ui = nullptr;

	// we do this so the m_uiStrategy will not be altered by the function
	UIS strategy = m_uiStrategy;

	// figure out what strategy fits here
	if(strategy == UIS::NoUIStrategy) {
		if(!m_optionsAttribute.isEmpty()) {
			// read values from iio and interpret them
			char buffer[ATTR_BUFFER_SIZE] = {0};
			ssize_t res;
			if(m_channel) {
				res = iio_channel_attr_read(m_channel, m_optionsAttribute.toStdString().c_str(), buffer,
							    ATTR_BUFFER_SIZE);
			} else if(m_device) {
				res = iio_device_attr_read(m_device, m_optionsAttribute.toStdString().c_str(), buffer,
							   ATTR_BUFFER_SIZE);
			} else { // context
				// editable as context attrs are read only and cannot be changed
				strategy = UIS::EditableUi;
			}

			if(res < 0) {
				qWarning(CAT_ATTRFACTORY) << "Could not read options from" << m_optionsAttribute;
				strategy = UIS::EditableUi;
			} else {
				if(QString(buffer).startsWith('[') && strategy == UIS::NoUIStrategy) {
					strategy = UIS::RangeUi;
				} else {
					strategy = UIS::ComboUi;
				}
			}
		} else if(!m_optionsValues.isEmpty()) {
			// const values
			if(m_optionsValues.startsWith('[')) {
				strategy = UIS::RangeUi;
			} else {
				strategy = UIS::ComboUi;
			}
		} else {
			strategy = UIS::EditableUi;
		}
	}

	switch(strategy) {
	case UIS::EditableUi:
		ui = new EditableGuiStrategy(m_generatedRecipe, m_isCompact, m_widgetParent);
		break;
	case UIS::SwitchUi:
	case UIS::ComboUi:
		ui = new ComboAttrUi(m_generatedRecipe, m_isCompact, m_widgetParent);
		break;
	case UIS::RangeUi:
		ui = new RangeAttrUi(m_generatedRecipe, m_isCompact, m_widgetParent);
		break;
	default:
		break;
	}

	return ui;
}

#include "moc_iiowidgetbuilder.cpp"
