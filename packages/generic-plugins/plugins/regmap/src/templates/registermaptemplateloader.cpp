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

#include "registermaptemplateloader.hpp"

#include "iregistermaptemplateparser.hpp"
#include "logging_categories.h"
#include "registermaptemplate.hpp"
#include "registermaptemplateparserregistry.hpp"
#include "registermaptemplatepostprocessor.hpp"

using namespace scopy::regmap;

bool RegisterMapTemplateLoader::load(const QString &path, const RegisterMapTemplateOptions &options,
				     RegisterMapTemplate *registerMapTemplate)
{
	IRegisterMapTemplateParser *parser = RegisterMapTemplateParserRegistry::instance()->parserFor(path);
	if(!parser) {
		qWarning(CAT_REGMAP) << "No template parser for" << path;
		return false;
	}

	if(!parser->parse(path, registerMapTemplate)) {
		qWarning(CAT_REGMAP) << "Template parser" << parser->id() << "failed for" << path;
		return false;
	}

	RegisterMapTemplatePostProcessor::apply(registerMapTemplate, options);
	qDebug(CAT_REGMAP) << "Loaded template" << path << "with parser" << parser->id();
	return true;
}
