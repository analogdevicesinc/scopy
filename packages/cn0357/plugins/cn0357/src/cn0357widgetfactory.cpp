/*
 * Copyright (c) 2026 Analog Devices Inc.
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
#include <component/device.h>
#include <component/channel.h>
#include <component/attribute.h>
#include <component/navigation.h>

using namespace scopy;
using namespace scopy::cn0357;

IIOWidget *Cn0357WidgetFactory::createComboWidget(component::Device *device, QString attr, QString availableAttr,
						  QString title, IIOWidgetGroup *group, QWidget *parent)
{
	// options are derived from the component::Attribute metadata (populated from
	// the "<attr>_available" companion); availableAttr is kept for documentation.
	Q_UNUSED(availableAttr)
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .attribute(component::attributeByName(device, attr))
				    .title(title)
				    .uiStrategy(IIOWidgetBuilder::ComboUi)
				    .buildSingle();
	if(group && widget)
		group->add(widget);
	return widget;
}

IIOWidget *Cn0357WidgetFactory::createAdcReadOnlyWidget(component::Channel *channel, QString attr, QString title,
							double factor, IIOWidgetGroup *group, QWidget *parent)
{
	if(!channel)
		return nullptr;

	// Formula: (raw / 32768.0 - 1) * factor
	// factor = 1200.0 for gas sensor input → mV
	// factor = 5.85  for power supply channel → V
	IIOWidget *widget = IIOWidgetBuilder(parent)
				    .attribute(component::attributeByName(channel, attr))
				    .title(title)
				    .buildSingle();
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

	if(group && widget)
		group->add(widget);
	return widget;
}
