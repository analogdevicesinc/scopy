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
#include <component/attribute.h>
#include <component/channel.h>
#include <component/device.h>
#include <component/navigation.h>

using namespace scopy;
using namespace scopy::adrv9009;

// Device attribute widgets
IIOWidget *Adrv9009WidgetFactory::createSpinboxWidget(component::Device *device, QString attr, QString title,
						      IIOWidgetGroup *group, QWidget *parent)
{
	component::Attribute *a = component::attributeByName(device, attr);
	if(!a) {
		return nullptr;
	}
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .attribute(a)
				    .title(title)
				    .uiStrategy(IIOWidgetBuilder::EditableUi)
				    .buildSingle();
	if(group && widget)
		group->add(widget);
	return widget;
}

IIOWidget *Adrv9009WidgetFactory::createCheckboxWidget(component::Device *device, QString attr, QString title,
						       IIOWidgetGroup *group, QWidget *parent)
{
	component::Attribute *a = component::attributeByName(device, attr);
	if(!a) {
		return nullptr;
	}
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .attribute(a)
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

IIOWidget *Adrv9009WidgetFactory::createComboWidget(component::Device *device, QString attr, QString title,
						    IIOWidgetGroup *group, QWidget *parent)
{
	component::Attribute *a = component::attributeByName(device, attr);
	if(!a) {
		return nullptr;
	}
	IIOWidget *widget =
		IIOWidgetBuilder(parent).attribute(a).title(title).uiStrategy(IIOWidgetBuilder::ComboUi).buildSingle();
	if(group && widget)
		group->add(widget);
	return widget;
}

IIOWidget *Adrv9009WidgetFactory::createCustomComboWidget(component::Device *device, QString attr,
							  const QMap<QString, QString> &optionsMap, QString title,
							  IIOWidgetGroup *group, QWidget *parent)
{
	component::Attribute *a = component::attributeByName(device, attr);
	if(!a) {
		return nullptr;
	}
	// Build space-separated display string from optionsMap values
	auto values = optionsMap.values();
	QString optionsValues = "";
	for(int i = 0; i < values.size(); i++) {
		optionsValues += " " + values.at(i);
	}

	// Create widget with ComboUi strategy
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .attribute(a)
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

IIOWidget *Adrv9009WidgetFactory::createRangeWidget(component::Device *device, QString attr, QString range,
						    QString title, IIOWidgetGroup *group, QWidget *parent)
{
	component::Attribute *a = component::attributeByName(device, attr);
	if(!a) {
		return nullptr;
	}
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .attribute(a)
				    .optionsValues(range)
				    .title(title)
				    .uiStrategy(IIOWidgetBuilder::RangeUi)
				    .buildSingle();
	if(group && widget)
		group->add(widget);
	return widget;
}

IIOWidget *Adrv9009WidgetFactory::createReadOnlyWidget(component::Device *device, QString attr, QString title,
						       bool compactMode, IIOWidgetGroup *group, QWidget *parent)
{
	component::Attribute *a = component::attributeByName(device, attr);
	if(!a) {
		return nullptr;
	}
	IIOWidget *widget = IIOWidgetBuilder(parent).attribute(a).title(title).compactMode(compactMode).buildSingle();
	if(widget) {
		widget->setEnabled(false);
		widget->showProgressBar(false);
	}
	if(group && widget)
		group->add(widget);
	return widget;
}

// Channel attribute widgets
IIOWidget *Adrv9009WidgetFactory::createSpinboxWidget(component::Channel *channel, QString attr, QString title,
						      IIOWidgetGroup *group, QWidget *parent)
{
	component::Attribute *a = component::attributeByName(channel, attr);
	if(!a) {
		return nullptr;
	}
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .attribute(a)
				    .title(title)
				    .uiStrategy(IIOWidgetBuilder::EditableUi)
				    .buildSingle();
	if(group && widget)
		group->add(widget);
	return widget;
}

IIOWidget *Adrv9009WidgetFactory::createCheckboxWidget(component::Channel *channel, QString attr, QString title,
						       IIOWidgetGroup *group, QWidget *parent)
{
	component::Attribute *a = component::attributeByName(channel, attr);
	if(!a) {
		return nullptr;
	}
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .attribute(a)
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

IIOWidget *Adrv9009WidgetFactory::createComboWidget(component::Channel *channel, QString attr, QString title,
						    IIOWidgetGroup *group, QWidget *parent)
{
	component::Attribute *a = component::attributeByName(channel, attr);
	if(!a) {
		return nullptr;
	}
	IIOWidget *widget =
		IIOWidgetBuilder(parent).attribute(a).title(title).uiStrategy(IIOWidgetBuilder::ComboUi).buildSingle();
	if(group && widget)
		group->add(widget);
	return widget;
}

IIOWidget *Adrv9009WidgetFactory::createRangeWidget(component::Channel *channel, QString attr, QString range,
						    QString title, IIOWidgetGroup *group, QWidget *parent)
{
	component::Attribute *a = component::attributeByName(channel, attr);
	if(!a) {
		return nullptr;
	}
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .attribute(a)
				    .optionsValues(range)
				    .title(title)
				    .uiStrategy(IIOWidgetBuilder::RangeUi)
				    .buildSingle();
	if(group && widget)
		group->add(widget);
	return widget;
}

IIOWidget *Adrv9009WidgetFactory::createReadOnlyWidget(component::Channel *channel, QString attr, QString title,
						       bool compactMode, IIOWidgetGroup *group, QWidget *parent)
{
	component::Attribute *a = component::attributeByName(channel, attr);
	if(!a) {
		return nullptr;
	}
	IIOWidget *widget = IIOWidgetBuilder(parent).attribute(a).title(title).compactMode(compactMode).buildSingle();
	if(widget) {
		widget->setEnabled(false);
		widget->showProgressBar(false);
	}
	if(group && widget)
		group->add(widget);
	return widget;
}

// Debug attribute widgets — debug attributes are plain Attribute children of the
// device in the component tree, resolved by name like any other attribute.
IIOWidget *Adrv9009WidgetFactory::createDebugRangeWidget(component::Device *device, QString attr, QString range,
							 QString title, IIOWidgetGroup *group, QWidget *parent)
{
	component::Attribute *a = component::attributeByName(device, attr);
	if(!a) {
		return nullptr;
	}
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .attribute(a)
				    .optionsValues(range)
				    .title(title)
				    .uiStrategy(IIOWidgetBuilder::RangeUi)
				    .buildSingle();
	if(group && widget)
		group->add(widget);
	return widget;
}

IIOWidget *Adrv9009WidgetFactory::createDebugCustomComboWidget(component::Device *device, QString attr,
							       const QMap<QString, QString> &optionsMap, QString title,
							       IIOWidgetGroup *group, QWidget *parent)
{
	component::Attribute *a = component::attributeByName(device, attr);
	if(!a) {
		return nullptr;
	}
	// Build space-separated display string from optionsMap values
	auto values = optionsMap.values();
	QString optionsValues = "";
	for(int i = 0; i < values.size(); i++) {
		if(i > 0)
			optionsValues += " ";
		// Use underscores in display values as per porting rules
		QString value = values.at(i);
		optionsValues += value.replace(" ", "_");
	}

	// Create widget with ComboUi strategy
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .attribute(a)
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

IIOWidget *Adrv9009WidgetFactory::createDebugCheckboxWidget(component::Device *device, QString attr, QString title,
							    IIOWidgetGroup *group, QWidget *parent)
{
	component::Attribute *a = component::attributeByName(device, attr);
	if(!a) {
		return nullptr;
	}
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .attribute(a)
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
