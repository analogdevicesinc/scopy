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

#ifndef READPOLICY_H
#define READPOLICY_H

#include "scopy-dcdebugger_export.h"

#include <QString>
#include <QStringList>

namespace scopy::component {
class Attribute;
}

namespace scopy::dcdebugger {

// When selecting a node (or expanding a group) may read its attributes from the device.
class SCOPY_DCDEBUGGER_EXPORT ReadPolicy
{
public:
	static constexpr const char *PREF = "dcdebugger_read_on_selection";
	// Read what has no cached value, once.
	static constexpr const char *ONLY_UNCACHED = "Only when not cached";
	// Read every attribute each time its node is selected.
	static constexpr const char *EVERY_SELECTION = "On every selection";

	static void initPreferences();
	static QStringList options();
	static bool everySelection();

	// True when selecting should read `attr` now. In the default mode an attribute is read this way
	// when it has no cached value and was not read this way before (a failed read is not retried).
	static bool takeAutoRead(component::Attribute *attr);

	// Navigation that must stay passive (no read at all) holds one of these.
	class Suppress
	{
	public:
		Suppress();
		~Suppress();
		Suppress(const Suppress &) = delete;
		Suppress &operator=(const Suppress &) = delete;
	};
	static bool suppressed();
};

} // namespace scopy::dcdebugger

#endif // READPOLICY_H
