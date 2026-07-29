#pragma once

#include "scopy-core_export.h"

#include "AcquisitionEngine.h"
#include "SampleBuffer.h"
#include "TriggerProcessor.h"

#include <QMap>
#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QString>

namespace scopy {
namespace acq {

// TriggerBinder — standardized, lifetime-safe wiring between a
// TriggerProcessor and an AcquisitionEngine.
//
// Callers (typically instrument controllers) use this instead of hand-rolled
// `QMetaObject::Connection *` gymnastics inside GUI lambdas. Each binder
// owns exactly one TriggerProcessor / AcquisitionEngine pair; all connections
// live under the binder as QObject-owned children of `this`, so destroying
// the binder tears every wire down atomically.
//
// Threading:
//   All external slots delivered by the binder run on the binder's own thread
//   (typically the GUI thread) via Qt::QueuedConnection. The `fired()` snapshot
//   carried by TriggerProcessor is copied into the queued event on the worker
//   thread, so consumers never touch worker-owned buffers.
class SCOPY_CORE_EXPORT TriggerBinder : public QObject
{
	Q_OBJECT
public:
	// Non-owning references. `trig` must have been added to `engine`
	// (via AcquisitionEngine::addProcessor) before or after construction —
	// the binder only wires signals, it does not manage ownership.
	TriggerBinder(TriggerProcessor *trig, AcquisitionEngine *engine,
		      QObject *parent = nullptr);
	~TriggerBinder() override;

	TriggerProcessor  *trigger() const { return m_trig; }
	AcquisitionEngine *engine()  const { return m_engine; }

	// --- One-shot: fire → stop the engine, then disarm. ------------------
	// After arm() the *next* TriggerProcessor::fired will:
	//   1. emit singleShotFired(sampleIndex, snapshot) on the GUI thread
	//   2. call engine->stop()
	//   3. disconnect itself (idempotent under re-fire races)
	// disarmSingleShot() cancels a pending arm without stopping the engine.
	// Both are safe to call regardless of prior state.
	void armSingleShot();
	void disarmSingleShot();
	bool singleShotArmed() const { return m_singleShotArmed; }

	// --- Persistent: forward every fire until unbindReplotOnFire(). -----
	// Intended for the "trigger-driven replot" use case where the receiver
	// is a plot refresh slot. The snapshot payload is the fire-cycle chunk
	// map captured inside TriggerProcessor::process() and is safe to read
	// on the GUI thread.
	// Only one persistent forward at a time; a second call replaces the
	// prior connection.
	void bindReplotOnFire();
	void unbindReplotOnFire();
	bool replotOnFireBound() const { return static_cast<bool>(m_replotConn); }

Q_SIGNALS:
	// Delivered on the binder's thread (GUI).
	void singleShotFired(quint32 sampleIndex,
			     QMap<QString, scopy::acq::SampleVariant> snapshot);
	void replotFired(quint32 sampleIndex,
			 QMap<QString, scopy::acq::SampleVariant> snapshot);

private:
	QPointer<TriggerProcessor>  m_trig;
	QPointer<AcquisitionEngine> m_engine;

	QMetaObject::Connection m_singleShotConn;
	QMetaObject::Connection m_replotConn;
	bool                    m_singleShotArmed{false};
};

} // namespace acq
} // namespace scopy
