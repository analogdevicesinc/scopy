/*
 * Copyright (c) 2021 Analog Devices Inc.
 *
 * This file is part of iio-emu
 * (see http://www.github.com/analogdevicesinc/iio-emu).
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

#include "factory_ops.hpp"
#include "iiod/adalm2000/adalm2000_ops.hpp"

using namespace iio_emu;

AbstractOps *FactoryOps::getOps(const char *type)
{
	AbstractOps *iiodOpsAbstract;

	if (!strncmp(type, "ADALM2000", sizeof("ADALM2000")-1)) {
		iiodOpsAbstract = new Adalm2000Ops();
	} else {
		return nullptr;
	}
	return iiodOpsAbstract;
}
