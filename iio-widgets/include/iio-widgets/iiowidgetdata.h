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

#ifndef SCOPY_IIOWIDGETDATA_H
#define SCOPY_IIOWIDGETDATA_H

#include <iio.h>
#include <QString>
#include "scopy-iio-widgets_export.h"

struct SCOPY_IIO_WIDGETS_EXPORT IIOWidgetFactoryRecipe
{
	struct iio_context *context = nullptr; // the context in which to search for iio structures
	struct iio_device *device = nullptr;   // the device in which to search for iio structures
	struct iio_channel *channel = nullptr; // the channel in which to search for iio structures
	QString data = "";	       // the name of the 'main' attribute that will be changed in the iio channel
	QString iioDataOptions = "";   // the IIO attribute that describes what values can the attribute take
	QString constDataOptions = ""; // the set of constant data that will populate an entry (e.g. combo box, range)
};

#endif // SCOPY_IIOWIDGETDATA_H
