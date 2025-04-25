/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#include "iiowidgetgroup.h"

using namespace scopy;

IIOWidgetGroup::IIOWidgetGroup(bool singleTrigger, QObject *parent)
	: QObject(parent)
	, m_widgets({})
	, m_singleTrigger(singleTrigger)
	, m_trigger(nullptr)
{}

IIOWidgetGroup::~IIOWidgetGroup() { m_widgets.clear(); }

void IIOWidgetGroup::add(IIOWidget *widget, bool triggerPoint)
{
	m_widgets.push_back(widget);
	bool conn = !m_singleTrigger;
	if(m_singleTrigger && triggerPoint && !m_trigger) {
		m_trigger = widget;
		conn = true;
	}

	if(conn) {
		connect(dynamic_cast<QObject *>(widget->getDataStrategy()),
			SIGNAL(emitStatus(QDateTime, QString, QString, int, bool)), this,
			SLOT(handleStatusChanged(QDateTime, QString, QString, int, bool)), Qt::QueuedConnection);
	}
}

void IIOWidgetGroup::handleStatusChanged(QDateTime date, QString old, QString newD, int ret, bool readop)
{
	if(old != newD) {
		for(auto w : qAsConst(m_widgets)) {
			w->readAsync();
		}
	}
}

#include "moc_iiowidgetgroup.cpp"
