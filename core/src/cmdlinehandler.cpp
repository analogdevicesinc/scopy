#include "cmdlinehandler.h"

using namespace scopy;

FILE *CmdLineHandler::logFile_{nullptr};

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

	QString scriptPath = parser.value("script");
	if(!scriptPath.isEmpty()) {
		QFile file(scriptPath);
		if(!file.open(QFile::ReadOnly)) {
			qCritical() << "Unable to open script file";
			return EXIT_FAILURE;
		}

		QTextStream stream(&file);
		QString firstLine = stream.readLine();
		if(!firstLine.startsWith("#!"))
			stream.seek(0);

		QString content = stream.readAll();
		file.close();
		QMetaObject::invokeMethod(&scopyApi, "runScript", Qt::QueuedConnection, Q_ARG(QString, content),
					  Q_ARG(QString, scriptPath));
	}
	return EXIT_SUCCESS;
}

void CmdLineHandler::withLogFileOption(QCommandLineParser &parser)
{
	QString fileName = parser.value("logfile");
	if(!fileName.isEmpty()) {
		QString filePath = scopy::config::settingsFolderPath() + "/" + fileName;
		logFile_ = fopen(filePath.toStdString().c_str(), "w");
		if(logFile_) {
			qInstallMessageHandler(logOutputHandler);
		}
	}
}

void CmdLineHandler::closeLogFile()
{
	if(logFile_) {
		fclose(logFile_);
		logFile_ = nullptr;
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
