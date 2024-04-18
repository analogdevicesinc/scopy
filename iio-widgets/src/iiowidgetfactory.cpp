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

#include "iiowidgetfactory.h"
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

IIOWidgetFactory::IIOWidgetFactory(QObject *parent)
	: QObject(parent)
{}

IIOWidgetFactory::~IIOWidgetFactory() {}

QList<IIOWidget *> IIOWidgetFactory::buildAllAttrsForChannel(struct iio_channel *channel, QWidget *parent)
{
	QList<IIOWidget *> result;

	QList<QString> channelAttributes;
	ssize_t channelCount = iio_channel_get_attrs_count(channel);
	for(int i = 0; i < channelCount; ++i) {
		const char *attrName = iio_channel_get_attr(channel, i);
		if(attrName != nullptr) {
			channelAttributes.append(attrName);
		}
	}

	for(const QString &attributeName : channelAttributes) {
		if(attributeName.endsWith("_available")) {
			continue;
		}

		uint32_t hint = AttrData;
		IIOWidgetFactoryRecipe recipe;
		recipe.channel = channel;
		recipe.data = attributeName;
		QString availableAttrName = attributeName + "_available";
		if(channelAttributes.contains(availableAttrName)) {
			recipe.iioDataOptions = availableAttrName;
			char buffer[ATTR_BUFFER_SIZE] = {0};
			ssize_t res = iio_channel_attr_read(channel, availableAttrName.toStdString().c_str(), buffer,
							    ATTR_BUFFER_SIZE);
			if(res < 0) {
				qWarning(CAT_ATTRFACTORY) << "Could not read data from" << availableAttrName;
				continue;
			}

			QString readOptions(buffer);
			if(readOptions.startsWith("[")) {
				hint |= RangeUi;
				//} else if(readOptions.split(" ", Qt::SkipEmptyParts).size() ==2) {
				// hint |= SwitchUi | InstantSave;
			} else {
				hint |= ComboUi;
			}
		} else {
			hint |= EditableUi;
		}

		result.append(IIOWidgetFactory::buildSingle(hint, recipe, parent));
	}

	return result;
}

QList<IIOWidget *> IIOWidgetFactory::buildAllAttrsForDevice(struct iio_device *dev, QWidget *parent)
{
	QList<IIOWidget *> result;

	QList<QString> devAttributes;
	ssize_t devAttrCount = iio_device_get_attrs_count(dev);
	for(int i = 0; i < devAttrCount; ++i) {
		const char *attrName = iio_device_get_attr(dev, i);
		if(attrName != nullptr) {
			devAttributes.append(attrName);
		}
	}

	for(const auto &attributeName : devAttributes) {
		if(attributeName.endsWith("_available")) {
			continue;
		}

		uint32_t hint = DeviceAttrData;
		IIOWidgetFactoryRecipe recipe;
		recipe.device = dev;
		recipe.data = attributeName;
		QString availableAttrName = attributeName + "_available";
		if(devAttributes.contains(availableAttrName)) {
			recipe.iioDataOptions = availableAttrName;
			char buffer[ATTR_BUFFER_SIZE] = {0};
			ssize_t res = iio_device_attr_read(dev, availableAttrName.toStdString().c_str(), buffer,
							   ATTR_BUFFER_SIZE);
			if(res < 0) {
				qWarning(CAT_ATTRFACTORY) << "Could not read data from" << availableAttrName;
				continue;
			}

			QString readOptions(buffer);
			if(readOptions.startsWith("[")) {
				hint |= RangeUi;
				//} else if(readOptions.split(" ", Qt::SkipEmptyParts).size() == 2)
				// hint |= SwitchUi | InstantSave;
				//{ // CustomSwitch is broken
			} else {
				hint |= ComboUi;
			}
		} else {
			hint |= EditableUi;
		}

		result.append(IIOWidgetFactory::buildSingle(hint, recipe, parent));
	}

	return result;
}

QList<IIOWidget *> IIOWidgetFactory::buildAllAttrsForContext(struct iio_context *context, QWidget *parent)
{
	QList<IIOWidget *> result;
	ssize_t attrCount = iio_context_get_attrs_count(context);
	for(int i = 0; i < attrCount; ++i) {
		const char *name;
		const char *value;
		int res = iio_context_get_attr(context, i, &name, &value);

		if(res < 0) {
			qWarning(CAT_ATTRFACTORY) << "Could not read any data from context attr with index" << i;
			continue;
		}

		IIOWidgetFactoryRecipe recipe = {
			.context = context,
			.data = QString(name),
		};

		result.append(IIOWidgetFactory::buildSingle(EditableUi | ContextAttrData, recipe, parent));
	}

	return result;
}

IIOWidget *IIOWidgetFactory::buildSingle(uint32_t hint, IIOWidgetFactoryRecipe recipe, QWidget *parent)
{
	GuiStrategyInterface *uiStrategy = nullptr;
	DataStrategyInterface *dataStrategy = nullptr;
	IIOWidget *attrWidget = nullptr;

	if(hint & AutoHint) {
		// TODO: implement
	}

	if(hint & EditableUi) {
		uiStrategy = new EditableGuiStrategy(recipe, parent);
	} else if(hint & SwitchUi) {
		uiStrategy = new SwitchAttrUi(recipe, parent);
	} else if(hint & ComboUi) {
		uiStrategy = new ComboAttrUi(recipe, parent);
	} else if(hint & RangeUi) {
		uiStrategy = new RangeAttrUi(recipe, parent);
	}

	if(hint & AttrData) {
		dataStrategy = new ChannelAttrDataStrategy(recipe, parent);
	} else if(hint & TriggerData) {
		dataStrategy = new TriggerDataStrategy(recipe, parent);
	} else if(hint & DeviceAttrData) {
		dataStrategy = new DeviceAttrDataStrategy(recipe, parent);
	} else if(hint & ContextAttrData) {
		dataStrategy = new ContextAttrDataStrategy(recipe, parent);
	} else if(hint & CMDQAttrData) {
		if(recipe.connection) {
			dataStrategy = new CmdQChannelAttrDataStrategy(recipe, parent);
		} else {
			qWarning(CAT_ATTRFACTORY)
				<< "Cannot create a CmdQDeviceAttrDataStrategy, no Connection object provided.";
		}
	} else if(hint & CMDQDeviceAttrData) {
		if(recipe.connection) {
			dataStrategy = new CmdQDeviceAttrDataStrategy(recipe, parent);
		} else {
			qWarning(CAT_ATTRFACTORY)
				<< "Cannot create a CmdQDeviceAttrDataStrategy, no Connection object provided.";
		}
	}

	if(uiStrategy && dataStrategy) {
		attrWidget = new IIOWidget(uiStrategy, dataStrategy, parent);
		attrWidget->setRecipe(recipe);
	} else {
		qCritical(CAT_ATTRFACTORY) << "Cannot create an IIOWidget, strategy mismatch.";
	}

	return attrWidget;
}

#include "moc_iiowidgetfactory.cpp"
