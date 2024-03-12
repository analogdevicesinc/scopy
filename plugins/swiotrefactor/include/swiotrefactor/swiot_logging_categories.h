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
Q_DECLARE_LOGGING_CATEGORY(CAT_SWIOT)
Q_DECLARE_LOGGING_CATEGORY(CAT_SWIOT_CONFIG)
Q_DECLARE_LOGGING_CATEGORY(CAT_SWIOT_AD74413R)
Q_DECLARE_LOGGING_CATEGORY(CAT_SWIOT_FAULTS)
Q_DECLARE_LOGGING_CATEGORY(CAT_SWIOT_MAX14906)
#else
#define CAT_SWIOT
#define CAT_SWIOT_CONFIG
#define CAT_SWIOT_AD74413R
#define CAT_SWIOT_FAULTS
#define CAT_SWIOT_MAX14906
#endif

#endif // SCOPY_SWIOT_LOGGING_CATEGORIES_H
