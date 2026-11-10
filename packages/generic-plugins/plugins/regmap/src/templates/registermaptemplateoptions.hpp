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

#ifndef REGISTERMAPTEMPLATEOPTIONS_HPP
#define REGISTERMAPTEMPLATEOPTIONS_HPP

#include "jsonformatedelement.hpp"

namespace scopy::regmap {
// What regmap-config.json contributes to a template, whatever the template's file format
struct RegisterMapTemplateOptions
{
	bool useRegisterDescriptionAsName = false;
	bool useBitFieldDescriptionAsName = false;
	bool fillReservedBitFields = true;
	int bitsPerRow = 8;

	// a template that isn't in regmap-config.json (null) gets the defaults
	static RegisterMapTemplateOptions fromConfig(const JsonFormatedElement *config)
	{
		RegisterMapTemplateOptions options;
		if(config) {
			options.useRegisterDescriptionAsName = config->getUseRegisterNameAsDescription();
			options.useBitFieldDescriptionAsName = config->getUseBifieldNameAsDescription();
			options.bitsPerRow = config->getBitsPerRow();
		}
		return options;
	}
};
} // namespace scopy::regmap
#endif // REGISTERMAPTEMPLATEOPTIONS_HPP
