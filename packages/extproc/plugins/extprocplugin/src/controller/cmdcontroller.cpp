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

#include "controller/cmdcontroller.h"
#include "mapkeys.h"
#include "extprocutils.h"
#include <QLoggingCategory>
#include <analysisconfig.h>
#include <pluginbase/preferences.h>
#include <pluginbase/statusbarmanager.h>

Q_LOGGING_CATEGORY(CAT_CMD_CONTROLLER, "CMDController");

using namespace scopy::extprocplugin;

CMDController::CMDController(CommandFormat *cmdFormat, QObject *parent)
	: QObject(parent)
	, m_cmdHandler(nullptr)
	, m_procConfig(nullptr)
{
	m_cmdHandler = new CmdHandler(cmdFormat, this);
	m_procConfig = new ProcConfiguration(this);

	connect(m_cmdHandler, &CmdHandler::responseReceived, this, &CMDController::onResponseReceived);
	connect(m_cmdHandler, &CmdHandler::errorOccurred, this, &CMDController::onCommunicationError);
	connect(m_cmdHandler, &CmdHandler::processFinished, this, &CMDController::onProcessFinished);

	m_cmdHandler->startProcess(findCli(), {});
}

void CMDController::setCmdFormat(CommandFormat *cmdFormat) { m_cmdHandler->setCmdFormat(cmdFormat); }

void CMDController::configureInput(const InputConfig &config)
{
	const QString stringCmd = m_cmdHandler->cmdFormat()->sendSetInputConfig(config);
	m_cmdHandler->sendCommand(stringCmd);
}

void CMDController::configureAnalysis(const QString &type, const QVariantMap &config)
{
	const QString stringCmd = m_cmdHandler->cmdFormat()->sendSetAnalysisConfig(type, config);
	m_cmdHandler->sendCommand(stringCmd);
}

void CMDController::configureOutput(const OutputConfig &config)
{
	const QString stringCmd = m_cmdHandler->cmdFormat()->sendSetOutputConfig(config);
	m_cmdHandler->sendCommand(stringCmd);
}

void CMDController::runAnalysis()
{
	const QString stringCmd = m_cmdHandler->cmdFormat()->sendRun();
	m_cmdHandler->sendCommand(stringCmd);
	Q_EMIT processDataStarted();
}

void CMDController::getAnalysisTypes()
{
	const QString stringCmd = m_cmdHandler->cmdFormat()->sendGetAnalysisTypes();
	m_cmdHandler->sendCommand(stringCmd);
}

void CMDController::getAnalysisInfo(QString type)
{
	const QString stringCmd = m_cmdHandler->cmdFormat()->sendGetAnalysisInfo(type);
	m_cmdHandler->sendCommand(stringCmd);
}

ProcConfiguration *CMDController::getCurrentConfig() { return m_procConfig; }

bool CMDController::isReady() { return m_procConfig->isComplete(); }

void CMDController::onCommunicationError(QString error)
{
	qWarning(CAT_CMD_CONTROLLER) << "Communication error:" << error;
	StatusBarManager::pushMessage("CLI error: " + error, 3000);
}

void CMDController::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	Q_EMIT processFinished(exitCode);
	if(exitStatus == QProcess::CrashExit || exitCode == EXIT_FAILURE) {
		StatusBarManager::pushMessage("Trying to relaunch the CLI", 3000);
		m_cmdHandler->startProcess(findCli(), {});
	}
}

void CMDController::onResponseReceived(QVariantMap response)
{
	QString cmd = response["command"].toString();
	if(cmd == CommandNames::SET_INPUT_CONFIG) {
		handleSetInputConfigResponse(response);
	} else if(cmd == CommandNames::SET_ANALYSIS_CONFIG) {
		handleSetAnalysisConfigResponse(response);
	} else if(cmd == CommandNames::SET_OUTPUT_CONFIG) {
		handleSetOutputConfigResponse(response);
	} else if(cmd == CommandNames::RUN) {
		handleRunResponse(response);
	} else if(cmd == CommandNames::GET_ANALYSIS_TYPES) {
		handleGetAnalysisTypesResponse(response);
	} else if(cmd == CommandNames::GET_ANALYSIS_INFO) {
		handleGetAnalysisInfoResponse(response);
	} else {
		qWarning() << "Unknown command" << cmd;
	}
}

void CMDController::handleSetInputConfigResponse(QVariantMap response)
{
	InputConfig inputConfig;
	QVariantMap config = response.value("config", {}).toMap();
	inputConfig.fromVariantMap(config);
	if(inputConfig.isValid()) {
		m_procConfig->setInputConfig(inputConfig);
		Q_EMIT inputConfigured(inputConfig);
	} else {
		qWarning(CAT_CMD_CONTROLLER) << "Invalid input config response!";
	}
}

void CMDController::handleSetAnalysisConfigResponse(QVariantMap response)
{
	QString type = response.value(KeysAnalysisConfig::ANALYSIS_TYPE, "").toString();
	QVariantMap config = response.value(KeysAnalysisConfig::PARAMS, {}).toMap();
	OutputInfo outInfo;
	outInfo.fromVariantMap(response.value(KeysAnalysisConfig::OUT_INFO, {}).toMap());

	AnalysisConfig analysis;
	analysis.setAnalysisType(type);
	analysis.setOutputInfo(outInfo);
	analysis.setParams(config);

	if(analysis.isValid()) {
		m_procConfig->setAnalysis(analysis);
		Q_EMIT analysisConfigured(type, config, outInfo);
	} else {
		qWarning(CAT_CMD_CONTROLLER) << "Invalid analysis config response!";
	}
}

void CMDController::handleSetOutputConfigResponse(QVariantMap response)
{
	QVariantMap configMap = response["config"].toMap();
	OutputConfig outConfig;
	outConfig.fromVariantMap(configMap);
	if(outConfig.isValid()) {
		m_procConfig->setOutputConfig(outConfig);
		Q_EMIT outputConfigured(outConfig);
	} else {
		qWarning(CAT_CMD_CONTROLLER) << "Invalid output config response!";
	}
}

void CMDController::handleRunResponse(QVariantMap response)
{
	RunResults runResult;
	runResult.setOutputFile(response.value(KeysRunResults::OUTPUT_FILE, "").toString());
	runResult.setResults(response.value(KeysRunResults::RESULTS, {}).toMap());
	runResult.setMeasurements(response.value(KeysRunResults::MEASUREMENTS, {}).toMap());

	if(runResult.isValid()) {
		Q_EMIT processDataCompleted(runResult);
	} else {
		qWarning(CAT_CMD_CONTROLLER) << "Invalid run response!";
	}
}

void CMDController::handleGetAnalysisTypesResponse(QVariantMap response)
{
	QStringList types = response.value("supported_types", {}).toStringList();
	if(!types.isEmpty()) {
		Q_EMIT analysisTypesReceived(types);
	} else {
		qWarning(CAT_CMD_CONTROLLER) << "Analysis types are not available!";
	}
}

void CMDController::handleGetAnalysisInfoResponse(QVariantMap response)
{
	QList<ExtProcPlotInfo> plotInfoList;
	QString type = response.value(KeysAnalysisInfo::ANALYSIS_TYPE, "").toString();
	QVariantMap parameters = response.value(KeysAnalysisInfo::PARAMETERS, {}).toMap();
	const QList<QVariant> plotInfoVar = response.value(KeysAnalysisInfo::PLOT_INFO, {}).toList();
	QVariantMap outInfoMap = response.value(KeysAnalysisInfo::OUT_INFO, {}).toMap();
	QStringList measurements = response.value(KeysAnalysisInfo::MEASUREMENTS, {}).toStringList();
	OutputInfo outInfo;
	outInfo.fromVariantMap(outInfoMap);

	for(const QVariant &variant : plotInfoVar) {
		ExtProcPlotInfo pInfo = ExtProcPlotInfo::fromVariantMap(variant.toMap());
		if(pInfo.isValid()) {
			plotInfoList.push_back(pInfo);
		}
	}

	if(outInfo.isValid() && !type.isEmpty() && !parameters.isEmpty() && !plotInfoList.isEmpty()) {
		Q_EMIT analysisInfo(type, parameters, outInfo, plotInfoList, measurements);
	} else {
		qWarning(CAT_CMD_CONTROLLER) << "Analysis info is not available!";
	}
}

QString CMDController::findCli()
{
	QString cliPath = "";
	// The path from the preferences has the highest priority. The path must be added manually.
	cliPath = buildCliPath(Preferences::get("ext_cli_path").toString());
	if(!cliPath.isEmpty() && QFile::exists(cliPath)) {
		qInfo(CAT_CMD_CONTROLLER) << "cli path from preferences:" << cliPath;
		return cliPath;
	}
	// Search the cli next to scopy executable.
	cliPath = buildCliPath(scopy::config::executableFolderPath());
	if(!cliPath.isEmpty() && QFile::exists(cliPath)) {
		qInfo(CAT_CMD_CONTROLLER) << "cli path:" << cliPath;
		return cliPath;
	}
	StatusBarManager::pushMessage("CLI executable not found!");
	return "";
}

QString CMDController::buildCliPath(QString dirPath)
{
	QString cliPath = "";
	if(!dirPath.isEmpty()) {
		cliPath = dirPath + QDir::separator() + "cli_analyzer";
#ifdef WIN32
		cliPath += ".exe";
#endif
	}
	return cliPath;
}

// void CMDController::updateConfiguration(QString responseType, QVariantMap data)
// {
// 	// TODO: Update configuration
// }

void CMDController::startOperationTimeout(QString operation)
{
	// TODO: Start operation timeout
}

void CMDController::onOperationTimeout()
{
	// TODO: Handle operation timeout
}
