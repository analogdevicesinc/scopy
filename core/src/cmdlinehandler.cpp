#include "cmdlinehandler.h"

using namespace scopy;

FILE *CmdLineHandler::logFile_{nullptr};
bool CmdLineHandler::tempLogFile_{false};

int CmdLineHandler::handle(QCommandLineParser &parser, ScopyMainWindow_API &scopyApi)
{
	bool acceptLicense = parser.isSet("accept-license");
	if(acceptLicense) {
		scopyApi.acceptLicense();
	}

	QString param = parser.value("connect");
	if(!param.isEmpty()) {
		QString deviceID = "";
		deviceID = scopyApi.addDevice("", param);
		scopyApi.connectDevice(deviceID);
		QString tool = parser.value("tool");
		if(!tool.isEmpty()) {
			scopyApi.switchTool(deviceID, tool);
		}
	}

	bool keepRunning = parser.isSet("keep-running");
	if(keepRunning) {
		qInfo() << "keep-running option is only useful with a script!";
	}

	QString scriptPath = parser.value("script");
	if(!scriptPath.isEmpty()) {
		bool exitApp = !keepRunning;
		QMetaObject::invokeMethod(&scopyApi, "runScript", Qt::QueuedConnection, Q_ARG(QString, scriptPath),
					  Q_ARG(bool, exitApp));
	}
	return EXIT_SUCCESS;
}

void CmdLineHandler::withLogFileOption(QCommandLineParser &parser)
{
	QString fileName = parser.value("logfile");
	if(fileName.isEmpty()) {
		fileName = scopy::config::tempLogFilePath();
		tempLogFile_ = true;
	}
	logFile_ = fopen(fileName.toStdString().c_str(), "w");
	if(logFile_) {
		qInstallMessageHandler(logOutputHandler);
	}
}

void CmdLineHandler::closeLogFile()
{
	if(logFile_) {
		fclose(logFile_);
		logFile_ = nullptr;
		removeTempLogFile();
	}
}

void CmdLineHandler::removeTempLogFile()
{
	if(tempLogFile_) {
		QFile::remove(scopy::config::tempLogFilePath());
	}
}

void CmdLineHandler::logOutputHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	QString message = qFormatLogMessage(type, context, msg);
	fprintf(stderr, "%s\n", message.toStdString().c_str());
	if(logFile_) {
		fprintf(logFile_, "%s\n", message.toStdString().c_str());
	}
}
