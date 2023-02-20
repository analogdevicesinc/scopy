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
#ifndef LOGGING_CATEGORIES_H
#define LOGGING_CATEGORIES_H

#include <QLoggingCategory>
#include "scopycore_export.h"

#define QDEBUG_LOG_TIME
//#define QDEBUG_LOG_DATE
#define QDEBUG_LOG_MSG_TYPE
#define QDEBUG_LOG_FILE
//#define QDEBUG_CATEGORY
//#define QDEBUG_FUNCTION


#define QDEBUG_LOG_TIME_STR "%{time h:mm:ss.zzz}"
#define QDEBUG_LOG_DATE_STR "%{time yyyyMMdd }"
#define QDEBUG_LOG_MSG_TYPE_STR "%{if-debug}D%{endif}%{if-info}I%{endif}%{if-warning}W%{endif}%{if-critical}C%{endif}%{if-fatal}F%{endif}"
#define QDEBUG_LOG_FILE_PREFIX_STR "file://"
#define QDEBUG_LOG_FILE_NAME_STR "%{file}:"
#define QDEBUG_LOG_LINE_NR_STR "%{line}"
#define QDEBUG_LOG_FILE_STR QDEBUG_LOG_FILE_PREFIX_STR QDEBUG_LOG_FILE_NAME_STR QDEBUG_LOG_LINE_NR_STR
#define QDEBUG_CATEGORY_STR "%{category}"
#define QDEBUG_FUNCTION_STR "%{function}"
#define QDEBUG_THREAD_STR "%{threadid}"
#define QDEBUG_PID_STR "%{pid}"
#define QDEBUG_APP_STR "%{appname}"


#ifndef QT_NO_DEBUG_OUTPUT
Q_DECLARE_LOGGING_CATEGORY(CAT_TOOL_LAUNCHER)
Q_DECLARE_LOGGING_CATEGORY(CAT_OSCILLOSCOPE)
Q_DECLARE_LOGGING_CATEGORY(CAT_SIGNAL_GENERATOR)
Q_DECLARE_LOGGING_CATEGORY(CAT_VOLTMETER)
Q_DECLARE_LOGGING_CATEGORY(CAT_POWER_CONTROLLER)
Q_DECLARE_LOGGING_CATEGORY(CAT_SPECTRUM_ANALYZER)
Q_DECLARE_LOGGING_CATEGORY(CAT_NETWORK_ANALYZER)
Q_DECLARE_LOGGING_CATEGORY(CAT_DIGITAL_IO)
Q_DECLARE_LOGGING_CATEGORY(CAT_LOGIC_ANALYZER)
Q_DECLARE_LOGGING_CATEGORY(CAT_PATTERN_GENERATOR)
Q_DECLARE_LOGGING_CATEGORY(CAT_CALIBRATION)
Q_DECLARE_LOGGING_CATEGORY(CAT_CALIBRATION_MANUAL)
Q_DECLARE_LOGGING_CATEGORY(CAT_IIO_MANAGER)
Q_DECLARE_LOGGING_CATEGORY(CAT_PLOT)
#else
#define CAT_TOOL_LAUNCHER
#define CAT_OSCILLOSCOPE
#define CAT_SIGNAL_GENERATOR
#define CAT_VOLTMETER
#define CAT_POWER_CONTROLLER
#define CAT_SPECTRUM_ANALYZER
#define CAT_NETWORK_ANALYZER
#define CAT_DIGITAL_IO
#define CAT_LOGIC_ANALYZER
#define CAT_PATTERN_GENERATOR
#define CAT_CALIBRATION
#define CAT_CALIBRATION_MANUAL
#define CAT_IIO_MANAGER
#define CAT_PLOT
#endif

void SetScopyQDebugMessagePattern();

#endif // LOGGING_CATEGORIES_H
