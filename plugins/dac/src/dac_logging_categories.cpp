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

#include "dac_logging_categories.h"

#ifndef QT_NO_DEBUG_OUTPUT
Q_LOGGING_CATEGORY(CAT_DAC, "DacPlugin");
Q_LOGGING_CATEGORY(CAT_DAC_INSTRUMENT, "DacInstrument")
Q_LOGGING_CATEGORY(CAT_DAC_DDS, "DacDds")
Q_LOGGING_CATEGORY(CAT_DAC_BUFFER, "DacBuffer")
Q_LOGGING_CATEGORY(CAT_DAC_DATA, "DacData")
Q_LOGGING_CATEGORY(CAT_DAC_DATABUILDER, "DacDataBuilder")
Q_LOGGING_CATEGORY(CAT_DAC_FILEMANAGER, "DacFileManager")
Q_LOGGING_CATEGORY(CAT_DAC_DATASTRATEGY, "DacDataStrategy")
#endif
