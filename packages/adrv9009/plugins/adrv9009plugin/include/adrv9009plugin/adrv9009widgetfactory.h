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

#ifndef ADRV9009WIDGETFACTORY_H
#define ADRV9009WIDGETFACTORY_H

#include "scopy-adrv9009plugin_export.h"
#include <iio-widgets/iiowidget.h>
#include <QString>
#include <QWidget>
#include <iio.h>

namespace scopy::adrv9009 {

class SCOPY_ADRV9009PLUGIN_EXPORT Adrv9009WidgetFactory
{
public:
	// Device attribute widgets
	static IIOWidget *createSpinboxWidget(iio_device *device, QString attr, QString title,
					      QWidget *parent = nullptr);
	static IIOWidget *createCheckboxWidget(iio_device *device, QString attr, QString title,
					       QWidget *parent = nullptr);
	static IIOWidget *createComboWidget(iio_device *device, QString attr, QString availableAttr, QString title,
					    QWidget *parent = nullptr);
	static IIOWidget *createCustomComboWidget(iio_device *device, QString attr, QMap<QString, QString> *optionsMap,
						  QString title, QWidget *parent = nullptr);
	static IIOWidget *createRangeWidget(iio_device *device, QString attr, QString range, QString title,
					    QWidget *parent = nullptr);
	static IIOWidget *createReadOnlyWidget(iio_device *device, QString attr, QString title, bool compactMode = true,
					       QWidget *parent = nullptr);

	// Channel attribute widgets
	static IIOWidget *createSpinboxWidget(iio_channel *channel, QString attr, QString title,
					      QWidget *parent = nullptr);
	static IIOWidget *createCheckboxWidget(iio_channel *channel, QString attr, QString title,
					       QWidget *parent = nullptr);
	static IIOWidget *createComboWidget(iio_channel *channel, QString attr, QString availableAttr, QString title,
					    QWidget *parent = nullptr);
	static IIOWidget *createRangeWidget(iio_channel *channel, QString attr, QString range, QString title,
					    QWidget *parent = nullptr);
	static IIOWidget *createReadOnlyWidget(iio_channel *channel, QString attr, QString title,
					       bool compactMode = true, QWidget *parent = nullptr);

private:
	Adrv9009WidgetFactory() = delete; // Static class, no instances
};

} // namespace scopy::adrv9009

#endif // ADRV9009WIDGETFACTORY_H
