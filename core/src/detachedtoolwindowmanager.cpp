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

#include "detachedtoolwindowmanager.h"

using namespace scopy;

DetachedToolWindowManager::DetachedToolWindowManager(QObject *parent)
	: QObject(parent)
{}

DetachedToolWindowManager::~DetachedToolWindowManager() {}

void DetachedToolWindowManager::add(QString id, ToolMenuEntry *tme)
{
	if(map.contains(id)) {
		delete map.take(id);
	}
	map.insert(id, new DetachedToolWindow(nullptr, tme));
}

bool DetachedToolWindowManager::remove(QString id)
{
	if(map.contains(id)) {
		delete map.take(id);
		return true;
	}
	return false;
}

void DetachedToolWindowManager::show(QString id)
{
	if(map.contains(id)) {
		map[id]->raise();
		map[id]->show();
	}
}

QWidget *DetachedToolWindowManager::getWidget(QString key)
{
	if(map.contains(key))
		return map[key];
	return nullptr;
}

bool DetachedToolWindowManager::contains(QString key) { return map.contains(key); }

#include "moc_detachedtoolwindowmanager.cpp"
