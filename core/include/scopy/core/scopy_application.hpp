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
#ifndef SCOPYAPPLICATION_H
#define SCOPYAPPLICATION_H

#include "config.hpp"

#include <QApplication>
#include <QtGlobal>

#define __STDC_FORMAT_MACROS

#if BREAKPAD_HANDLER
#ifdef Q_OS_WIN
#include "breakpad/client/windows/handler/exception_handler.h"
#endif

#ifdef Q_OS_LINUX
#include "client/linux/handler/exception_handler.h"
#endif

//#define CATCH_UNHANDLED_EXCEPTIONS
#endif
#define GetScopyApplicationInstance() dynamic_cast<ScopyApplication*>(qApp)

#if BREAKPAD_HANDLER
#define WriteScopyMinidump() GetScopyApplicationInstance()->getExceptionHandler()->WriteMinidump()
#else
#define WriteScopyMinidump() (0)
#endif

class ScopyApplication final : public QApplication
{

private:
	bool debugMode;

public:
	ScopyApplication(int& argc, char** argv);
	bool getDebugMode() const;
	void setDebugMode(bool value);
#if BREAKPAD_HANDLER
	~ScopyApplication();
	QString initBreakPadHandler(QString crashDumpPath);
#ifdef CATCH_UNHANDLED_EXCEPTIONS
	virtual bool notify(QObject* receiver, QEvent* e) override;
#endif

#ifdef Q_OS_WIN
	static bool dumpCallback(const wchar_t* dump_path, const wchar_t* minidump_id, void* context,
				 EXCEPTION_POINTERS* exinfo, MDRawAssertionInfo* assertion, bool succeeded);
#endif

#ifdef Q_OS_LINUX
	static bool dumpCallback(const google_breakpad::MinidumpDescriptor& descriptor, void* context, bool succeeded);
#endif
	google_breakpad::ExceptionHandler* getExceptionHandler() const;
	void setExceptionHandler(google_breakpad::ExceptionHandler* value);

private:
	google_breakpad::ExceptionHandler* handler;
#ifdef Q_OS_LINUX
	google_breakpad::MinidumpDescriptor* descriptor;
#endif
#endif
};

#endif
