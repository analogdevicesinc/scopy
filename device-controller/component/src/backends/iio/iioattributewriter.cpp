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

#include "component/backends/iio/iioattributewriter.h"

#include "component/capabilityexecutor.h"
#include "core/icmdexecutor.h"
#include "iioutil/commands/attrwritecommand.h"

#include <qcoro/qcorofuture.h>

using namespace scopy;
using namespace scopy::component::iio;

IIOAttributeWriter::IIOAttributeWriter(scopy::iio::IAttrOps *ops, scopy::iio::AttrHandle handle,
				       scopy::ICmdExecutor *executor, QObject *parent)
	: AttributeWriter(parent)
	, m_ops(ops)
	, m_handle(handle)
	, m_executor(executor)
{}

QCoro::Task<CommandResponse<void>> IIOAttributeWriter::writeAsync(const QString &value)
{
	auto *cmd = new scopy::iio::AttrWriteCommand(m_ops, m_handle, value);
	return runCommand(
		m_executor, cmd, [this](const Result<void> &) { Q_EMIT writeSucceeded(); },
		[this](const scopy::Error &error) { Q_EMIT writeFailed(error); });
}
