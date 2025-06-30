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
 *
 */

#ifndef SCOPYEXCEPTIONHANDLER_H
#define SCOPYEXCEPTIONHANDLER_H

#include <QApplication>
//#include <config.h>
#include <libm2k/m2kexceptions.hpp>
//#include <scopyApplication.hpp>
#include <QDebug>
#include <QThread>

// using namespace scopy::m2k;

// Debug messages are currently disabled for users, can be used for development.
// The message box was removed because exceptions might occur both from the control flow
// and for signaling problems. Until those are handled separately, we can't use
// QMessageBox to show the exception, since those can be thrown from different threads.

#define HANDLE_EXCEPTION(e)                                                                                            \
//if (GetScopyApplicationInstance()->getDebugMode()) { \
//	auto me = dynamic_cast<libm2k::m2k_exception *>(&e); \
//	QString str; \
//	if (me) { \
//		str = QString("Exception %1\ne.type() - %2\ne.what()  - %3\ne.iioCode() - %4\nthrown from %5:%6\ncaught in %7:%8\nScopy git tag %9\n") \
//		.arg("m2k_exception").arg(me->type()).arg(me->what()).arg(me->iioCode()).arg(QString::fromStdString(me->file())).arg(QString::number(me->line())).arg(__FILE__).arg(__LINE__).arg(SCOPY_VERSION_GIT); \
//	} else { \
//		str = QString("Exception  %1\ncaught in %2:%3\nScopy git tag %4\n").arg(e.what()).arg(__FILE__).arg(__LINE__).arg(SCOPY_VERSION_GIT); \
//	} \
//	if (WriteScopyMinidump()) { \
//		str = str + "Created minidump."; \
//	} \
//	qDebug() << str << "\n"; \
} \

#endif // SCOPYEXCEPTIONHANDLER_H
