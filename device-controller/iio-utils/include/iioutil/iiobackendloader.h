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

#pragma once

#include "iioutil/handles.h"

#include <QMap>
#include <QMutex>

namespace scopy::iio {

class IBackend;

class IIOBackendLoader
{
public:
	static IIOBackendLoader *instance();

	IBackend *backend(LibiioVersion requested);

	bool isLoaded(LibiioVersion v) const;

private:
	IIOBackendLoader() = default;
	~IIOBackendLoader();

	// Loads the plugin for a concrete version (V0/V1) and caches it. Returns the
	// backend, or nullptr on failure. Caller holds m_mutex.
	IBackend *ensureLoaded(LibiioVersion v);
	// Resolves Default to a concrete version, loading if needed. Caller holds m_mutex.
	IBackend *resolve(LibiioVersion requested);

	mutable QMutex m_mutex;
	QMap<LibiioVersion, void *> m_dlHandles;    // V0 / V1 dlopen handles
	QMap<LibiioVersion, IBackend *> m_backends; // V0 / V1 backend instances
};

} // namespace scopy::iio
