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
 */

#include "savestrategy/externalsavestrategy.h"

#include <gui/stylehelper.h>
#include <QLoggingCategory>

using namespace scopy;

Q_LOGGING_CATEGORY(CAT_ATTR_SAVE_STRATEGY, "AttrSaveStrategy")

ExternalSaveStrategy::ExternalSaveStrategy(IIOWidgetFactoryRecipe recipe, QWidget *parent)
	: QWidget(parent)
	, m_recipe(recipe)
	, m_ui(new QPushButton("Write"))
	, m_dataReceived(false)
{
	StyleHelper::SmallBlueButton(m_ui, "SaveButton" + m_recipe.data);
	connect(m_ui, &QPushButton::clicked, this, &ExternalSaveStrategy::writeData);
}

ExternalSaveStrategy::~ExternalSaveStrategy() { m_ui->deleteLater(); }

QWidget *ExternalSaveStrategy::ui() { return m_ui; }

bool ExternalSaveStrategy::isValid()
{
	if(m_recipe.channel != nullptr && m_recipe.data != "") {
		return true;
	}
	return false;
}

void ExternalSaveStrategy::receiveData(QString data)
{
	qDebug(CAT_ATTR_SAVE_STRATEGY) << "received" << data;
	m_data = data;
	m_dataReceived = true;
	Q_EMIT receivedData();
}

void ExternalSaveStrategy::writeData()
{
	if(m_dataReceived) {
		Q_EMIT saveData(m_data);
	}
	qCritical(CAT_ATTR_SAVE_STRATEGY) << "Writing from external" << m_data << "to" << m_recipe.data;
}

#include "moc_externalsavestrategy.cpp"
