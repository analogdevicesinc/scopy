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

#include "component/backends/iio/iioctxbackend.h"

#include "component/backends/iio/iiocomponentbuilder.h"
#include "component/backends/iio/iioconnection.h"
#include "component/backends/iio/iiocontext.h"
#include "component/backends/iio/iioprovider.h"

using namespace scopy::component;
using namespace scopy::component::iio;

ConnectionKind IIOCtxBackend::connectionKind() const { return IIOProvider::KIND; }

bool IIOCtxBackend::compatible(const Connection &c) const { return c.as<IIOConnection>() != nullptr; }

Context *IIOCtxBackend::create(Connection &c)
{
	auto *conn = c.as<IIOConnection>();
	if(!conn) {
		return nullptr;
	}
	auto *ctx = new IIOContext;
	ctx->setName(conn->name());
	ctx->setDescription(conn->description());

	IIOComponentBuilder builder(conn->backend(), conn->handle());
	if(!builder.build(ctx, conn->executor())) {
		delete ctx;
		return nullptr;
	}
	return ctx;
}
