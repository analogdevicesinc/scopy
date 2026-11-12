/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#include "component/regmap/registerstrategies.h"

#include "component/backends/file/regmap/fileregisterreadstrategy.h"
#include "component/backends/file/regmap/fileregisterwritestrategy.h"
#include "component/backends/iio/iioregisterreader.h"
#include "component/backends/iio/iioregisterwriter.h"
#include "component/backends/iio/regmap/iioregisterreadstrategy.h"
#include "component/backends/iio/regmap/iioregisterwritestrategy.h"
#include "component/controller.h"
#include "component/device.h"

using namespace scopy::component;

namespace {
template <typename Strategy>
Strategy *findByAddressSpace(QObject *owner, uint32_t addressSpace)
{
	for(Strategy *s : owner->findChildren<Strategy *>(Qt::FindDirectChildrenOnly)) {
		if(s->addressSpace() == addressSpace) {
			return s;
		}
	}
	return nullptr;
}

template <typename Strategy>
Strategy *findByPath(QObject *owner, const QString &path)
{
	for(Strategy *s : owner->findChildren<Strategy *>(Qt::FindDirectChildrenOnly)) {
		if(s->path() == path) {
			return s;
		}
	}
	return nullptr;
}
} // namespace

RegisterStrategies scopy::component::registerStrategies(Device *device, uint32_t addressSpace)
{
	if(!device) {
		return {};
	}
	auto *read = findByAddressSpace<iio::IIORegisterReadStrategy>(device, addressSpace);
	auto *write = findByAddressSpace<iio::IIORegisterWriteStrategy>(device, addressSpace);

	// a new address space gets its own pair, kept for the next caller
	if(!read) {
		if(auto *reader = device->findChild<iio::IIORegisterReader *>(QString(), Qt::FindDirectChildrenOnly)) {
			read = new iio::IIORegisterReadStrategy(reader, addressSpace, device);
		}
	}
	if(!write) {
		if(auto *writer = device->findChild<iio::IIORegisterWriter *>(QString(), Qt::FindDirectChildrenOnly)) {
			write = new iio::IIORegisterWriteStrategy(writer, addressSpace, device);
		}
	}
	return {read, write};
}

RegisterStrategies scopy::component::fileRegisterStrategies(const QString &path)
{
	if(path.isEmpty()) {
		return {};
	}
	// there is no Device for a file: the controller owns them, until it goes
	Controller *owner = Controller::GetInstance();
	auto *read = findByPath<file::FileRegisterReadStrategy>(owner, path);
	if(!read) {
		read = new file::FileRegisterReadStrategy(path, owner);
	}
	auto *write = findByPath<file::FileRegisterWriteStrategy>(owner, path);
	if(!write) {
		write = new file::FileRegisterWriteStrategy(path, owner);
	}
	return {read, write};
}
