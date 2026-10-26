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
#include "iioutil/handles.h"

#include "component/component_export.h"

namespace scopy::iio {
class IBackend;
}

namespace scopy::component::iio {

// A libiio link: owns the iio_context handle and the ping. Views (IIOContext, M2kContext, ...)
// read the handle from here.
class COMPONENT_EXPORT IIOConnection : public Connection
{
	Q_OBJECT
public:
	struct Version
	{
		unsigned int major = 0;
		unsigned int minor = 0;
		QString gitTag;
	};

	IIOConnection(QString uri, scopy::iio::IBackend *backend, scopy::iio::ContextHandle handle,
		      QObject *parent = nullptr);
	~IIOConnection() override;

	scopy::iio::ContextHandle handle() const { return m_handle; }
	// The libiio backend linked into this build; resident for the whole process, non-owning.
	scopy::iio::IBackend *backend() const { return m_backend; }
	Version version() const { return m_version; }
	QString name() const { return m_name; }
	QString description() const { return m_description; }

	void start() override;

private:
	scopy::iio::IBackend *m_backend;
	scopy::iio::ContextHandle m_handle;
	Version m_version;
	QString m_name;
	QString m_description;
};

} // namespace scopy::component::iio
