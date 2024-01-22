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
#include "savestrategy/timesavestrategy.h"
#include "savestrategy/instantsavestrategy.h"
#include "savestrategy/externalsavestrategy.h"
#include "guistrategy/editableguistrategy.h"
#include "guistrategy/switchguistrategy.h"
#include "datastrategy/channelattrdatastrategy.h"
#include "datastrategy/triggerdatastrategy.h"
#include "datastrategy/deviceattrdatastrategy.h"
#include "datastrategy/filedemodatastrategy.h"
#include "datastrategy/contextattrdatastrategy.h"
#include "guistrategy/comboguistrategy.h"
#include "guistrategy/rangeguistrategy.h"
#include <QLoggingCategory>

using namespace scopy;

#define ATTR_BUFFER_SIZE 256
Q_LOGGING_CATEGORY(CAT_ATTRFACTORY, "AttrFactory")

IIOWidgetFactory::IIOWidgetFactory(QWidget *parent)
	: QWidget(parent)
{}

IIOWidgetFactory::~IIOWidgetFactory() {}

QList<IIOWidget *> IIOWidgetFactory::buildAllAttrsForChannel(struct iio_channel *channel)
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

	for(const auto &attributeName : channelAttributes) {
		if(attributeName.endsWith("_available")) {
			continue;
		}

		uint32_t hint = AttrData;
		IIOWidgetFactoryRecipe recipe;
		recipe.channel = channel;
		recipe.data = attributeName;
		QString availableAttrName = attributeName + "_available";
		if(channelAttributes.contains(availableAttrName)) {
			recipe.dataOptions = availableAttrName;
			char buffer[ATTR_BUFFER_SIZE] = {0};
			ssize_t res = iio_channel_attr_read(channel, availableAttrName.toStdString().c_str(), buffer,
							    ATTR_BUFFER_SIZE);
			if(res < 0) {
				qWarning(CAT_ATTRFACTORY) << "Could not read data from" << availableAttrName;
				continue;
			}

			QString readOptions(buffer);
			if(readOptions.startsWith("[")) {
				hint |= RangeUi | TimeSave;
			} else if(readOptions.split(" ", Qt::SkipEmptyParts).size() == 2) {
				hint |= SwitchUi | InstantSave;
			} else {
				hint |= ComboUi | InstantSave;
			}
		} else {
			hint |= EditableUi | InstantSave;
		}

		result.append(this->buildSingle(hint, recipe));
	}

	return result;
}

QList<IIOWidget *> IIOWidgetFactory::buildAllAttrsForDevice(struct iio_device *dev)
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
			recipe.dataOptions = availableAttrName;
			char buffer[ATTR_BUFFER_SIZE] = {0};
			ssize_t res = iio_device_attr_read(dev, availableAttrName.toStdString().c_str(), buffer,
							   ATTR_BUFFER_SIZE);
			if(res < 0) {
				qWarning(CAT_ATTRFACTORY) << "Could not read data from" << availableAttrName;
				continue;
			}

			QString readOptions(buffer);
			if(readOptions.startsWith("[")) {
				hint |= RangeUi | TimeSave;
			} else if(readOptions.split(" ", Qt::SkipEmptyParts).size() == 2) { // CustomSwitch is broken
				hint |= SwitchUi | InstantSave;
			} else {
				hint |= ComboUi | TimeSave;
			}
		} else {
			hint |= EditableUi | TimeSave;
		}

		result.append(this->buildSingle(hint, recipe));
	}

	return result;
}

QList<IIOWidget *> IIOWidgetFactory::buildAllAttrsForContext(struct iio_context *context)
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

		result.append(this->buildSingle(EditableUi | InstantSave | ContextAttrData, recipe));
	}

	return result;
}

IIOWidget *IIOWidgetFactory::buildSingle(uint32_t hint, IIOWidgetFactoryRecipe recipe)
{
	AttrUiStrategyInterface *uiStrategy = nullptr;
	SaveStrategyInterface *saveStrategy = nullptr;
	DataStrategyInterface *dataStrategy = nullptr;
	IIOWidget *attrWidget = nullptr;

	if(hint & AutoHint) {
		// TODO: implement
	}

	if(hint & TimeSave) {
		saveStrategy = new TimerSaveStrategy(recipe, this);
	} else if(hint & InstantSave) {
		saveStrategy = new InstantSaveStrategy(recipe, this);
	} else if(hint & ExternalSave) {
		saveStrategy = new ExternalSaveStrategy(recipe, this);
	}

	if(hint & EditableUi) {
		uiStrategy = new EditableGuiStrategy(recipe, this);
	} else if(hint & SwitchUi) {
		uiStrategy = new SwitchAttrUi(recipe, this);
	} else if(hint & ComboUi) {
		uiStrategy = new ComboAttrUi(recipe, this);
	} else if(hint & RangeUi) {
		uiStrategy = new RangeAttrUi(recipe, this);
	}

	if(hint & AttrData) {
		dataStrategy = new ChannelAttrDataStrategy(recipe, this);
	} else if(hint & TriggerData) {
		dataStrategy = new TriggerDataStrategy(recipe, this);
	} else if(hint & DeviceAttrData) {
		dataStrategy = new DeviceAttrDataStrategy(recipe, this);
	} else if(hint & FileDemoData) {
		dataStrategy = new FileDemoDataStrategy(recipe, this);
	} else if(hint & ContextAttrData) {
		dataStrategy = new ContextAttrDataStrategy(recipe, this);
	}

	if(uiStrategy && saveStrategy && dataStrategy) {
		attrWidget = new IIOWidget(uiStrategy, saveStrategy, dataStrategy, this);
		attrWidget->setRecipe(recipe);
	}

	return attrWidget;
}

#include "moc_iiowidgetfactory.cpp"
