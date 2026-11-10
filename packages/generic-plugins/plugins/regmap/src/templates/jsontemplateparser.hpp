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

#ifndef JSONTEMPLATEPARSER_HPP
#define JSONTEMPLATEPARSER_HPP

#include "iregistermaptemplateparser.hpp"

class QJsonObject;

namespace scopy::regmap {
class BitFieldModel;
class RegisterModel;

// A mock JSON format, *.regmap.json:
// {"registers": [{"name", "address", "description", "width",
//                 "bitfields": [{"name", "offset", "width", "access", "default", "description"}]}]}
// address and default are hex text, offset is the bit position in the register
class JsonTemplateParser : public IRegisterMapTemplateParser
{
public:
	QString id() const override;
	QStringList fileSuffixes() const override;
	bool parse(const QString &path, RegisterMapTemplate *registerMapTemplate) override;

private:
	RegisterModel *parseRegister(const QJsonObject &reg, RegisterMapTemplate *registerMapTemplate);
	BitFieldModel *parseBitField(const QJsonObject &bitField);
};
} // namespace scopy::regmap
#endif // JSONTEMPLATEPARSER_HPP
