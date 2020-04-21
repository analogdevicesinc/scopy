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

#ifndef APIOBJECTMANAGER_H
#define APIOBJECTMANAGER_H

#include "apiObject.hpp"

#include <vector>

namespace adiscope {
class ApiObjectManager {
public:
	void registerApiObject(ApiObject *apiObject);
	void unregisterApiObject(ApiObject *apiObject);

	void save(QSettings &settings);
	void load(QSettings &settings);

	static ApiObjectManager &getInstance();

private:
	ApiObjectManager();
	std::vector<ApiObject *> api_objects;
};
} // namespace adiscope

#endif // APIOBJECTMANAGER_H
