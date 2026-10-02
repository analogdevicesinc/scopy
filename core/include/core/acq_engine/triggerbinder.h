/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#include "scopy-core_export.h"

#include "acquisitionengine.h"
#include "samplebuffer.h"
#include "triggerprocessor.h"

#include <QMap>
#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QString>

namespace scopy {
namespace acq {

// Lifetime-safe wiring between one TriggerProcessor and one AcquisitionEngine,
// so call sites don't hand-roll `QMetaObject::Connection *` bookkeeping inside
// GUI lambdas. Destroying the binder tears every wire down at once.
//
// Both signals below are delivered on the binder's own thread (typically the
// GUI thread) via Qt::QueuedConnection. The fired() snapshot is copied into the
// queued event on the worker thread, so consumers never touch worker buffers.
class SCOPY_CORE_EXPORT TriggerBinder : public QObject
{
	Q_OBJECT
public:
	// Non-owning references; the binder wires signals but owns neither end.
	TriggerBinder(TriggerProcessor *trig, AcquisitionEngine *engine, QObject *parent = nullptr);
	~TriggerBinder() override;

	TriggerProcessor *trigger() const { return m_trig; }
	AcquisitionEngine *engine() const { return m_engine; }

	// One-shot: the next fire emits singleShotFired(), stops the engine, and
	// disconnects itself. Both calls are safe in any prior state; disarm
	// cancels a pending arm without stopping the engine.
	void armSingleShot();
	void disarmSingleShot();
	bool singleShotArmed() const { return m_singleShotArmed; }

	// Persistent: forward every fire as replotFired() until unbound. Only one
	// forward at a time — a second bind replaces the previous connection.
	void bindReplotOnFire();
	void unbindReplotOnFire();
	bool replotOnFireBound() const { return static_cast<bool>(m_replotConn); }

Q_SIGNALS:
	void singleShotFired(quint32 sampleIndex, QMap<QString, scopy::acq::SampleVariant> snapshot);
	void replotFired(quint32 sampleIndex, QMap<QString, scopy::acq::SampleVariant> snapshot);

private:
	QPointer<TriggerProcessor> m_trig;
	QPointer<AcquisitionEngine> m_engine;

	QMetaObject::Connection m_singleShotConn;
	QMetaObject::Connection m_replotConn;
	bool m_singleShotArmed{false};
};

} // namespace acq
} // namespace scopy
