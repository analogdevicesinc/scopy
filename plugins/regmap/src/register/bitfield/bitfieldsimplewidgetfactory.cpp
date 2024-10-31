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

#include "bitfieldsimplewidgetfactory.hpp"

#include "bitfieldmodel.hpp"
#include "bitfieldsimplewidget.hpp"

#include <regmapstylehelper.hpp>

using namespace scopy;
using namespace regmap;

BitFieldSimpleWidgetFactory::BitFieldSimpleWidgetFactory(QObject *parent)
	: QObject{parent}
{}

BitFieldSimpleWidget *BitFieldSimpleWidgetFactory::buildWidget(BitFieldModel *model, int streach)
{
	BitFieldSimpleWidget *bfsw =
		new BitFieldSimpleWidget(model->getName(), model->getDefaultValue(), model->getDescription(),
					 model->getWidth(), model->getNotes(), model->getRegOffset(), streach);

	RegmapStyleHelper::BitFieldSimpleWidgetStyle(bfsw);

	return bfsw;
}
