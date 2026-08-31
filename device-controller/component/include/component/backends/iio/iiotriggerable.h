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

namespace scopy {
class ICmdExecutor;
namespace iio {
class IDeviceOps;
class SetTriggerCommand;
} // namespace iio
} // namespace scopy

namespace scopy::component::iio {

class IIOTrigger;

// Consumer side of the trigger relationship over one libiio device. IIO-only, no
// generic base. hasTrigger/assignedTriggerName are metadata (direct, no I/O);
// setTrigger/clearTrigger go through the executor via SetTriggerCommand. A null
// source (or clearTrigger) assigns no trigger. Parented to its IIODevice;
// discovered via findChild<IIOTriggerable*>().
class IIOTriggerable : public QObject
{
	Q_OBJECT
	Q_PROPERTY(bool hasTrigger READ hasTrigger NOTIFY triggerSucceeded)
public:
	IIOTriggerable(scopy::iio::IDeviceOps *ops, scopy::iio::DeviceHandle dev, scopy::ICmdExecutor *executor,
		       QObject *parent = nullptr);

	bool hasTrigger() const;
	QString assignedTriggerName() const;

	Q_INVOKABLE QCoro::Task<CommandResponse<void>> setTriggerAsync(IIOTrigger *source);
	Q_INVOKABLE QCoro::Task<CommandResponse<void>> clearTriggerAsync();

Q_SIGNALS:
	void triggerSucceeded();
	void triggerFailed(const scopy::Error &error);

private:
	QCoro::Task<CommandResponse<void>> setInternal(scopy::iio::SetTriggerCommand *cmd);

	scopy::iio::IDeviceOps *m_ops;
	scopy::iio::DeviceHandle m_dev;
	scopy::ICmdExecutor *m_executor;
};

} // namespace scopy::component::iio
