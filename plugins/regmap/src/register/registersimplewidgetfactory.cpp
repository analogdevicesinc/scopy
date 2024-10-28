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

#include "registersimplewidgetfactory.hpp"

#include "bitfield/bitfieldmodel.hpp"
#include "bitfield/bitfieldsimplewidget.hpp"
#include "bitfield/bitfieldsimplewidgetfactory.hpp"
#include "registermodel.hpp"
#include "registersimplewidget.hpp"
#include "registermaptemplate.hpp"

#include <regmapstylehelper.hpp>
#include <utils.hpp>

using namespace scopy;
using namespace regmap;

RegisterSimpleWidgetFactory::RegisterSimpleWidgetFactory(QObject *parent)
	: QObject{parent}
{}

RegisterSimpleWidget *RegisterSimpleWidgetFactory::buildWidget(RegisterModel *model)
{
	QVector<BitFieldSimpleWidget *> *bitFields = new QVector<BitFieldSimpleWidget *>;

	BitFieldSimpleWidgetFactory bitFieldSimpleWidgetFactory;
	int remaingSpaceOnRow = model->registerMapTemaplate()->bitsPerRow();
	for(int i = 0; i < model->getBitFields()->size(); ++i) {
		BitFieldModel *modelBitField = model->getBitFields()->at(i);
		int width = modelBitField->getWidth();
		while(width > remaingSpaceOnRow) {
			bitFields->push_back(bitFieldSimpleWidgetFactory.buildWidget(model->getBitFields()->at(i),
										     remaingSpaceOnRow));
			width = width - remaingSpaceOnRow;
			remaingSpaceOnRow = model->registerMapTemaplate()->bitsPerRow();
		}

		bitFields->push_back(bitFieldSimpleWidgetFactory.buildWidget(model->getBitFields()->at(i), width));
		remaingSpaceOnRow -= width;
		if(remaingSpaceOnRow == 0) {
			remaingSpaceOnRow = model->registerMapTemaplate()->bitsPerRow();
		}
	}

	RegisterSimpleWidget *rsw = new RegisterSimpleWidget(model, bitFields);

	RegmapStyleHelper::RegisterSimpleWidgetStyle(rsw);
	return rsw;
}
