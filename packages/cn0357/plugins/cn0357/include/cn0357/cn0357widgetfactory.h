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

#ifndef CN0357WIDGETFACTORY_H
#define CN0357WIDGETFACTORY_H

#include "scopy-cn0357_export.h"
#include <QString>
#include <QWidget>
#include <iio-widgets/iiowidget.h>
#include <iio-widgets/iiowidgetgroup.h>
#include <iio.h>

namespace scopy::cn0357 {

class SCOPY_CN0357_EXPORT Cn0357WidgetFactory
{
public:
	// Sampling frequency combo (device-level, uses _available attribute)
	static IIOWidget *createComboWidget(iio_device *device, QString attr, QString availableAttr, QString title,
					    IIOWidgetGroup *group = nullptr, QWidget *parent = nullptr);

	// ADC raw ReadOnly widget with linear conversion: (raw/32768.0 - 1) * factor
	// Used for voltage0-voltage0 (factor=1200.0 → mV) and supply (factor=5.85 → V)
	// When channel is null, creates a disabled widget with info message instead
	static IIOWidget *createAdcReadOnlyWidget(iio_device *device, iio_channel *channel, QString attr, QString title,
						  double factor, IIOWidgetGroup *group = nullptr,
						  QWidget *parent = nullptr);

private:
	Cn0357WidgetFactory() = delete;
};

} // namespace scopy::cn0357
#endif // CN0357WIDGETFACTORY_H
