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

#include "savestrategy/instantsavestrategy.h"
#include <gui/stylehelper.h>
#include <QLoggingCategory>
#include <utility>

using namespace scopy;

Q_LOGGING_CATEGORY(CAT_INSTANT_DATA_STRATEGY, "InstantDataSaveStrategy")

InstantSaveStrategy::InstantSaveStrategy(IIOWidgetFactoryRecipe recipe, QWidget *parent)
	: QWidget(parent)
	, m_recipe(recipe)
{}

QWidget *InstantSaveStrategy::ui() { return nullptr; }

bool InstantSaveStrategy::isValid()
{
	if(m_recipe.channel != nullptr && m_recipe.data != "") {
		return true;
	}
	return false;
}

void InstantSaveStrategy::receiveData(QString data)
{
	qCritical(CAT_INSTANT_DATA_STRATEGY) << "Instantly writing" << data << "to" << m_recipe.data;
	Q_EMIT saveData(data);
}

#include "moc_instantsavestrategy.cpp"
