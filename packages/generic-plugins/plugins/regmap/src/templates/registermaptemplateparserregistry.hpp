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

#ifndef REGISTERMAPTEMPLATEPARSERREGISTRY_HPP
#define REGISTERMAPTEMPLATEPARSERREGISTRY_HPP

#include <QList>
#include <QString>

namespace scopy::regmap {
class IRegisterMapTemplateParser;

// The template parsers regmap knows, picked by file name ending
class RegisterMapTemplateParserRegistry
{
public:
	static RegisterMapTemplateParserRegistry *instance();

	// takes ownership
	void registerParser(IRegisterMapTemplateParser *parser);
	// the parser with the longest matching suffix, or nullptr
	IRegisterMapTemplateParser *parserFor(const QString &path) const;

private:
	RegisterMapTemplateParserRegistry();
	~RegisterMapTemplateParserRegistry();

	QList<IRegisterMapTemplateParser *> m_parsers;
};
} // namespace scopy::regmap
#endif // REGISTERMAPTEMPLATEPARSERREGISTRY_HPP
