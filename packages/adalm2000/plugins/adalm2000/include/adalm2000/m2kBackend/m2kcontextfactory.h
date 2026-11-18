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
 */

#pragma once

#include "component/contextbackend.h"

namespace scopy::adalm2000 {

class M2kContextFactory : public component::ContextBackend
{
public:
	static constexpr const char *ID = "m2k";

	component::BackendId id() const override { return ID; }
	component::ConnectionKind connectionKind() const override;
	bool compatible(const component::Connection &c) const override;
	component::Context *create(component::Connection &c) override;
	void destroy(component::Context *ctx) override;
};

} // namespace scopy::adalm2000
