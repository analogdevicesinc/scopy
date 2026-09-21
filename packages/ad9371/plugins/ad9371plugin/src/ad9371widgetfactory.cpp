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

#include <component/device.h>
#include <component/channel.h>
#include <component/attribute.h>
#include <component/navigation.h>

using namespace scopy;
using namespace scopy::ad9371;

// Device attribute widgets
IIOWidget *Ad9371WidgetFactory::createSpinboxWidget(component::Device *device, QString attr, QString title,
						    QWidget *parent)
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
	return widget;
}

IIOWidget *Ad9371WidgetFactory::createCheckboxWidget(component::Device *device, QString attr, QString title,
						     QWidget *parent)
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
	return widget;
}

IIOWidget *Ad9371WidgetFactory::createComboWidget(component::Device *device, QString attr, QString availableAttr,
						  QString title, QWidget *parent)
{
	Q_UNUSED(availableAttr)
	component::Attribute *a = component::attributeByName(device, attr);
	if(!a) {
		return nullptr;
	}
	IIOWidget *widget =
		IIOWidgetBuilder(parent).attribute(a).title(title).uiStrategy(IIOWidgetBuilder::ComboUi).buildSingle();
	return widget;
}

IIOWidget *Ad9371WidgetFactory::createCustomComboWidget(component::Device *device, QString attr,
							const QMap<QString, QString> &optionsMap, QString title,
							QWidget *parent)
{
	component::Attribute *a = component::attributeByName(device, attr);
	if(!a) {
		return nullptr;
	}

	auto values = optionsMap.values();
	QString optionsValues = "";
	for(int i = 0; i < values.size(); i++) {
		optionsValues += " " + values.at(i);
	}

	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .attribute(a)
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

IIOWidget *Ad9371WidgetFactory::createRangeWidget(component::Device *device, QString attr, QString range, QString title,
						  QWidget *parent)
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
	return widget;
}

IIOWidget *Ad9371WidgetFactory::createReadOnlyWidget(component::Device *device, QString attr, QString title,
						     bool compactMode, QWidget *parent)
{
	Q_UNUSED(compactMode)
	component::Attribute *a = component::attributeByName(device, attr);
	if(!a) {
		return nullptr;
	}
	IIOWidget *widget = IIOWidgetBuilder(parent).attribute(a).title(title).buildSingle();
	if(widget) {
		widget->setEnabled(false);
	}
	return widget;
}

// Channel attribute widgets
IIOWidget *Ad9371WidgetFactory::createSpinboxWidget(component::Channel *channel, QString attr, QString title,
						    QWidget *parent)
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
	return widget;
}

IIOWidget *Ad9371WidgetFactory::createCheckboxWidget(component::Channel *channel, QString attr, QString title,
						     QWidget *parent)
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
	return widget;
}

IIOWidget *Ad9371WidgetFactory::createComboWidget(component::Channel *channel, QString attr, QString availableAttr,
						  QString title, QWidget *parent)
{
	Q_UNUSED(availableAttr)
	component::Attribute *a = component::attributeByName(channel, attr);
	if(!a) {
		return nullptr;
	}
	IIOWidget *widget =
		IIOWidgetBuilder(parent).attribute(a).title(title).uiStrategy(IIOWidgetBuilder::ComboUi).buildSingle();
	return widget;
}

IIOWidget *Ad9371WidgetFactory::createRangeWidget(component::Channel *channel, QString attr, QString range,
						  QString title, QWidget *parent)
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
	return widget;
}

IIOWidget *Ad9371WidgetFactory::createReadOnlyWidget(component::Channel *channel, QString attr, QString title,
						     bool compactMode, QWidget *parent)
{
	Q_UNUSED(compactMode)
	component::Attribute *a = component::attributeByName(channel, attr);
	if(!a) {
		return nullptr;
	}
	IIOWidget *widget = IIOWidgetBuilder(parent).attribute(a).title(title).buildSingle();
	if(widget) {
		widget->setEnabled(false);
	}
	return widget;
}

// Debug attribute widgets (debug attrs are regular Device attribute children in the component tree)
IIOWidget *Ad9371WidgetFactory::createDebugRangeWidget(component::Device *device, QString attr, QString range,
						       QString title, QWidget *parent)
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
	return widget;
}

IIOWidget *Ad9371WidgetFactory::createDebugCustomComboWidget(component::Device *device, QString attr,
							     const QMap<QString, QString> &optionsMap, QString title,
							     QWidget *parent)
{
	component::Attribute *a = component::attributeByName(device, attr);
	if(!a) {
		return nullptr;
	}

	auto values = optionsMap.values();
	QString optionsValues = "";
	for(int i = 0; i < values.size(); i++) {
		if(i > 0)
			optionsValues += " ";
		QString value = values.at(i);
		optionsValues += value.replace(" ", "_");
	}

	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .attribute(a)
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

IIOWidget *Ad9371WidgetFactory::createDebugCheckboxWidget(component::Device *device, QString attr, QString title,
							  QWidget *parent)
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
	return widget;
}
