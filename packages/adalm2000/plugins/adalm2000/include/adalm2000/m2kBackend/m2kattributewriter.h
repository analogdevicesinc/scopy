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

#include "m2kcallcommand.h"

#include "component/attributewriter.h"
#include "component/capabilityexecutor.h"

#include <QString>

#include <functional>
#include <utility>

namespace scopy {
class ICmdExecutor;
}

namespace scopy::adalm2000 {

class M2kAttributeWriter : public component::AttributeWriter
{
	Q_OBJECT
public:
	using Setter = std::function<void(const QString &)>;

	M2kAttributeWriter(void *resource, Setter setter, scopy::ICmdExecutor *executor, QObject *parent = nullptr)
		: component::AttributeWriter(parent)
		, m_resource(resource)
		, m_setter(std::move(setter))
		, m_executor(executor)
	{}

	QCoro::Task<CommandResponse<void>> writeAsync(const QString &value) override
	{
		auto setter = m_setter;
		auto *cmd = new M2kCallCommand<void>(m_resource, [setter, value]() { setter(value); });
		return component::runCommand(
			m_executor, cmd, [this](const scopy::Result<void> &) { Q_EMIT writeSucceeded(); },
			[this](const scopy::Error &error) { Q_EMIT writeFailed(error); });
	}

private:
	void *m_resource;
	Setter m_setter;
	scopy::ICmdExecutor *m_executor;
};

} // namespace scopy::adalm2000
