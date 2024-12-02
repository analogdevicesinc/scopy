/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "logging_categories.h"

#ifndef QT_NO_DEBUG_OUTPUT
Q_LOGGING_CATEGORY(CAT_IIO_OPERATION, "RegReadWrite")
Q_LOGGING_CATEGORY(CAT_OSCILLOSCOPE, "oscilloscope")
Q_LOGGING_CATEGORY(CAT_SIGNAL_GENERATOR, "signalGenerator")
Q_LOGGING_CATEGORY(CAT_VOLTMETER, "voltmeter")
Q_LOGGING_CATEGORY(CAT_POWER_CONTROLLER, "powerController")
Q_LOGGING_CATEGORY(CAT_SPECTRUM_ANALYZER, "spectrumAnalyzer")
Q_LOGGING_CATEGORY(CAT_NETWORK_ANALYZER, "networkAnalyzer")
Q_LOGGING_CATEGORY(CAT_DIGITAL_IO, "digitalIO")
Q_LOGGING_CATEGORY(CAT_LOGIC_ANALYZER, "logicAnalyzer")
Q_LOGGING_CATEGORY(CAT_PATTERN_GENERATOR, "patternGenerator")
Q_LOGGING_CATEGORY(CAT_CALIBRATION, "calibration")
Q_LOGGING_CATEGORY(CAT_CALIBRATION_MANUAL, "calibration.manual")
Q_LOGGING_CATEGORY(CAT_IIO_MANAGER, "iioManager")
Q_LOGGING_CATEGORY(CAT_PLOT, "plot")
Q_LOGGING_CATEGORY(CAT_REGMAP, "REGISTER MAP")
Q_LOGGING_CATEGORY(CAT_REGISTER_MAP_TABLE, "RegisterMapTable")
#endif

void SetScopyQDebugMessagePattern() {

	qSetMessagePattern(
        "[ "
		#ifdef QDEBUG_LOG_MSG_TYPE
			QDEBUG_LOG_MSG_TYPE_STR " "
			QDEBUG_CATEGORY_STR " "
		#endif
		#ifdef QDEBUG_LOG_TIME
			QDEBUG_LOG_TIME_STR
		#endif
		#ifdef QDEBUG_LOG_DATE
			QDEBUG_LOG_DATE_STR
		#endif
		#ifdef QDEBUG_LOG_CATEGORY
		QDEBUG_CATEGORY_STR
		#endif
		" ] "
		#ifdef QDEBUG_LOG_FILE
		QDEBUG_LOG_FILE_STR
		#endif

		" - "
		"%{message}"
		);
}
