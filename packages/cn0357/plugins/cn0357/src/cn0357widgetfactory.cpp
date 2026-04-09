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

#include "cn0357widgetfactory.h"

#include <iio-widgets/iiowidgetbuilder.h>

using namespace scopy;
using namespace scopy::cn0357;

IIOWidget *Cn0357WidgetFactory::createComboWidget(iio_device *device, QString attr, QString availableAttr,
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

IIOWidget *Cn0357WidgetFactory::createAdcReadOnlyWidget(iio_device *device, iio_channel *channel, QString attr,
							QString title, double factor, IIOWidgetGroup *group,
							QWidget *parent)
{
	IIOWidget *widget = nullptr;

	if(channel) {
		// Formula: (raw / 32768.0 - 1) * factor
		// factor = 1200.0 for gas sensor input → mV
		// factor = 5.85  for power supply channel → V
		widget = IIOWidgetBuilder(parent).channel(channel).attribute(attr).title(title).buildSingle();
		if(widget) {
			widget->setEnabled(false);
			widget->showProgressBar(false);
			widget->setDataToUIConversion([factor](QString data) -> QString {
				bool ok;
				double raw = data.toDouble(&ok);
				if(!ok)
					return data;
				return QString::number((raw / 32768.0 - 1.0) * factor, 'f', 3);
			});
		}
	} else {
		// Channel not found — use device to satisfy builder guards, then disable
		widget = IIOWidgetBuilder(parent)
				 .device(device)
				 .attribute(attr)
				 .title(title)
				 .uiStrategy(IIOWidgetBuilder::EditableUi)
				 .infoMessage("the " + attr + " is not available")
				 .buildSingle();
		if(widget) {
			widget->setEnabled(false);
			widget->showProgressBar(false);
		}
	}

	if(group && widget)
		group->add(widget);
	return widget;
}
