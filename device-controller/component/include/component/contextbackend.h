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

#include "component/connection.h"

namespace scopy::component {

class Context;

// A view: builds one component tree over an open Connection of the kind it declares. Views are
// flat and independent; several can share one connection (e.g. "iio" and "m2k" on one
// iio_context). Registered at runtime by id.
class ContextBackend
{
public:
	virtual ~ContextBackend() = default;

	virtual BackendId id() const = 0;
	virtual ConnectionKind connectionKind() const = 0;

	// Cheap check on the open, idle connection (context attributes, device names); no tree.
	// Must not call back into the Controller.
	virtual bool compatible(const Connection &c) const = 0;

	// 1 = this view needs a serial executor on the connection; -1 = no constraint.
	virtual int maxThreads() const { return -1; }

	// Builds the tree; drives I/O through c.executor(). The Controller parents the result to c.
	virtual Context *create(Connection &c) = 0;

	// Releases backend resources of a view before the Controller deletes it.
	virtual void destroy(Context *ctx) { Q_UNUSED(ctx); }
};

} // namespace scopy::component
