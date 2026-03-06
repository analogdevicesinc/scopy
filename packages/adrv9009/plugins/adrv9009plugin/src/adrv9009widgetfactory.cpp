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
IIOWidget *Adrv9009WidgetFactory::createSpinboxWidget(iio_device *device, QString attr, QString title,
						      IIOWidgetGroup *group, QWidget *parent)
{
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .device(device)
				    .attribute(attr)
				    .title(title)
				    .uiStrategy(IIOWidgetBuilder::EditableUi)
				    .buildSingle();
	if(group && widget)
		group->add(widget);
	return widget;
}

IIOWidget *Adrv9009WidgetFactory::createCheckboxWidget(iio_device *device, QString attr, QString title,
						       IIOWidgetGroup *group, QWidget *parent)
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
	if(group && widget)
		group->add(widget);
	return widget;
}

IIOWidget *Adrv9009WidgetFactory::createComboWidget(iio_device *device, QString attr, QString availableAttr,
						    QString title, IIOWidgetGroup *group, QWidget *parent)
{
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .device(device)
				    .attribute(attr)
				    .optionsAttribute(availableAttr)
				    .title(title)
				    .uiStrategy(IIOWidgetBuilder::ComboUi)
				    .buildSingle();
	if(group && widget)
		group->add(widget);
	return widget;
}

IIOWidget *Adrv9009WidgetFactory::createCustomComboWidget(iio_device *device, QString attr,
							  const QMap<QString, QString> &optionsMap, QString title,
							  IIOWidgetGroup *group, QWidget *parent)
{
	// Build space-separated display string from optionsMap values
	auto values = optionsMap.values();
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

	// Set bidirectional conversion functions; capture map by value so the widget
	// owns its own copy and has no dependency on the caller's lifetime.
	if(widget) {
		widget->setUItoDataConversion([map = QMap<QString, QString>(optionsMap)](QString data) mutable {
			return IIOWidgetUtils::comboUiToDataConversionFunction(data, &map);
		});
		widget->setDataToUIConversion([map = QMap<QString, QString>(optionsMap)](QString data) mutable {
			return IIOWidgetUtils::comboDataToUiConversionFunction(data, &map);
		});
	}
	if(group && widget)
		group->add(widget);
	return widget;
}

IIOWidget *Adrv9009WidgetFactory::createRangeWidget(iio_device *device, QString attr, QString range, QString title,
						    IIOWidgetGroup *group, QWidget *parent)
{
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .device(device)
				    .attribute(attr)
				    .optionsValues(range)
				    .title(title)
				    .uiStrategy(IIOWidgetBuilder::RangeUi)
				    .buildSingle();
	if(group && widget)
		group->add(widget);
	return widget;
}

IIOWidget *Adrv9009WidgetFactory::createReadOnlyWidget(iio_device *device, QString attr, QString title,
						       bool compactMode, IIOWidgetGroup *group, QWidget *parent)
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
	if(group && widget)
		group->add(widget);
	return widget;
}

// Channel attribute widgets
IIOWidget *Adrv9009WidgetFactory::createSpinboxWidget(iio_channel *channel, QString attr, QString title,
						      IIOWidgetGroup *group, QWidget *parent)
{
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .channel(channel)
				    .attribute(attr)
				    .title(title)
				    .uiStrategy(IIOWidgetBuilder::EditableUi)
				    .buildSingle();
	if(group && widget)
		group->add(widget);
	return widget;
}

IIOWidget *Adrv9009WidgetFactory::createCheckboxWidget(iio_channel *channel, QString attr, QString title,
						       IIOWidgetGroup *group, QWidget *parent)
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
	if(group && widget)
		group->add(widget);
	return widget;
}

IIOWidget *Adrv9009WidgetFactory::createComboWidget(iio_channel *channel, QString attr, QString availableAttr,
						    QString title, IIOWidgetGroup *group, QWidget *parent)
{
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .channel(channel)
				    .attribute(attr)
				    .optionsAttribute(availableAttr)
				    .title(title)
				    .uiStrategy(IIOWidgetBuilder::ComboUi)
				    .buildSingle();
	if(group && widget)
		group->add(widget);
	return widget;
}

IIOWidget *Adrv9009WidgetFactory::createRangeWidget(iio_channel *channel, QString attr, QString range, QString title,
						    IIOWidgetGroup *group, QWidget *parent)
{
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .channel(channel)
				    .attribute(attr)
				    .optionsValues(range)
				    .title(title)
				    .uiStrategy(IIOWidgetBuilder::RangeUi)
				    .buildSingle();
	if(group && widget)
		group->add(widget);
	return widget;
}

IIOWidget *Adrv9009WidgetFactory::createReadOnlyWidget(iio_channel *channel, QString attr, QString title,
						       bool compactMode, IIOWidgetGroup *group, QWidget *parent)
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
	if(group && widget)
		group->add(widget);
	return widget;
}

// Debug attribute widgets (following proper porting rules)
IIOWidget *Adrv9009WidgetFactory::createDebugRangeWidget(iio_device *device, QString attr, QString range, QString title,
							 IIOWidgetGroup *group, QWidget *parent)
{
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .device(device)
				    .attribute(attr)
				    .optionsValues(range)
				    .title(title)
				    .uiStrategy(IIOWidgetBuilder::RangeUi) // UI strategy auto-sets data strategy
				    .includeDebugAttributes(true)
				    .buildSingle();
	if(group && widget)
		group->add(widget);
	return widget;
}

IIOWidget *Adrv9009WidgetFactory::createDebugCustomComboWidget(iio_device *device, QString attr,
							       const QMap<QString, QString> &optionsMap, QString title,
							       IIOWidgetGroup *group, QWidget *parent)
{
	// Build space-separated display string from optionsMap values (following Template 2B)
	auto values = optionsMap.values();
	QString optionsValues = "";
	for(int i = 0; i < values.size(); i++) {
		if(i > 0)
			optionsValues += " ";
		// Use underscores in display values as per porting rules
		QString value = values.at(i);
		optionsValues += value.replace(" ", "_");
	}

	// Create widget with ComboUi strategy (following porting rules)
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .device(device)
				    .attribute(attr)
				    .title(title)
				    .uiStrategy(IIOWidgetBuilder::ComboUi) // UI strategy auto-sets data strategy
				    .optionsValues(optionsValues)
				    .includeDebugAttributes(true)
				    .buildSingle();

	// Set bidirectional conversion functions; capture map by value so the widget
	// owns its own copy and has no dependency on the caller's lifetime.
	if(widget) {
		widget->setUItoDataConversion([map = QMap<QString, QString>(optionsMap)](QString data) mutable {
			return IIOWidgetUtils::comboUiToDataConversionFunction(data, &map);
		});
		widget->setDataToUIConversion([map = QMap<QString, QString>(optionsMap)](QString data) mutable {
			return IIOWidgetUtils::comboDataToUiConversionFunction(data, &map);
		});
	}
	if(group && widget)
		group->add(widget);
	return widget;
}

IIOWidget *Adrv9009WidgetFactory::createDebugCheckboxWidget(iio_device *device, QString attr, QString title,
							    IIOWidgetGroup *group, QWidget *parent)
{
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .device(device)
				    .attribute(attr)
				    .title(title)
				    .uiStrategy(IIOWidgetBuilder::CheckBoxUi) // UI strategy auto-sets data strategy
				    .includeDebugAttributes(true)
				    .buildSingle();
	if(widget) {
		widget->showProgressBar(false);
	}
	if(group && widget)
		group->add(widget);
	return widget;
}
