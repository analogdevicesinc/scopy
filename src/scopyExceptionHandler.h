#ifndef SCOPYEXCEPTIONHANDLER_H
#define SCOPYEXCEPTIONHANDLER_H

#include <QMessageBox>
#include <QApplication>
#include <config.h>
#include <scopyApplication.hpp>

using namespace adiscope;

#define HANDLE_EXCEPTION(e) \
	if(GetScopyApplicationInstance()->getDebugMode()) { \
		QMessageBox msg; \
		QString str = QString("Exception %1 thrown from %2:%3 caught in %4:%5. Scopy git tag %6.") \
		.arg("TestString").arg( __FILE__).arg(__LINE__).arg(__FILE__).arg(__LINE__).arg(SCOPY_VERSION_GIT); \
		if(GetScopyApplicationInstance()->getExceptionHandler()->WriteMinidump()) str = str + "Created minidump.";\
		msg.setText(str);\
		msg.exec();\
	} \

#endif // SCOPYEXCEPTIONHANDLER_H
