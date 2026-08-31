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

#include <QString>
#include <QVector>

namespace scopy::iio {

struct ContextHandle
{
	void *ptr = nullptr;
};
struct DeviceHandle
{
	void *ptr = nullptr;
};
struct ChannelHandle
{
	void *ptr = nullptr;
};
struct AttrHandle
{
	void *ptr = nullptr;
};
struct BufferHandle
{
	void *ptr = nullptr;
};
struct BlockHandle
{
	void *ptr = nullptr;
};
struct EventStreamHandle
{
	void *ptr = nullptr;
};
struct ScanHandle
{
	void *ptr = nullptr;
};
struct ChannelsMaskHandle
{
	void *ptr = nullptr;
};

// I changed the plan, and I removed this enum class from here.
// Instead, i added a new enum class in the controller header (named BackendKind)
// This should be removed!
enum class LibiioVersion
{
	V0,
	V1,
	Default
};

struct ContextParams
{
	int timeoutMs = 0; // 0 = backend default; -1 = infinite; INT_MIN = non-blocking
			   // Additional params (out, err, logLevel, stderrLevel, timestampLevel, flags)
			   // can be added here when needed — map to iio_context_params fields in v1contextops.cpp;
			   // v0 ignores anything beyond timeoutMs.
};

struct ScanResult
{
	QString uri;
	QString description;
};

struct ContextVersion
{
	unsigned int major = 0;
	unsigned int minor = 0;
	QString gitTag;
};

} // namespace scopy::iio
