#include "qiqcontroller/qiqcontroller.h"
#include "mapkeys.h"
#include "qiqutils.h"
#include <QLoggingCategory>
#include <analysisconfig.h>

Q_LOGGING_CATEGORY(CAT_QIQ_CONTROLLER, "QIQController");

using namespace scopy::qiqplugin;

QIQController::QIQController(CommandFormat *cmdFormat, QObject *parent)
	: QObject(parent)
	, m_cmdHandler(nullptr)
// , m_qiqConfig(nullptr)
{
	m_cmdHandler = new CmdHandler(cmdFormat, this);

	connect(m_cmdHandler, &CmdHandler::responseReceived, this, &QIQController::onResponseReceived);
	connect(m_cmdHandler, &CmdHandler::errorOccurred, this, &QIQController::onCommunicationError);
	connect(m_cmdHandler, &CmdHandler::processFinished, this, &QIQController::onProcessFinished);
}

void QIQController::setCmdFormat(CommandFormat *cmdFormat) { m_cmdHandler->setCmdFormat(cmdFormat); }

void QIQController::configureInput(InputConfig &config)
{
	const QString stringCmd = m_cmdHandler->cmdFormat()->sendSetInputConfig(config);
	m_cmdHandler->sendCommand(stringCmd);
}

void QIQController::configureAnalysis(QString &type, QVariantMap &config)
{
	const QString stringCmd = m_cmdHandler->cmdFormat()->sendSetAnalysisConfig(type, config);
	m_cmdHandler->sendCommand(stringCmd);
}

void QIQController::configureOutput(OutputConfig &config)
{
	const QString stringCmd = m_cmdHandler->cmdFormat()->sendSetOutputConfig(config);
	m_cmdHandler->sendCommand(stringCmd);
}

void QIQController::runAnalysis()
{
	const QString stringCmd = m_cmdHandler->cmdFormat()->sendRun();
	m_cmdHandler->sendCommand(stringCmd);
	Q_EMIT processDataStarted();
}

void QIQController::getAnalysisTypes()
{
	const QString stringCmd = m_cmdHandler->cmdFormat()->sendGetAnalysisTypes();
	m_cmdHandler->sendCommand(stringCmd);
}

void QIQController::getAnalysisInfo(QString type)
{
	const QString stringCmd = m_cmdHandler->cmdFormat()->sendGetAnalysisInfo(type);
	m_cmdHandler->sendCommand(stringCmd);
}

// QIQConfiguration QIQController::getCurrentConfig()
// {
// 	// TODO: Return current configuration
// 	return QIQConfiguration();
// }

bool QIQController::isReady()
{
	// TODO: Check if ready for run command
	// input config must be send, output_config must be recieved, analysis response also
	return false;
}

void QIQController::onCommunicationError(QString error)
{
	qWarning(CAT_QIQ_CONTROLLER) << "Communication error:" << error;
}

void QIQController::onProcessFinished(int exitCode) {}

void QIQController::onResponseReceived(QVariantMap response)
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

void QIQController::handleSetInputConfigResponse(QVariantMap response)
{
	InputConfig inputConfig;
	QVariantMap config = response.value("config", {}).toMap();
	inputConfig.fromVariantMap(config);
	if(inputConfig.isValid()) {
		Q_EMIT inputConfigured(inputConfig);
	} else {
		qWarning(CAT_QIQ_CONTROLLER) << "Invalid input config response!";
	}
}

void QIQController::handleSetAnalysisConfigResponse(QVariantMap response)
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
		Q_EMIT analysisConfigured(type, config, outInfo);
	} else {
		qWarning(CAT_QIQ_CONTROLLER) << "Invalid analysis config response!";
	}
}

void QIQController::handleSetOutputConfigResponse(QVariantMap response)
{
	QVariantMap configMap = response["config"].toMap();
	OutputConfig outConfig;
	outConfig.fromVariantMap(configMap);
	if(outConfig.isValid()) {
		Q_EMIT outputConfigured(outConfig);
	} else {
		qWarning(CAT_QIQ_CONTROLLER) << "Invalid output config response!";
	}
}

void QIQController::handleRunResponse(QVariantMap response)
{
	RunResults runResult;
	runResult.setOutputFile(response.value(KeysRunResults::OUTPUT_FILE, "").toString());
	runResult.setResults(response.value(KeysRunResults::RESULTS, {}).toMap());
	runResult.setMeasurements(response.value(KeysRunResults::MEASUREMENTS, {}).toMap());

	if(runResult.isValid()) {
		Q_EMIT processDataCompleted(runResult);
	} else {
		qWarning(CAT_QIQ_CONTROLLER) << "Invalid run response!";
	}
}

void QIQController::handleGetAnalysisTypesResponse(QVariantMap response)
{
	QStringList types = response.value("supported_types", {}).toStringList();
	if(!types.isEmpty()) {
		Q_EMIT analysisTypesReceived(types);
	} else {
		qWarning(CAT_QIQ_CONTROLLER) << "Analysis types are not available!";
	}
}

void QIQController::handleGetAnalysisInfoResponse(QVariantMap response)
{
	QList<QIQPlotInfo> plotInfoList;
	QString type = response.value(KeysAnalysisInfo::ANALYSIS_TYPE, "").toString();
	QVariantMap parameters = response.value(KeysAnalysisInfo::PARAMETERS, {}).toMap();
	const QList<QVariant> plotInfoVar = response.value(KeysAnalysisInfo::PLOT_INFO, {}).toList();
	QVariantMap outInfoMap = response.value(KeysAnalysisInfo::OUT_INFO, {}).toMap();
	OutputInfo outInfo;
	outInfo.fromVariantMap(outInfoMap);

	for(const QVariant &variant : plotInfoVar) {
		QIQPlotInfo pInfo = QIQPlotInfo::fromVariantMap(variant.toMap());
		if(pInfo.isValid()) {
			plotInfoList.push_back(pInfo);
		}
	}

	if(outInfo.isValid() && !type.isEmpty() && !parameters.isEmpty() && !plotInfoList.isEmpty()) {
		Q_EMIT analysisInfo(type, parameters, outInfo, plotInfoList);
	} else {
		qWarning(CAT_QIQ_CONTROLLER) << "Analysis info is not available!";
	}
}

// void QIQController::updateConfiguration(QString responseType, QVariantMap data)
// {
// 	// TODO: Update configuration
// }

void QIQController::startOperationTimeout(QString operation)
{
	// TODO: Start operation timeout
}

void QIQController::onOperationTimeout()
{
	// TODO: Handle operation timeout
}
