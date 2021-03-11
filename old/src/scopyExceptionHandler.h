#ifndef SCOPYEXCEPTIONHANDLER_H
#define SCOPYEXCEPTIONHANDLER_H

#include <QMessageBox>
#include <QApplication>
#include <config.h>
#include <libm2k/m2kexceptions.hpp>
#include <scopyApplication.hpp>
#include <QThread>

using namespace adiscope;

#define HANDLE_EXCEPTION(e) \
if (GetScopyApplicationInstance()->getDebugMode()) { \
	auto me = dynamic_cast<libm2k::m2k_exception *>(&e); \
	auto handleLambda = [me, e](){ \
		QMessageBox msg; \
		QString str; \
		if (me) { \
			str = QString("Exception %1\ne.type() - %2\ne.what()  - %3\ne.iioCode() - %4\nthrown from %5:%6\ncaught in %7:%8\nScopy git tag %9\n") \
			.arg("m2k_exception").arg(me->type()).arg(me->what()).arg(me->iioCode()).arg(QString::fromStdString(me->file())).arg(QString::number(me->line())).arg(__FILE__).arg(__LINE__).arg(SCOPY_VERSION_GIT); \
		} else { \
			str = QString("Exception  %1\ncaught in %2:%3\nScopy git tag %4\n").arg(e.what()).arg(__FILE__).arg(__LINE__).arg(SCOPY_VERSION_GIT); \
		} \
		if (WriteScopyMinidump()) { \
			str = str + "Created minidump."; \
		} \
		msg.setText(str); \
		msg.exec(); \
	}; \
	if (GetScopyApplicationInstance()->thread() != QThread::currentThread()) { \
		QMetaObject::invokeMethod(GetScopyApplicationInstance(), handleLambda, Qt::BlockingQueuedConnection); \
	} else { \
		handleLambda(); \
	} \
} \

#endif // SCOPYEXCEPTIONHANDLER_H
