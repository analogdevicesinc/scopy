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

#ifndef REGISTERMAPTEMPLATEPOSTPROCESSOR_HPP
#define REGISTERMAPTEMPLATEPOSTPROCESSOR_HPP

#include "registermaptemplateoptions.hpp"

namespace scopy::regmap {
class RegisterMapTemplate;
class RegisterModel;

// The template logic that doesn't depend on the file format. It runs once, after any parser.
class RegisterMapTemplatePostProcessor
{
public:
	static void apply(RegisterMapTemplate *registerMapTemplate, const RegisterMapTemplateOptions &options);

private:
	static void swapNameAndDescription(RegisterModel *reg, const RegisterMapTemplateOptions &options);
	static void fillReservedBitFields(RegisterModel *reg);
};
} // namespace scopy::regmap
#endif // REGISTERMAPTEMPLATEPOSTPROCESSOR_HPP
