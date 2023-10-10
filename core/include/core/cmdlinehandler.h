#ifndef CMDLINEHANDLER_H
#define CMDLINEHANDLER_H

#include "scopy-core_export.h"
#include "scopymainwindow_api.h"

#include <QCommandLineParser>

namespace scopy {
class SCOPY_CORE_EXPORT CmdLineHandler
{
public:
	static int handle(QCommandLineParser &parser, ScopyMainWindow_API &scopyApi);
	static void withLogFileOption(QCommandLineParser &parser);
	static void closeLogFile();

private:
	static void logOutputHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
	static FILE *logFile_;
};
} // namespace scopy

#endif // CMDLINEHANDLER_H
