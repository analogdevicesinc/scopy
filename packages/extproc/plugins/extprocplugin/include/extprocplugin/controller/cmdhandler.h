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

#ifndef CMDHANDLER_H
#define CMDHANDLER_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QVariantMap>

#include "commandformat.h"

namespace scopy::extprocplugin {

class CmdHandler : public QObject
{
	Q_OBJECT
public:
	explicit CmdHandler(CommandFormat *cmdFormat, QObject *parent = nullptr);
	~CmdHandler() = default;

	void setCmdFormat(CommandFormat *cmdFormat);
	void startProcess(const QString &program, const QStringList &arguments);
	void stopProcess();
	bool isProcessRunning();
	void sendCommand(const QString &cmd);

	void processData(QString &data);

	CommandFormat *cmdFormat() const;

Q_SIGNALS:
	void responseReceived(QVariantMap response);
	void errorOccurred(QString error);
	void processFinished(int exitCode, QProcess::ExitStatus exitStatus);

private Q_SLOTS:
	void onProcessReadyReadStandardOutput();
	void onProcessReadyReadStandardError();
	void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
	void onProcessError(QProcess::ProcessError error);

private:
	CommandFormat *m_cmdFormat;
	QProcess *m_cliProcess;
	QString m_response;
};

} // namespace scopy::extprocplugin

#endif // CMDHANDLER_H
