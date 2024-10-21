/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#include "bitfielddetailedwidgetfactory.hpp"

#include "bitfielddetailedwidget.hpp"
#include "bitfieldmodel.hpp"

#include <regmapstylehelper.hpp>

using namespace scopy;
using namespace regmap;

BitFieldDetailedWidgetFactory::BitFieldDetailedWidgetFactory(QObject *parent)
	: QObject{parent}
{}

BitFieldDetailedWidget *BitFieldDetailedWidgetFactory::buildWidget(BitFieldModel *model)
{
	BitFieldDetailedWidget *bfdw = new BitFieldDetailedWidget(
		model->getName(), model->getAccess(), model->getDefaultValue(), model->getDescription(),
		model->getWidth(), model->getNotes(), model->getRegOffset(), model->getOptions());
	RegmapStyleHelper::BitFieldDetailedWidgetStyle(bfdw);
	return bfdw;
}
