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

#ifndef QIQCONTROLLER_H
#define QIQCONTROLLER_H

#include "qiqcontroller/qiqplotinfo.h"
#include "cmdhandler.h"
#include "outputconfig.h"
#include "outputinfo.h"
#include "runresults.h"
#include <QObject>

namespace scopy::qiqplugin {

class QIQController : public QObject
{
	Q_OBJECT

private:
	CmdHandler *m_cmdHandler;
	// QIQConfiguration *m_qiqConfig;

public:
	// Constructor
	explicit QIQController(CommandFormat *cmdFormat, QObject *parent = nullptr);

	// Destructor
	~QIQController() = default;

	// Configuration methods
	void setCmdFormat(CommandFormat *cmdFormat);
	void configureInput(const InputConfig &config);
	void configureAnalysis(const QString &type, const QVariantMap &config);
	void configureOutput(const OutputConfig &config);

	// Control methods
	void runAnalysis();
	void getAnalysisTypes();
	void getAnalysisInfo(QString type);
	// QIQConfiguration getCurrentConfig();
	bool isReady();

Q_SIGNALS:
	void inputConfigured(const InputConfig &config);
	// maybe plot info ??
	void analysisConfigured(const QString &type, const QVariantMap &config, const OutputInfo &outputInfo);
	void analysisInfo(const QString &type, const QVariantMap &param, const OutputInfo &outputInfo,
			  const QList<QIQPlotInfo> plotInfo); // plot info
	void outputConfigured(const OutputConfig &config);
	void processDataStarted();
	void processDataCompleted(RunResults results);
	void analysisTypesReceived(QStringList types);
	void errorOccurred(QString operation, QString error);
	void configurationChanged();

private Q_SLOTS:
	// Communication error handling
	void onCommunicationError(QString error);

	// Response handlers from CommandFormat
	void onProcessFinished(int exitCode);
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
};

} // namespace scopy::qiqplugin

#endif // QIQCONTROLLER_H
