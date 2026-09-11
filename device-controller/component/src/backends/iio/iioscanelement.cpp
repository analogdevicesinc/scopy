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

#include "component/backends/iio/iioscanelement.h"

#include "core/icmdexecutor.h"
#include "iioutil/commands/chndisablecommand.h"
#include "iioutil/commands/chnenablecommand.h"
#include "iioutil/ichannelops.h"

#include <cerrno>
#include <qcoro/qcorofuture.h>

using namespace scopy;
using namespace scopy::component::iio;

IIOScanElement::IIOScanElement(scopy::iio::IChannelOps *ops, scopy::iio::ChannelHandle handle,
			       scopy::iio::ChannelsMaskHandle mask, scopy::ICmdExecutor *executor, QObject *parent)
	: QObject(parent)
	, m_ops(ops)
	, m_handle(handle)
	, m_mask(mask)
	, m_executor(executor)
{}

Command *IIOScanElement::makeCommand(bool en)
{
	return en ? static_cast<Command *>(new scopy::iio::ChnEnableCommand(m_ops, m_handle, m_mask))
		  : static_cast<Command *>(new scopy::iio::ChnDisableCommand(m_ops, m_handle, m_mask));
}

QCoro::Task<CommandResponse<void>> IIOScanElement::enableAsync(bool en)
{
	auto *cmd = makeCommand(en);
	const QUuid id = cmd->id();
	const bool was = m_ops->isEnabled(m_handle, m_mask);
	co_await m_executor->execute(cmd);
	cmd->deleteLater();

	const bool now = m_ops->isEnabled(m_handle, m_mask);
	if(now != was) {
		Q_EMIT enabledChanged(now);
	}
	if(now == en) {
		co_return CommandResponse<void>(id);
	}
	Unexpected err{Error{-EIO, QStringLiteral("channel mask did not reach requested state")}};
	Q_EMIT enableFailed(err.error);
	co_return CommandResponse<void>(id, std::move(err));
}

bool IIOScanElement::isEnabled() const { return m_ops->isEnabled(m_handle, m_mask); }
