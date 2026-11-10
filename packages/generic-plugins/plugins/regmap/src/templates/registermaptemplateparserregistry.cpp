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

#include "registermaptemplateparserregistry.hpp"

#include "iregistermaptemplateparser.hpp"
#include "jsontemplateparser.hpp"
#include "xmltemplateparser.hpp"

using namespace scopy::regmap;

RegisterMapTemplateParserRegistry *RegisterMapTemplateParserRegistry::instance()
{
	static RegisterMapTemplateParserRegistry registry;
	return &registry;
}

RegisterMapTemplateParserRegistry::RegisterMapTemplateParserRegistry()
{
	registerParser(new XmlTemplateParser());
	registerParser(new JsonTemplateParser());
}

RegisterMapTemplateParserRegistry::~RegisterMapTemplateParserRegistry() { qDeleteAll(m_parsers); }

void RegisterMapTemplateParserRegistry::registerParser(IRegisterMapTemplateParser *parser)
{
	if(parser) {
		m_parsers.append(parser);
	}
}

IRegisterMapTemplateParser *RegisterMapTemplateParserRegistry::parserFor(const QString &path) const
{
	IRegisterMapTemplateParser *match = nullptr;
	int matchLength = 0;
	for(IRegisterMapTemplateParser *parser : m_parsers) {
		for(const QString &suffix : parser->fileSuffixes()) {
			if(suffix.length() > matchLength && path.endsWith(suffix, Qt::CaseInsensitive)) {
				match = parser;
				matchLength = suffix.length();
			}
		}
	}
	return match;
}

QStringList RegisterMapTemplateParserRegistry::fileSuffixes() const
{
	QStringList suffixes;
	for(IRegisterMapTemplateParser *parser : m_parsers) {
		suffixes.append(parser->fileSuffixes());
	}
	return suffixes;
}

QString RegisterMapTemplateParserRegistry::suffixOf(const QString &fileName) const
{
	QString match;
	for(const QString &suffix : fileSuffixes()) {
		if(suffix.length() > match.length() && fileName.endsWith(suffix, Qt::CaseInsensitive)) {
			match = suffix;
		}
	}
	return match;
}
