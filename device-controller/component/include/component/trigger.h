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

#include <QObject>
#include <QString>

#include <qcoro/qcorotask.h>
#include "component/component_export.h"

namespace scopy::component {

// Trigger SOURCE (abstract base): something a Triggerable can be gated by. Pure metadata.
// Parented to the device that provides it; discovered with findChildren<Trigger*>().
class COMPONENT_EXPORT Trigger : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString name READ name CONSTANT)
public:
	explicit Trigger(QObject *parent = nullptr)
		: QObject(parent)
	{}
	~Trigger() override = default;

	virtual QString name() const = 0;
};

// Consumer side of the trigger relationship (abstract base): the acquisition of the parent
// device can be assigned one Trigger. A null source (or clearTriggerAsync) assigns none. A
// backend only accepts its own sources; others fail with -EINVAL.
class COMPONENT_EXPORT Triggerable : public QObject
{
	Q_OBJECT
	Q_PROPERTY(bool hasTrigger READ hasTrigger NOTIFY triggerSucceeded)
	Q_PROPERTY(QString assignedTriggerName READ assignedTriggerName NOTIFY triggerSucceeded)
public:
	explicit Triggerable(QObject *parent = nullptr)
		: QObject(parent)
	{}
	~Triggerable() override = default;

	virtual bool hasTrigger() const = 0;
	virtual QString assignedTriggerName() const = 0;
	Q_INVOKABLE virtual QCoro::Task<CommandResponse<void>> setTriggerAsync(scopy::component::Trigger *source) = 0;
	Q_INVOKABLE virtual QCoro::Task<CommandResponse<void>> clearTriggerAsync() = 0;

Q_SIGNALS:
	void triggerSucceeded();
	void triggerFailed(const scopy::Error &error);
};

} // namespace scopy::component
