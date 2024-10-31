/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

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

	QStringList scriptListPath = parser.values("script-list");
	if(!scriptListPath.isEmpty()) {
		bool exitApp = !keepRunning;
		QMetaObject::invokeMethod(&scopyApi, "runScriptList", Qt::QueuedConnection,
					  Q_ARG(QStringList, scriptListPath), Q_ARG(bool, exitApp));
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
