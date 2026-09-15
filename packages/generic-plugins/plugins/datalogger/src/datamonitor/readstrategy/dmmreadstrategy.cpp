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

#include "datamonitor/readstrategy/dmmreadstrategy.hpp"
#include <QDate>
#include <QDebug>
#include <QwtDate>
#include <timemanager.hpp>

using namespace scopy;
using namespace datamonitor;

DMMReadStrategy::DMMReadStrategy(component::Attribute *readAttr)
	: m_readAttr(readAttr)
{}

void DMMReadStrategy::read()
{
	if(!m_readAttr || !m_readAttr->readCapability()) {
		return;
	}
	// Fire the coroutine read; the previous one (if any) is dropped when replaced.
	m_task = readTask();
}

QCoro::Task<void> DMMReadStrategy::readTask()
{
	auto response = co_await m_readAttr->readCapability()->readAsync();
	if(!response) {
		qDebug() << "device read error " << response.error().message;
		co_return;
	}

	double result = QString::fromUtf8(response.value()).toDouble();
	qDebug() << "dmm read success  ";

	auto &&timeTracker = TimeManager::GetInstance();
	double currentTime = QwtDate::toDouble(timeTracker->lastReadValue());

	Q_EMIT readDone(currentTime, result);
}
