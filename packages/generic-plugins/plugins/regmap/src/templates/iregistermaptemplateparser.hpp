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

#ifndef IREGISTERMAPTEMPLATEPARSER_HPP
#define IREGISTERMAPTEMPLATEPARSER_HPP

#include <QString>
#include <QStringList>

namespace scopy::regmap {
class RegisterMapTemplate;

// Reads one register map file format into the template model. A parser only reports what the file says: the
// regmap-config.json options and the "Reserved" bitfields are applied afterwards by
// RegisterMapTemplatePostProcessor, the same way for every format.
class IRegisterMapTemplateParser
{
public:
	virtual ~IRegisterMapTemplateParser() = default;

	virtual QString id() const = 0;
	// file name endings this parser handles, e.g. ".xml"
	virtual QStringList fileSuffixes() const = 0;
	// false if the file can't be read
	virtual bool parse(const QString &path, RegisterMapTemplate *registerMapTemplate) = 0;
};
} // namespace scopy::regmap
#endif // IREGISTERMAPTEMPLATEPARSER_HPP
