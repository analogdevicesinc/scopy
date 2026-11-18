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

#include "component/context.h"

namespace libm2k::context {
class M2k;
}

namespace scopy::adalm2000 {

class M2kContext : public component::Context
{
	Q_OBJECT
public:
	explicit M2kContext(QObject *parent = nullptr)
		: component::Context(parent)
	{}
	~M2kContext() override;

	// Stops the streams and releases the libm2k context. Idempotent; must run before the
	// connection's iio_context is destroyed.
	void close();

	libm2k::context::M2k *m2k() const { return m_m2k; }
	void setM2k(libm2k::context::M2k *m2k) { m_m2k = m2k; }

	// Set once the connection's ping fails; device I/O would only block on a dead link.
	bool connectionLost() const { return m_connectionLost; }
	void setConnectionLost() { m_connectionLost = true; }

private:
	libm2k::context::M2k *m_m2k = nullptr;
	bool m_connectionLost = false;
};

} // namespace scopy::adalm2000
