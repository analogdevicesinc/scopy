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

#include "resourcemanager.h"

#include <QApplication>
#include <QLoggingCategory>
Q_LOGGING_CATEGORY(CAT_RESOURCEMANAGER, "ResourceManager");

using namespace scopy;

ResourceManager *ResourceManager::pinstance_{nullptr};
ResourceManager::ResourceManager(QObject *parent)
	: QObject(parent)
{}

ResourceManager::~ResourceManager() {}

bool ResourceManager::open(QString resource, ResourceUser *res, bool force)
{
	ResourceManager *rm = ResourceManager::GetInstance();
	if(rm->map.contains(resource)) {
		if(force) {
			rm->map[resource]->stop();
			rm->map.insert(resource, res);
			return true;
		}
	} else {
		rm->map.insert(resource, res);
		return true;
	}
	return false;
}

void ResourceManager::close(QString resource)
{
	ResourceManager *rm = ResourceManager::GetInstance();
	if(rm->map.contains(resource)) {
		rm->map.remove(resource);
	}
}

ResourceManager *ResourceManager::GetInstance()
{
	if(pinstance_ == nullptr) {
		pinstance_ = new ResourceManager(QApplication::instance()); // singleton has the app as parent
	}
	return pinstance_;
}

#include "moc_resourcemanager.cpp"
