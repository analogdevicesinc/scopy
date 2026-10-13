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

#include "acquisitionerror.h"
#include "datastore.h"
#include "processorblock.h"
#include "sourceblock.h"

#include <atomic>
#include <optional>
#include <QHash>
#include <QList>
#include <QMutex>
#include <QObject>
#include <QSet>
#include <QThread>

namespace scopy {
namespace acq {

// Drives acquisition on a dedicated worker thread.
//
// Each cycle: every enabled SourceBlock writes its chunks into the DataStore, then
// every enabled ProcessorBlock whose watchedKeys were all written this cycle runs.
// Processors are re-scanned until none becomes newly runnable, so chains resolve in
// dependency order without a declared graph.
//
// Threading: the public API is GUI-thread facing and safe to call while running.
// Blocks are only ever invoked from the worker (streamInfo()/outputKeys() aside),
// and add/remove/enable changes are applied there at the next cycle boundary.
// started/stopped/forceStopped/error/blocksChanged are emitted from the worker;
// cycleComplete is delivered on the engine's own thread. Connect queued.
//
// Ownership: blocks are parented to the engine. removeSource()/removeProcessor()
// hand the block back for disposal: the engine deletes it once the worker has let
// go of it, so callers must not delete a removed block themselves.
class SCOPY_CORE_EXPORT AcquisitionEngine : public QObject
{
	Q_OBJECT
public:
	// Continuous: the loop free-runs and cycleComplete() is rate-limited to maxFPS.
	// Triggered: the loop itself is throttled to maxFPS.
	enum class Mode
	{
		Continuous,
		Triggered
	};

	explicit AcquisitionEngine(DataStore *store, QObject *parent = nullptr);
	~AcquisitionEngine() override;

	// Borrowed; must outlive the engine.
	DataStore *store() const { return m_store; }

	void addSource(SourceBlock *src);
	void removeSource(SourceBlock *src);
	QList<SourceBlock *> sources() const;

	void addProcessor(ProcessorBlock *proc);
	void removeProcessor(ProcessorBlock *proc);
	QList<ProcessorBlock *> processors() const;

	// What a key means (label, unit, rate, X source, how to draw it), asked of the
	// producing block on every call. nullopt when no block describes it.
	std::optional<StreamInfo> streamInfo(const DataKey &key) const;

	// Every key any block declares, written yet or not, plus the index ramp. Lets a
	// view create its channel — and its depth claim — before the first cycle.
	QList<DataKey> declaredKeys() const;

	void setMode(Mode m);
	Mode mode() const;

	// Latched by sources at their start; changes apply to the next run.
	void setBufferSize(std::size_t size);
	std::size_t bufferSize() const;

	// The shared sample-index ramp: 0..L-1 as Float32, published as a single chunk
	// and rewritten only when L changes, so it never lands in cycleKeys() and a
	// window wider than one buffer does not read back as a sawtooth. Set L to the
	// widest plot window. Safe to call while running.
	static DataKey indexRampKey();
	void setIndexRampLength(std::size_t n);
	std::size_t indexRampLength() const;

	// cycleComplete() rate ceiling; 0 = as fast as the pipeline runs. Re-read every
	// cycle.
	void setMaxFPS(unsigned int fps);
	unsigned int maxFPS() const;

	bool isRunning() const;

	// Reports below this severity are dropped before being built. Default Warning.
	void setMinReportSeverity(AcquisitionError::Severity sev);
	AcquisitionError::Severity minReportSeverity() const;

	// Run until stop(); or for exactly `count` cycles. Ignored while a run is still
	// alive, including its teardown tail.
	void run();
	void single(unsigned int count = 1);
	// Asks every block to unblock (Block::requestStop), then joins the worker.
	void stop();

Q_SIGNALS:
	void started();
	void stopped();
	// Emitted instead of stopped() when a block threw and aborted the run.
	void forceStopped();
	// Coalesced: at most one is pending at a time, so a slow GUI never builds a
	// backlog. Read the store for the newest data.
	void cycleComplete();
	void error(int severity, const QString &id, const QString &message);
	// sources()/processors() changed.
	void blocksChanged();

private:
	void startLoop(int acqCount);
	void loop();
	void joinThread();

	// Applies queued add/remove requests and disposes of removed blocks. Worker
	// thread while running, caller's thread otherwise.
	void syncBlocks();

	// Starts enabled-but-unstarted sources and stops disabled-but-started ones.
	// Worker thread. Returns false if a start threw.
	bool syncSourceStates();
	void stopSource(SourceBlock *src);
	void stopAllSources();

	// Both lists, copied under m_blockMutex so callers can call into blocks with
	// the lock released.
	QList<Block *> blockSnapshot() const;

	void publishIndexRamp(std::size_t n);
	bool runProcessors();
	void postCycleComplete();

	template <class Fn>
	bool guarded(const QString &id, const QString &what, Fn &&fn);
	void reportInfo(const QString &id, const QString &msg);
	// Warns once per distinct (id, msg) until the message changes. Cleared per run.
	void reportWarningOnce(const QString &id, const QString &msg);

	DataStore *m_store;

	// Live lists. Mutated only in syncBlocks(), under m_blockMutex.
	QList<SourceBlock *> m_sources;
	QList<ProcessorBlock *> m_processors;

	mutable QMutex m_blockMutex;
	QList<SourceBlock *> m_pendingAddSources, m_pendingRemoveSources;
	QList<ProcessorBlock *> m_pendingAddProcessors, m_pendingRemoveProcessors;

	// Worker-only: sources whose onStart() has run this run.
	QSet<SourceBlock *> m_startedSources;

	QThread *m_thread{nullptr};

	// m_running gates the loop; m_threadAlive stays set through the teardown tail.
	std::atomic<bool> m_running{false};
	std::atomic<bool> m_threadAlive{false};
	std::atomic<bool> m_faultStop{false};
	std::atomic<bool> m_cyclePending{false};
	std::atomic<Mode> m_mode{Mode::Triggered};
	std::atomic<AcquisitionError::Severity> m_minSeverity{AcquisitionError::Severity::Warning};
	std::atomic<int> m_acqCount{0};
	std::atomic<std::size_t> m_bufferSize{1024};
	std::atomic<unsigned int> m_maxFPS{0};

	// Worker-only.
	QHash<QString, QString> m_lastWarning;

	mutable QMutex m_rampMutex;
	QVector<float> m_indexRamp;
};

} // namespace acq
} // namespace scopy
