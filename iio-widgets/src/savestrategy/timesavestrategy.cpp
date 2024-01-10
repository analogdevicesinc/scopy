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

#include "savestrategy/timesavestrategy.h"
#include <gui/stylehelper.h>
#include <QLoggingCategory>
#include <utility>

using namespace scopy;

Q_LOGGING_CATEGORY(CAT_TIME_DATA_STRATEGY, "TimeSaveStrategy")

TimerSaveStrategy::TimerSaveStrategy(IIOWidgetFactoryRecipe recipe, QWidget *parent)
	: QWidget(parent)
	, m_recipe(recipe)
	, m_progressBar(new SmallProgressBar(nullptr))
{
	connect(m_progressBar, &SmallProgressBar::progressFinished, this, &TimerSaveStrategy::writeData);
}

QWidget *TimerSaveStrategy::ui() { return m_progressBar; }

bool TimerSaveStrategy::isValid()
{
	if(m_recipe.channel != nullptr && m_recipe.data != "") {
		return true;
	}
	return false;
}

void TimerSaveStrategy::receiveData(QString data)
{
	qDebug(CAT_TIME_DATA_STRATEGY) << "Saving data" << data << "and restarting timer";
	m_lastData = data;
	m_progressBar->startProgress();
}

void TimerSaveStrategy::writeData()
{
	Q_EMIT saveData(m_lastData);
	qCritical(CAT_TIME_DATA_STRATEGY) << "Writing late" << m_lastData << "to" << m_recipe.data;
}

#include "moc_timesavestrategy.cpp"
