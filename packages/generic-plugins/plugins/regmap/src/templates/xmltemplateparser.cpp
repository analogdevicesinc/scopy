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

#include "xmltemplateparser.hpp"

#include "logging_categories.h"
#include "register/bitfield/bitfieldmodel.hpp"
#include "register/bitfield/bitfieldoption.hpp"
#include "register/registermodel.hpp"
#include "registermaptemplate.hpp"
#include "utils.hpp"

#include <QDomDocument>
#include <QFile>

using namespace scopy::regmap;

namespace {
QString childText(const QDomElement &element, const QString &tag)
{
	return element.firstChildElement(tag).firstChild().toText().data();
}
} // namespace

QString XmlTemplateParser::id() const { return "xml"; }

QStringList XmlTemplateParser::fileSuffixes() const { return {".xml"}; }

bool XmlTemplateParser::parse(const QString &path, RegisterMapTemplate *registerMapTemplate)
{
	QFile f(path);
	if(!f.open(QIODevice::ReadOnly)) {
		qWarning(CAT_REGMAP) << "Error while loading file" << path;
		return false;
	}
	QDomDocument xmlBOM;
	xmlBOM.setContent(&f);
	f.close();

	QMap<uint32_t, RegisterModel *> *regList = new QMap<uint32_t, RegisterModel *>();
	QDomElement reg = xmlBOM.documentElement().firstChildElement("Register");
	while(!reg.isNull()) {
		regList->insert(Utils::convertQStringToUint32(childText(reg, "Address")),
				parseRegister(reg, registerMapTemplate));
		reg = reg.nextSibling().toElement();
	}

	if(regList->isEmpty()) {
		delete regList;
	} else {
		registerMapTemplate->setRegisterList(regList);
	}
	return true;
}

RegisterModel *XmlTemplateParser::parseRegister(const QDomElement &reg, RegisterMapTemplate *registerMapTemplate)
{
	// "Exists" is read from the element, not from its text, as the original reader did, so it is always false
	return new RegisterModel(
		childText(reg, "Name"), Utils::convertQStringToUint32(childText(reg, "Address")),
		childText(reg, "Description"), reg.firstChildElement("Exists").toText().data().toUpper() == "TRUE",
		childText(reg, "Width").toInt(), childText(reg, "Notes"), parseBitFields(reg), registerMapTemplate);
}

QVector<BitFieldModel *> *XmlTemplateParser::parseBitFields(const QDomElement &reg)
{
	QVector<BitFieldModel *> *bitFieldsList = new QVector<BitFieldModel *>();
	QDomElement bf = reg.firstChildElement("BitFields").firstChild().toElement();
	while(!bf.isNull()) {
		bitFieldsList->push_back(parseBitField(bf));
		bf = bf.nextSibling().toElement();
	}
	return bitFieldsList;
}

BitFieldModel *XmlTemplateParser::parseBitField(const QDomElement &bitField)
{
	QVector<BitFieldOption *> *options = new QVector<BitFieldOption *>();
	QDomElement bfOption = bitField.firstChildElement("Options").firstChildElement("Option");
	while(!bfOption.isNull()) {
		options->push_back(
			new BitFieldOption(QString::number(Utils::convertQStringToUint32(childText(bfOption, "Value"))),
					   childText(bfOption, "Description")));
		bfOption = bfOption.nextSibling().toElement();
	}

	return new BitFieldModel(childText(bitField, "Name"), childText(bitField, "Access"),
				 Utils::convertQStringToUint32(childText(bitField, "DefaultValue")),
				 childText(bitField, "Description"), childText(bitField, "Visibility"),
				 childText(bitField, "Width").toInt(), childText(bitField, "Notes"),
				 childText(bitField, "BitOffset").toInt(), childText(bitField, "RegOffset").toInt(),
				 childText(bitField, "SliceWidth").toInt(), options);
}
