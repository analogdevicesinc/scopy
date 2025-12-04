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

#include "adrv9009widgetfactory.h"
#include <iio-widgets/iiowidgetbuilder.h>
#include <iio-widgets/iiowidgetutils.h>

using namespace scopy;
using namespace scopy::adrv9009;

// Device attribute widgets
IIOWidget *Adrv9009WidgetFactory::createSpinboxWidget(iio_device *device, QString attr, QString title, QWidget *parent)
{
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .device(device)
				    .attribute(attr)
				    .title(title)
				    .uiStrategy(IIOWidgetBuilder::EditableUi)
				    .buildSingle();
	return widget;
}

IIOWidget *Adrv9009WidgetFactory::createCheckboxWidget(iio_device *device, QString attr, QString title, QWidget *parent)
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

IIOWidget *Adrv9009WidgetFactory::createComboWidget(iio_device *device, QString attr, QString availableAttr,
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

IIOWidget *Adrv9009WidgetFactory::createCustomComboWidget(iio_device *device, QString attr,
							  QMap<QString, QString> *optionsMap, QString title,
							  QWidget *parent)
{
	// Build space-separated display string from optionsMap values
	auto values = optionsMap->values();
	QString optionsValues = "";
	for(int i = 0; i < values.size(); i++) {
		optionsValues += " " + values.at(i);
	}

	// Create widget with ComboUi strategy
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .device(device)
				    .attribute(attr)
				    .title(title)
				    .uiStrategy(IIOWidgetBuilder::ComboUi)
				    .optionsValues(optionsValues)
				    .buildSingle();

	// Set bidirectional conversion functions
	if(widget) {
		widget->setUItoDataConversion([optionsMap](QString data) {
			return IIOWidgetUtils::comboUiToDataConversionFunction(data, optionsMap);
		});
		widget->setDataToUIConversion([optionsMap](QString data) {
			return IIOWidgetUtils::comboDataToUiConversionFunction(data, optionsMap);
		});
	}

	return widget;
}

IIOWidget *Adrv9009WidgetFactory::createRangeWidget(iio_device *device, QString attr, QString range, QString title,
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

IIOWidget *Adrv9009WidgetFactory::createReadOnlyWidget(iio_device *device, QString attr, QString title,
						       bool compactMode, QWidget *parent)
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
IIOWidget *Adrv9009WidgetFactory::createSpinboxWidget(iio_channel *channel, QString attr, QString title,
						      QWidget *parent)
{
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .channel(channel)
				    .attribute(attr)
				    .title(title)
				    .uiStrategy(IIOWidgetBuilder::EditableUi)
				    .buildSingle();
	return widget;
}

IIOWidget *Adrv9009WidgetFactory::createCheckboxWidget(iio_channel *channel, QString attr, QString title,
						       QWidget *parent)
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

IIOWidget *Adrv9009WidgetFactory::createComboWidget(iio_channel *channel, QString attr, QString availableAttr,
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

IIOWidget *Adrv9009WidgetFactory::createRangeWidget(iio_channel *channel, QString attr, QString range, QString title,
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

IIOWidget *Adrv9009WidgetFactory::createReadOnlyWidget(iio_channel *channel, QString attr, QString title,
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
