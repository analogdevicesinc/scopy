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

#ifndef CMDCONTROLLER_H
#define CMDCONTROLLER_H

#include "controller/extprocplotinfo.h"
#include "procconfiguration.h"
#include "cmdhandler.h"
#include "outputconfig.h"
#include "outputinfo.h"
#include "runresults.h"
#include <QObject>

namespace scopy::extprocplugin {

class CMDController : public QObject
{
	Q_OBJECT
public:
	// Constructor
	explicit CMDController(CommandFormat *cmdFormat, QObject *parent = nullptr);

	// Destructor
	~CMDController() = default;

	// Configuration methods
	void setCmdFormat(CommandFormat *cmdFormat);
	void configureInput(const InputConfig &config);
	void configureAnalysis(const QString &type, const QVariantMap &config);
	void configureOutput(const OutputConfig &config);

	// Control methods
	void runAnalysis();
	void getAnalysisTypes();
	void getAnalysisInfo(QString type);
	ProcConfiguration *getCurrentConfig();
	bool isReady();

Q_SIGNALS:
	void inputConfigured(const InputConfig &config);
	void analysisConfigured(const QString &type, const QVariantMap &config, const OutputInfo &outputInfo);
	void analysisInfo(const QString &type, const QVariantMap &param, const OutputInfo &outputInfo,
			  const QList<ExtProcPlotInfo> plotInfo, QStringList measurementsLabels);
	void outputConfigured(const OutputConfig &config);
	void processDataStarted();
	void processDataCompleted(RunResults results);
	void analysisTypesReceived(QStringList types);
	void processFinished(int exitCode);
	void configurationChanged();

private Q_SLOTS:
	// Communication error handling
	void onCommunicationError(QString error);

	// Response handlers from CommandFormat
	void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
	void onResponseReceived(QVariantMap response);
	void handleSetInputConfigResponse(QVariantMap response);
	void handleSetAnalysisConfigResponse(QVariantMap response);
	void handleSetOutputConfigResponse(QVariantMap response);
	void handleRunResponse(QVariantMap response);
	void handleGetAnalysisTypesResponse(QVariantMap response);
	void handleGetAnalysisInfoResponse(QVariantMap response);

	// Configuration and timeout management
	// void updateConfiguration(QString responseType, QVariantMap data);
	void startOperationTimeout(QString operation);
	void onOperationTimeout();

private:
	QString buildCliPath(QString dirPath);
	QString findCli();

	CmdHandler *m_cmdHandler;
	ProcConfiguration *m_procConfig;
};

} // namespace scopy::extprocplugin

#endif // CMDCONTROLLER_H
