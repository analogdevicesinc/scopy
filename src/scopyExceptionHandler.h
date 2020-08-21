#ifndef SCOPYEXCEPTIONHANDLER_H
#define SCOPYEXCEPTIONHANDLER_H

#include <QMessageBox>
#include <QApplication>
#include <config.h>
#include <libm2k/m2kexceptions.hpp>
#include <scopyApplication.hpp>

using namespace adiscope;

#define HANDLE_EXCEPTION(e) \
	if(GetScopyApplicationInstance()->getDebugMode()) { \
		QMessageBox msg; \
		QString str; \
		if(dynamic_cast<libm2k::m2k_exception*>(&e)) { \
		str = QString("Exception %1 - %2 %3 thrown from %2:%3 caught in %4:%5. Scopy git tag %6.") \
		.arg("m2k_exception").arg(e.type()).arg(e.what()).arg(QString::fromStdString(e.file())).arg(QString::number(e.line())).arg(__FILE__).arg(__LINE__).arg(SCOPY_VERSION_GIT); \
		} \
		else { str = QString("Exception  %1 - caught in %2:%3. Scopy git tag %4").arg(e.what()).arg(__FILE__).arg(__LINE__).arg(SCOPY_VERSION_GIT);} \
		if(WriteScopyMinidump()) str = str + "Created minidump.";\
		msg.setText(str);\
		msg.exec();\
	} \

#endif // SCOPYEXCEPTIONHANDLER_H
