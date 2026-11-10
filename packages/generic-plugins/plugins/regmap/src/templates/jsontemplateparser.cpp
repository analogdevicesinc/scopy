/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#include "jsontemplateparser.hpp"

#include "logging_categories.h"
#include "register/bitfield/bitfieldmodel.hpp"
#include "register/bitfield/bitfieldoption.hpp"
#include "register/registermodel.hpp"
#include "registermaptemplate.hpp"
#include "utils.hpp"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

using namespace scopy::regmap;

QString JsonTemplateParser::id() const { return "json"; }

QStringList JsonTemplateParser::fileSuffixes() const { return {".regmap.json"}; }

bool JsonTemplateParser::parse(const QString &path, RegisterMapTemplate *registerMapTemplate)
{
	QFile f(path);
	if(!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qWarning(CAT_REGMAP) << "Error while loading file" << path;
		return false;
	}

	QJsonParseError error;
	QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &error);
	f.close();
	if(error.error != QJsonParseError::NoError || !doc.isObject()) {
		qWarning(CAT_REGMAP) << "Invalid JSON template" << path << error.errorString();
		return false;
	}

	QMap<uint32_t, RegisterModel *> *regList = new QMap<uint32_t, RegisterModel *>();
	const QJsonArray registers = doc.object().value("registers").toArray();
	for(const QJsonValue &value : registers) {
		RegisterModel *reg = parseRegister(value.toObject(), registerMapTemplate);
		regList->insert(reg->getAddress(), reg);
	}

	if(regList->isEmpty()) {
		delete regList;
	} else {
		registerMapTemplate->setRegisterList(regList);
	}
	return true;
}

RegisterModel *JsonTemplateParser::parseRegister(const QJsonObject &reg, RegisterMapTemplate *registerMapTemplate)
{
	QVector<BitFieldModel *> *bitFields = new QVector<BitFieldModel *>();
	const QJsonArray bitFieldArray = reg.value("bitfields").toArray();
	for(const QJsonValue &value : bitFieldArray) {
		bitFields->push_back(parseBitField(value.toObject()));
	}

	// exists is false, as the XML parser gives it
	return new RegisterModel(reg.value("name").toString(),
				 Utils::convertQStringToUint32(reg.value("address").toString()),
				 reg.value("description").toString(), false, reg.value("width").toInt(), "", bitFields,
				 registerMapTemplate);
}

BitFieldModel *JsonTemplateParser::parseBitField(const QJsonObject &bitField)
{
	const int width = bitField.value("width").toInt();
	return new BitFieldModel(bitField.value("name").toString(), bitField.value("access").toString(),
				 Utils::convertQStringToUint32(bitField.value("default").toString()),
				 bitField.value("description").toString(), "", width, "", 0,
				 bitField.value("offset").toInt(), width, new QVector<BitFieldOption *>());
}
