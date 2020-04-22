#include "scopyApplication.hpp"

#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>

#if BREAKPAD_HANDLER
using namespace google_breakpad;

ScopyApplication::ScopyApplication(int& argc, char** argv)
	: QApplication(argc, argv)
{}
QString ScopyApplication::initBreakPadHandler(QString crashDumpPath)
{
	QString prevCrashDump = "";
	QString appDir = crashDumpPath;
	QDir qd(appDir);
	if (qd.exists()) {
		qd.mkdir("crashdmp");
	}
	qd.cd("crashdmp");
	QStringList dumps = qd.entryList();
	for (QString dump : dumps) {
		if (dump == "." || dump == "..")
			continue;
		if (dump.startsWith("ScopyCrashDump"))
			continue;
		QString dumpFullPath = qd.path() + qd.separator() + dump;
		QFileInfo fi(dumpFullPath);
		QString dumpDateAndTime = fi.lastModified().toString(Qt::ISODate).replace("T", "--").replace(":", "-");
		prevCrashDump = qd.path() + qd.separator() + "ScopyCrashDump--" + dumpDateAndTime + ".dmp";
		QFile::rename(dumpFullPath, prevCrashDump);
	}

#ifdef Q_OS_LINUX
	descriptor = new google_breakpad::MinidumpDescriptor(qd.path().toStdString().c_str());
	handler->set_minidump_descriptor(*descriptor);
#endif
#ifdef Q_OS_WIN
	handler->set_dump_path(qd.path().toStdWString());
#endif
	return prevCrashDump;
}

ScopyApplication::~ScopyApplication()
{
#ifdef Q_OS_LINUX
	delete descriptor;
#endif
}

#ifdef CATCH_UNHANDLED_EXCEPTIONS
bool ScopyApplication::notify(QObject* receiver, QEvent* e)
{

	try {
		return QApplication::notify(receiver, e);
		// Handle the desired exception type
	} catch (...) {
		// Handle the rest
		handler->WriteMinidump();
		std::terminate();
	}
	return false;
}
#endif

#ifdef Q_OS_WIN
bool ScopyApplication::dumpCallback(const wchar_t* dump_path, const wchar_t* minidump_id, void* context,
				    EXCEPTION_POINTERS* exinfo, MDRawAssertionInfo* assertion, bool succeeded)
{
	printf("Dump path: %s\n", dump_path);
	return succeeded;
}
#endif
ExceptionHandler* ScopyApplication::getExceptionHandler() const { return handler; }

void ScopyApplication::setExceptionHandler(ExceptionHandler* value) { handler = value; }

#ifdef Q_OS_LINUX
bool ScopyApplication::dumpCallback(const google_breakpad::MinidumpDescriptor& descriptor, void* context,
				    bool succeeded)
{
	printf("Dump path: %s\n", descriptor.path());
	return succeeded;
}
#endif
#endif
