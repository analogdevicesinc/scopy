#include "qiqcontroller/jsonformat.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <mapkeys.h>
#include <qiqutils.h>

using namespace scopy::qiqplugin;

QString JsonFormat::sendSetInputConfig(const InputConfig &config)
{
	QVariantMap map;
	map.insert("config", config.toVariantMap());
	return buildJsonCommand(CommandNames::SET_INPUT_CONFIG, map);
}

QString JsonFormat::sendSetAnalysisConfig(const QString &type, const QVariantMap &config)
{
	QVariantMap map;
	map.insert(KeysAnalysisConfig::ANALYSIS_TYPE, type);
	map.insert("config", config);
	return buildJsonCommand(CommandNames::SET_ANALYSIS_CONFIG, map);
}

QString JsonFormat::sendSetOutputConfig(const OutputConfig &config)
{
	QVariantMap map;
	map.insert("config", config.toVariantMap());
	return buildJsonCommand(CommandNames::SET_OUTPUT_CONFIG, map);
}

QString JsonFormat::sendRun() { return buildJsonCommand(CommandNames::RUN, {}); }

QString JsonFormat::sendGetAnalysisTypes() { return buildJsonCommand(CommandNames::GET_ANALYSIS_TYPES, {}); }

QString JsonFormat::sendGetAnalysisInfo(const QString &type)
{
	QVariantMap map;
	map[KeysAnalysisConfig::ANALYSIS_TYPE] = type;
	return buildJsonCommand(CommandNames::GET_ANALYSIS_INFO, map);
}

QVariantMap JsonFormat::parseResponse(const QString &data) { return parseJsonResponse(data); }

QString JsonFormat::getProtocolName() { return PROTOCOL_NAME; }

QString JsonFormat::buildJsonCommand(const QString &cmd, const QVariantMap &param)
{
	QJsonObject root;
	const QStringList keys = param.keys();
	root["command"] = cmd;

	for(const QString &key : keys) {
		root[key] = QJsonValue::fromVariant(param[key]);
	}

	QJsonDocument doc(root);
	if(!doc.isObject()) {
		return {};
	}
	return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

QVariantMap JsonFormat::parseJsonResponse(const QString &response)
{
	QJsonDocument doc = QJsonDocument::fromJson(response.toUtf8());
	if(!doc.isObject()) {
		return {};
	}
	return doc.object().toVariantMap();
}
