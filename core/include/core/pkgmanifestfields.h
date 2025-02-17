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
 *
 */

#ifndef PKGMANIFESTFIELDS_H
#define PKGMANIFESTFIELDS_H

#include <vector>

struct PkgManifest
{
	static constexpr auto PKG_ID = "id";
	static constexpr auto PKG_TITLE = "title";
	static constexpr auto PKG_CATEGORY = "category";
	static constexpr auto PKG_VERSION = "version";
	static constexpr auto PKG_DESCRIPTION = "description";
	static constexpr auto PKG_AUTHOR = "author";
	static constexpr auto PKG_LICENSE = "license";
	static constexpr auto PKG_BASE = "base-pkg";
	static constexpr auto PKG_SCOPY_COMPATIBILITY = "scopy_compatibility";

	inline static const std::vector<const char *> requiredFields = {
		PKG_ID, PKG_VERSION, PKG_LICENSE, PKG_AUTHOR, PKG_SCOPY_COMPATIBILITY, PKG_CATEGORY};
};

#endif // PKGMANIFESTFIELDS_H
