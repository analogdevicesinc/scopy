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

#include "controller/jsonformat.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <mapkeys.h>
#include <extprocutils.h>

using namespace scopy::extprocplugin;

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
