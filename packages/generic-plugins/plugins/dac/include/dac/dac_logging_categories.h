/*
 * Copyright (c) 2023 Analog Devices Inc.
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

#ifndef SCOPY_SWIOT_LOGGING_CATEGORIES_H
#define SCOPY_SWIOT_LOGGING_CATEGORIES_H

#include <QLoggingCategory>

#ifndef QT_NO_DEBUG_OUTPUT
Q_DECLARE_LOGGING_CATEGORY(CAT_DAC)
Q_DECLARE_LOGGING_CATEGORY(CAT_DAC_INSTRUMENT)
Q_DECLARE_LOGGING_CATEGORY(CAT_DAC_DDS)
Q_DECLARE_LOGGING_CATEGORY(CAT_DAC_BUFFER)
Q_DECLARE_LOGGING_CATEGORY(CAT_DAC_DATA)
Q_DECLARE_LOGGING_CATEGORY(CAT_DAC_DATABUILDER)
Q_DECLARE_LOGGING_CATEGORY(CAT_DAC_FILEMANAGER)
Q_DECLARE_LOGGING_CATEGORY(CAT_DAC_DATASTRATEGY)
#else
#define CAT_DAC
#define CAT_DAC_INSTRUMENT
#define CAT_DAC_DDS
#define CAT_DAC_BUFFER
#define CAT_DAC_DATA
#define CAT_DAC_DATABUILDER
#define CAT_DAC_FILEMANAGER
#define CAT_DAC_DATASTRATEGY
#endif

#endif // SCOPY_SWIOT_LOGGING_CATEGORIES_H
