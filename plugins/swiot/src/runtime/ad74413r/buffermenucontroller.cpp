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

using namespace scopy::swiot;

BufferMenuController::BufferMenuController(BufferMenuView* genericMenu, BufferMenuModel* model, int chnlIdx):
	m_chnlIdx(chnlIdx)
      ,m_genericMenu(genericMenu)
      ,m_model(model)
{}

BufferMenuController::~BufferMenuController()
{
	delete m_model;
	delete m_genericMenu;
}

void BufferMenuController::addMenuAttrValues(QMap<QString, QMap<QString, QStringList> > values)
{
	if (!values.empty()) {
		m_genericMenu->initAdvMenu(values);
	}
}

void BufferMenuController::createConnections()
{
	BufferMenu* advMenu = m_genericMenu->getAdvMenu();
	connect(advMenu, &BufferMenu::attrValuesChanged, this, &BufferMenuController::attributesChanged);
	connect(m_model, &BufferMenuModel::attrWritten, advMenu, &BufferMenu::onAttrWritten);
	connect(m_model, &BufferMenuModel::menuModelInitDone, this, &BufferMenuController::addMenuAttrValues);

	connect(advMenu, SIGNAL(broadcastThresholdReadForward(QString)), this, SIGNAL(broadcastThresholdReadForward(QString)));
	connect(this, SIGNAL(broadcastThresholdReadBackward(QString)), advMenu, SIGNAL(broadcastThresholdReadBackward(QString)));
	connect(this, &BufferMenuController::thresholdControlEnable, advMenu, &BufferMenu::thresholdControlEnable);
}

void BufferMenuController::attributesChanged(QString attrName, QString chnlType)
{
	BufferMenu* menu=m_genericMenu->getAdvMenu();
	QMap<QString, QMap<QString, QStringList>> attributes = menu->getAttrValues();
	m_model->updateChnlAttributes(attributes, attrName, chnlType);
}

int BufferMenuController::getChnlIdx()
{
	return m_chnlIdx;
}

