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

#ifndef XMLTEMPLATEPARSER_HPP
#define XMLTEMPLATEPARSER_HPP

#include "iregistermaptemplateparser.hpp"

#include <QVector>

class QDomElement;

namespace scopy::regmap {
class BitFieldModel;
class RegisterModel;

// The regmap XML format (<Register>, <BitFields>, <Options>)
class XmlTemplateParser : public IRegisterMapTemplateParser
{
public:
	QString id() const override;
	QStringList fileSuffixes() const override;
	bool parse(const QString &path, RegisterMapTemplate *registerMapTemplate) override;

private:
	RegisterModel *parseRegister(const QDomElement &reg, RegisterMapTemplate *registerMapTemplate);
	QVector<BitFieldModel *> *parseBitFields(const QDomElement &reg);
	BitFieldModel *parseBitField(const QDomElement &bitField);
};
} // namespace scopy::regmap
#endif // XMLTEMPLATEPARSER_HPP
