#include <QApplication>
#include "scopyApplication.hpp"
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QDebug>

using namespace google_breakpad;

ScopyApplication::ScopyApplication(int& argc, char** argv) : QApplication(argc, argv) {}
QString ScopyApplication::initBreakPadHandler(QString crashDumpPath) {
	QString prevCrashDump="";
	QString appDir = crashDumpPath;
	QDir qd(appDir);
	if(qd.exists()) {
		qd.mkdir("crashdmp");
	}
	qd.cd("crashdmp");
	QStringList dumps= qd.entryList();
	for(QString dump : dumps)
	{
		if(dump=="." || dump == "..") continue;
		if(dump.startsWith("ScopyCrashDump"))
			continue;
		QString dumpFullPath = qd.path()+qd.separator()+dump;
		QFileInfo fi(dumpFullPath);
		QString dumpDateAndTime = fi.lastModified().toString(Qt::ISODate).replace("T","--").replace(":","-");
		prevCrashDump = qd.path()+qd.separator()+"ScopyCrashDump--"+dumpDateAndTime+".dmp";
		QFile::rename(dumpFullPath, prevCrashDump);


	}

	QByteArray ba = qd.path().toLocal8Bit();
	const char *appDirStr = ba.data();

	static size_t google_custom_count = 3;
	static google_breakpad::CustomInfoEntry google_custom_entries[] = {
		google_breakpad::CustomInfoEntry(L"prod", L"Scopy"),
		google_breakpad::CustomInfoEntry(L"ver", L"1.0.3"),
	};

	google_breakpad::CustomClientInfo custom_info = {google_custom_entries,
													 google_custom_count};

	handler->set_dump_path(qd.path().toStdWString());
	return prevCrashDump;
}

#ifdef CATCH_UNHANDLED_EXCEPTIONS
bool ScopyApplication::notify(QObject *receiver, QEvent *e) {

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
bool ScopyApplication::dumpCallback(const wchar_t* dump_path,
							const wchar_t* minidump_id,
							void* context,
							EXCEPTION_POINTERS* exinfo,
							MDRawAssertionInfo* assertion,
							bool succeeded) {
	printf("Dump path: %s\n",dump_path);
	return succeeded;
}

ExceptionHandler *ScopyApplication::getExceptionHandler() const
{
	return handler;
}

void ScopyApplication::setExceptionHandler(ExceptionHandler *value)
{
	handler = value;
}
#endif

#ifdef Q_OS_LINUX
static bool dumpCallback(const google_breakpad::MinidumpDescriptor& descriptor,
						 void* context, bool succeeded) {
	printf("Dump path: %s\n", descriptor.path());
	return succeeded;
}
#endif


