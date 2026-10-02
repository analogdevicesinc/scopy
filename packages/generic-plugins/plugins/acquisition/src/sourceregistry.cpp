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

#include "sourceregistry.h"

using namespace scopy::adc;

AcqSourceRegistry &AcqSourceRegistry::instance()
{
	// Function-local, so it is constructed on the first add() rather than in an
	// unspecified position among the sources' own static initializers.
	static AcqSourceRegistry registry;
	return registry;
}

bool AcqSourceRegistry::add(AcqSourceFactory f)
{
	if(!f) {
		return false;
	}
	m_sources.append(std::move(f));
	return true;
}

const QList<AcqSourceFactory> &AcqSourceRegistry::all() const { return m_sources; }
