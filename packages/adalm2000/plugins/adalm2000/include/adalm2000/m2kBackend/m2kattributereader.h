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

#include "component/attributereader.h"
#include "component/capabilityexecutor.h"

#include <QByteArray>

#include <functional>
#include <utility>

namespace scopy {
class ICmdExecutor;
}

namespace scopy::adalm2000 {

class M2kAttributeReader : public component::AttributeReader
{
	Q_OBJECT
public:
	using Getter = std::function<QByteArray()>;

	M2kAttributeReader(void *resource, Getter getter, scopy::ICmdExecutor *executor, QObject *parent = nullptr)
		: component::AttributeReader(parent)
		, m_resource(resource)
		, m_getter(std::move(getter))
		, m_executor(executor)
	{}

	QCoro::Task<CommandResponse<QByteArray>> readAsync(size_t bytes = 4096) override
	{
		Q_UNUSED(bytes)
		auto *cmd = new M2kCallCommand<QByteArray>(m_resource, m_getter);
		return component::runCommand(
			m_executor, cmd, [this](scopy::Result<QByteArray> &r) { Q_EMIT readSucceeded(r); },
			[this](const scopy::Error &error) { Q_EMIT readFailed(error); });
	}

private:
	void *m_resource;
	Getter m_getter;
	scopy::ICmdExecutor *m_executor;
};

} // namespace scopy::adalm2000
