/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#include "controller/cmdhandler.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_EXT_CMD_HANDLER, "ExtCmdHandler")
using namespace scopy::extprocplugin;

CmdHandler::CmdHandler(CommandFormat *cmdFormat, QObject *parent)
	: QObject(parent)
	, m_cmdFormat(cmdFormat)
	, m_cliProcess(new QProcess(this))
{
	connect(m_cliProcess, &QProcess::readyReadStandardOutput, this, &CmdHandler::onProcessReadyReadStandardOutput);
	connect(m_cliProcess, &QProcess::readyReadStandardError, this, &CmdHandler::onProcessReadyReadStandardError);
	connect(m_cliProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
		&CmdHandler::onProcessFinished);
	connect(m_cliProcess, &QProcess::errorOccurred, this, &CmdHandler::onProcessError);
}

void CmdHandler::setCmdFormat(CommandFormat *cmdFormat) { m_cmdFormat = cmdFormat; }

void CmdHandler::startProcess(const QString &program, const QStringList &arguments)
{
	if(m_cliProcess->state() != QProcess::NotRunning) {
		stopProcess();
	}

	m_cliProcess->start(program, arguments);
	m_cliProcess->waitForStarted();
}

void CmdHandler::stopProcess()
{
	if(m_cliProcess->state() != QProcess::NotRunning) {
		m_cliProcess->terminate();
		if(!m_cliProcess->waitForFinished(3000)) {
			m_cliProcess->kill();
		}
	}
}

bool CmdHandler::isProcessRunning() { return m_cliProcess->state() == QProcess::Running; }

void CmdHandler::sendCommand(const QString &cmd)
{
	if(isProcessRunning()) {
		m_cliProcess->write(cmd.toUtf8() + "\n");
		m_cliProcess->waitForBytesWritten();
	}
}

void CmdHandler::processData(QString &data)
{
	if(m_cmdFormat) {
		QVariantMap parsedData = m_cmdFormat->parseResponse(data);
		if(parsedData.isEmpty() || !parsedData.contains("status")) {
			qDebug(CAT_EXT_CMD_HANDLER) << "This is not a command response!";
			return;
		}
		Q_EMIT responseReceived(parsedData); // slot in CMDController
	}
}

void CmdHandler::onProcessReadyReadStandardOutput()
{
	QByteArray data = m_cliProcess->readAllStandardOutput();
	QString output = QString::fromUtf8(data);
	processData(output);
}

void CmdHandler::onProcessReadyReadStandardError()
{
	QByteArray data = m_cliProcess->readAllStandardError();
	QString error = QString::fromUtf8(data);
	Q_EMIT errorOccurred(error);
}

void CmdHandler::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	Q_EMIT processFinished(exitCode, exitStatus);
}

void CmdHandler::onProcessError(QProcess::ProcessError error)
{
	QString errorString = m_cliProcess->errorString();
	Q_EMIT errorOccurred(errorString);
}

CommandFormat *CmdHandler::cmdFormat() const { return m_cmdFormat; }
