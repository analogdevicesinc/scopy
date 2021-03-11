/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "detachedwindowsmanager.h"

#include <QApplication>

using namespace adiscope;

DetachedWindowsManager& DetachedWindowsManager::getInstance()
{
	static DetachedWindowsManager INSTANCE;

	return INSTANCE;
}

DetachedWindow *DetachedWindowsManager::getWindow()
{
	// If the pool is empty create a new DetachedWindow
	// else get one from the pool
	if (pool.empty()) {
		return new DetachedWindow(nullptr);
	} else {
		auto window = pool.front();
		pool.pop_front();
		return window;
	}
}

void DetachedWindowsManager::returnWindow(DetachedWindow *window)
{
	// Return window to the pool and make sure it
	// has no central widget
	window->takeCentralWidget();
	pool.push_back(window);
}

DetachedWindowsManager::DetachedWindowsManager()
{
	// Create an initial pool of size equal to the number of
	// screens available to the user
	int initialSize = QApplication::screens().size();

	for (int i = 0; i < initialSize; ++i) {
		pool.push_back(new DetachedWindow(nullptr));
	}
}

DetachedWindowsManager::~DetachedWindowsManager()
{
	for (auto iterator : pool) {
		delete iterator;
	}

	pool.clear();
}
