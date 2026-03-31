/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#include "ad9371widgetfactory.h"
#include <iio-widgets/iiowidgetbuilder.h>
#include <iio-widgets/iiowidgetutils.h>

using namespace scopy;
using namespace scopy::ad9371;

// Device attribute widgets
IIOWidget *Ad9371WidgetFactory::createSpinboxWidget(iio_device *device, QString attr, QString title, QWidget *parent)
{
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .device(device)
				    .attribute(attr)
				    .title(title)
				    .uiStrategy(IIOWidgetBuilder::EditableUi)
				    .buildSingle();
	return widget;
}

IIOWidget *Ad9371WidgetFactory::createCheckboxWidget(iio_device *device, QString attr, QString title, QWidget *parent)
{
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .device(device)
				    .attribute(attr)
				    .title(title)
				    .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
				    .buildSingle();
	if(widget) {
		widget->showProgressBar(false);
	}
	return widget;
}

IIOWidget *Ad9371WidgetFactory::createComboWidget(iio_device *device, QString attr, QString availableAttr,
						  QString title, QWidget *parent)
{
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .device(device)
				    .attribute(attr)
				    .optionsAttribute(availableAttr)
				    .title(title)
				    .uiStrategy(IIOWidgetBuilder::ComboUi)
				    .buildSingle();
	return widget;
}

IIOWidget *Ad9371WidgetFactory::createCustomComboWidget(iio_device *device, QString attr,
							const QMap<QString, QString> &optionsMap, QString title,
							QWidget *parent)
{
	auto values = optionsMap.values();
	QString optionsValues = "";
	for(int i = 0; i < values.size(); i++) {
		optionsValues += " " + values.at(i);
	}

	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .device(device)
				    .attribute(attr)
				    .title(title)
				    .uiStrategy(IIOWidgetBuilder::ComboUi)
				    .optionsValues(optionsValues)
				    .buildSingle();

	if(widget) {
		widget->setUItoDataConversion([map = QMap<QString, QString>(optionsMap)](QString data) mutable {
			return IIOWidgetUtils::comboUiToDataConversionFunction(data, &map);
		});
		widget->setDataToUIConversion([map = QMap<QString, QString>(optionsMap)](QString data) mutable {
			return IIOWidgetUtils::comboDataToUiConversionFunction(data, &map);
		});
	}

	return widget;
}

IIOWidget *Ad9371WidgetFactory::createRangeWidget(iio_device *device, QString attr, QString range, QString title,
						  QWidget *parent)
{
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .device(device)
				    .attribute(attr)
				    .optionsValues(range)
				    .title(title)
				    .uiStrategy(IIOWidgetBuilder::RangeUi)
				    .buildSingle();
	return widget;
}

IIOWidget *Ad9371WidgetFactory::createReadOnlyWidget(iio_device *device, QString attr, QString title, bool compactMode,
						     QWidget *parent)
{
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .device(device)
				    .attribute(attr)
				    .title(title)
				    .compactMode(compactMode)
				    .buildSingle();
	if(widget) {
		widget->setEnabled(false);
		widget->showProgressBar(false);
	}
	return widget;
}

// Channel attribute widgets
IIOWidget *Ad9371WidgetFactory::createSpinboxWidget(iio_channel *channel, QString attr, QString title, QWidget *parent)
{
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .channel(channel)
				    .attribute(attr)
				    .title(title)
				    .uiStrategy(IIOWidgetBuilder::EditableUi)
				    .buildSingle();
	return widget;
}

IIOWidget *Ad9371WidgetFactory::createCheckboxWidget(iio_channel *channel, QString attr, QString title, QWidget *parent)
{
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .channel(channel)
				    .attribute(attr)
				    .title(title)
				    .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
				    .buildSingle();
	if(widget) {
		widget->showProgressBar(false);
	}
	return widget;
}

IIOWidget *Ad9371WidgetFactory::createComboWidget(iio_channel *channel, QString attr, QString availableAttr,
						  QString title, QWidget *parent)
{
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .channel(channel)
				    .attribute(attr)
				    .optionsAttribute(availableAttr)
				    .title(title)
				    .uiStrategy(IIOWidgetBuilder::ComboUi)
				    .buildSingle();
	return widget;
}

IIOWidget *Ad9371WidgetFactory::createRangeWidget(iio_channel *channel, QString attr, QString range, QString title,
						  QWidget *parent)
{
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .channel(channel)
				    .attribute(attr)
				    .optionsValues(range)
				    .title(title)
				    .uiStrategy(IIOWidgetBuilder::RangeUi)
				    .buildSingle();
	return widget;
}

IIOWidget *Ad9371WidgetFactory::createReadOnlyWidget(iio_channel *channel, QString attr, QString title,
						     bool compactMode, QWidget *parent)
{
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .channel(channel)
				    .attribute(attr)
				    .title(title)
				    .compactMode(compactMode)
				    .buildSingle();
	if(widget) {
		widget->setEnabled(false);
		widget->showProgressBar(false);
	}
	return widget;
}

// Debug attribute widgets
IIOWidget *Ad9371WidgetFactory::createDebugRangeWidget(iio_device *device, QString attr, QString range, QString title,
						       QWidget *parent)
{
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .device(device)
				    .attribute(attr)
				    .optionsValues(range)
				    .title(title)
				    .uiStrategy(IIOWidgetBuilder::RangeUi)
				    .includeDebugAttributes(true)
				    .buildSingle();
	return widget;
}

IIOWidget *Ad9371WidgetFactory::createDebugCustomComboWidget(iio_device *device, QString attr,
							     const QMap<QString, QString> &optionsMap, QString title,
							     QWidget *parent)
{
	auto values = optionsMap.values();
	QString optionsValues = "";
	for(int i = 0; i < values.size(); i++) {
		if(i > 0)
			optionsValues += " ";
		QString value = values.at(i);
		optionsValues += value.replace(" ", "_");
	}

	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .device(device)
				    .attribute(attr)
				    .title(title)
				    .uiStrategy(IIOWidgetBuilder::ComboUi)
				    .optionsValues(optionsValues)
				    .includeDebugAttributes(true)
				    .buildSingle();

	if(widget) {
		widget->setUItoDataConversion([map = QMap<QString, QString>(optionsMap)](QString data) mutable {
			return IIOWidgetUtils::comboUiToDataConversionFunction(data, &map);
		});
		widget->setDataToUIConversion([map = QMap<QString, QString>(optionsMap)](QString data) mutable {
			return IIOWidgetUtils::comboDataToUiConversionFunction(data, &map);
		});
	}

	return widget;
}

IIOWidget *Ad9371WidgetFactory::createDebugCheckboxWidget(iio_device *device, QString attr, QString title,
							  QWidget *parent)
{
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .device(device)
				    .attribute(attr)
				    .title(title)
				    .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
				    .includeDebugAttributes(true)
				    .buildSingle();
	if(widget) {
		widget->showProgressBar(false);
	}
	return widget;
}
