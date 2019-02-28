#ifndef SCOPYAPPLICATION_H
#define SCOPYAPPLICATION_H

#include <QtGlobal>

#define __STDC_FORMAT_MACROS
#ifdef Q_OS_WIN
#include "breakpad/client/windows/handler/exception_handler.h"
#endif

#ifdef Q_OS_LINUX
#include "client/linux/handler/exception_handler.h"
#endif

//#define CATCH_UNHANDLED_EXCEPTIONS


class ScopyApplication final : public QApplication {

public:
	ScopyApplication(int& argc, char** argv);
	~ScopyApplication();
	QString initBreakPadHandler(QString crashDumpPath) ;
#ifdef CATCH_UNHANDLED_EXCEPTIONS
	virtual bool notify(QObject *receiver, QEvent *e) override ;
#endif

#ifdef Q_OS_WIN
	static bool dumpCallback(const wchar_t* dump_path,
							 const wchar_t* minidump_id,
							 void* context,
							 EXCEPTION_POINTERS* exinfo,
							 MDRawAssertionInfo* assertion,
							 bool succeeded) ;
#endif

#ifdef Q_OS_LINUX
	static bool dumpCallback(const google_breakpad::MinidumpDescriptor& descriptor,
							 void* context, bool succeeded) ;
#endif
	google_breakpad::ExceptionHandler *getExceptionHandler() const;
	void setExceptionHandler(google_breakpad::ExceptionHandler *value);

private:
	google_breakpad::ExceptionHandler *handler;
#ifdef Q_OS_LINUX
	google_breakpad::MinidumpDescriptor *descriptor;
#endif;
};
#endif
