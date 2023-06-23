/*
 * Copyright (c) 2023 Analog Devices Inc.
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
 */


#include "buffermenucontroller.h"
#include "qdebug.h"

using namespace scopy::swiot;

BufferMenuController::BufferMenuController(BufferMenuView* genericMenu, BufferMenuModel* model, int chnlIdx):
	m_chnlIdx(chnlIdx)
      ,m_genericMenu(genericMenu)
      ,m_model(model)
{}

BufferMenuController::~BufferMenuController()
{}

void BufferMenuController::addMenuAttrValues()
{
	//get all the attributes from the iio_channel
	QMap<QString, QStringList> contextValues = m_model->getChnlAttrValues();
	if (!contextValues.empty()) {
		m_genericMenu->initAdvMenu(contextValues);
	}
}

void BufferMenuController::createConnections()
{
	BufferMenu* advMenu = m_genericMenu->getAdvMenu();
	connect(advMenu, &BufferMenu::attrValuesChanged, this, &BufferMenuController::attributesChanged);
}

void BufferMenuController::attributesChanged(QString attrName)
{
	BufferMenu* menu=m_genericMenu->getAdvMenu();
	QMap<QString, QStringList> attributes = menu->getAttrValues();
	m_model->updateChnlAttributes(attributes, attrName);

}

int BufferMenuController::getChnlIdx()
{
	return m_chnlIdx;
}

