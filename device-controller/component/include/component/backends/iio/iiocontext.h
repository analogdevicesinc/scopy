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

#include "component/context.h"
#include "iioutil/handles.h"

namespace scopy::iio {
class IBackend;
}

namespace scopy::component::iio {

// IIO identity root. Carries the libiio context handle on top of the shared
// Context identity (uri/name/description/executor).
class IIOContext : public Context
{
	Q_OBJECT
public:
	explicit IIOContext(QObject *parent = nullptr)
		: Context(parent)
	{}

	~IIOContext() override;

	scopy::iio::ContextHandle handle() const { return m_handle; }
	void setHandle(scopy::iio::ContextHandle handle) { m_handle = handle; }

	// The backend this context was built with. The loader keeps every backend
	// resident for the whole process, so this is a non-owning pointer.
	scopy::iio::IBackend *backend() const { return m_backend; }
	void setBackend(scopy::iio::IBackend *backend) { m_backend = backend; }

private:
	scopy::iio::ContextHandle m_handle;
	scopy::iio::IBackend *m_backend = nullptr;
};

} // namespace scopy::component::iio
