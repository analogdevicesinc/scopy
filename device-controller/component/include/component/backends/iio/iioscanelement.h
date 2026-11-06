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

#include "core/result.h"
#include "iioutil/handles.h"

#include <QObject>
#include <QString>
#include <QUuid>

#include <qcoro/qcorotask.h>
#include "component/component_export.h"
#include "component/scanelement.h"

namespace scopy {
class Command;
class ICmdExecutor;
namespace iio {
class IChannelOps;
}
} // namespace scopy

namespace scopy::component::iio {

// IIO scan element: the channel's bit in its stream's libiio channels mask.
class COMPONENT_EXPORT IIOScanElement : public ScanElement
{
	Q_OBJECT
public:
	IIOScanElement(scopy::iio::IChannelOps *ops, scopy::iio::ChannelHandle handle,
		       scopy::iio::ChannelsMaskHandle mask, scopy::ICmdExecutor *executor, QObject *parent = nullptr);

	scopy::iio::ChannelHandle channelHandle() const { return m_handle; }

	QCoro::Task<CommandResponse<void>> enableAsync(bool en) override;
	bool isEnabled() const override;

private:
	scopy::Command *makeCommand(bool en);

	scopy::iio::IChannelOps *m_ops;
	scopy::iio::ChannelHandle m_handle;
	scopy::iio::ChannelsMaskHandle m_mask;
	scopy::ICmdExecutor *m_executor;
};

} // namespace scopy::component::iio
