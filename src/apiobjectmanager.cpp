/*
 * Copyright 2018 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "apiobjectmanager.h"

#include <algorithm>

using namespace adiscope;

ApiObjectManager::ApiObjectManager() {}

void ApiObjectManager::registerApiObject(ApiObject *apiObject) { api_objects.push_back(apiObject); }

void ApiObjectManager::unregisterApiObject(ApiObject *apiObject) {
	api_objects.erase(std::find(api_objects.begin(), api_objects.end(), apiObject));
}

void ApiObjectManager::save(QSettings &settings) {
	for (auto &apiObject : api_objects) {
		apiObject->save(settings);
	}
}

void ApiObjectManager::load(QSettings &settings) {
	for (auto &apiObject : api_objects) {
		apiObject->load(settings);
	}
}

ApiObjectManager &ApiObjectManager::getInstance() {
	static ApiObjectManager Instance;

	return Instance;
}
