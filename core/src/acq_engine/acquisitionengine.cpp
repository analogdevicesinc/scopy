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

#include "acquisitionengine.h"

#include <algorithm>
#include <exception>
#include <numeric>
#include <QElapsedTimer>
#include <QMetaObject>
#include <QMutexLocker>
#include <QStringList>

namespace scopy {
namespace acq {

using Severity = AcquisitionError::Severity;

namespace {
constexpr std::size_t kDefaultIndexRampLength = 1024;
} // namespace

AcquisitionEngine::AcquisitionEngine(DataStore *store, QObject *parent)
	: QObject(parent)
	, m_store(store)
{
	qRegisterMetaType<scopy::acq::DataKey>("scopy::acq::DataKey");

	// A channel can read its X source before the first cycle.
	if(m_store)
		publishIndexRamp(kDefaultIndexRampLength);
}

AcquisitionEngine::~AcquisitionEngine() { stop(); }

// --- Block registration ------------------------------------------------------
//
// Requests are queued and applied between cycles by syncBlocks(); while stopped
// it runs inline so the lists are immediately observable.

void AcquisitionEngine::addSource(SourceBlock *src)
{
	{
		QMutexLocker lk(&m_blockMutex);
		m_pendingRemoveSources.removeAll(src);
		m_pendingAddSources.append(src);
	}
	if(!m_threadAlive)
		syncBlocks();
}

void AcquisitionEngine::removeSource(SourceBlock *src)
{
	{
		QMutexLocker lk(&m_blockMutex);
		m_pendingAddSources.removeAll(src);
		if(!m_pendingRemoveSources.contains(src))
			m_pendingRemoveSources.append(src);
	}
	if(!m_threadAlive)
		syncBlocks();
}

void AcquisitionEngine::addProcessor(ProcessorBlock *proc)
{
	{
		QMutexLocker lk(&m_blockMutex);
		m_pendingRemoveProcessors.removeAll(proc);
		m_pendingAddProcessors.append(proc);
	}
	if(!m_threadAlive)
		syncBlocks();
}

void AcquisitionEngine::removeProcessor(ProcessorBlock *proc)
{
	{
		QMutexLocker lk(&m_blockMutex);
		m_pendingAddProcessors.removeAll(proc);
		if(!m_pendingRemoveProcessors.contains(proc))
			m_pendingRemoveProcessors.append(proc);
	}
	if(!m_threadAlive)
		syncBlocks();
}

void AcquisitionEngine::syncBlocks()
{
	QList<SourceBlock *> removedSources;
	QList<ProcessorBlock *> removedProcessors;
	bool changed;
	{
		QMutexLocker lk(&m_blockMutex);
		changed = !m_pendingAddSources.isEmpty() || !m_pendingRemoveSources.isEmpty() ||
			!m_pendingAddProcessors.isEmpty() || !m_pendingRemoveProcessors.isEmpty();

		for(SourceBlock *s : m_pendingRemoveSources)
			m_sources.removeAll(s);
		for(SourceBlock *s : m_pendingAddSources)
			if(!m_sources.contains(s))
				m_sources.append(s);

		for(ProcessorBlock *p : m_pendingRemoveProcessors)
			m_processors.removeAll(p);
		for(ProcessorBlock *p : m_pendingAddProcessors)
			if(!m_processors.contains(p))
				m_processors.append(p);

		removedSources.swap(m_pendingRemoveSources);
		removedProcessors.swap(m_pendingRemoveProcessors);
		m_pendingAddSources.clear();
		m_pendingAddProcessors.clear();
	}

	// Out of every list now, so nothing on the worker can reach them again. A started
	// source is stopped here, on the thread that started it. deleteLater() posts to
	// the block's own (GUI) thread, which is where its widgets and connections live.
	for(SourceBlock *s : removedSources) {
		stopSource(s);
		s->deleteLater();
	}
	for(ProcessorBlock *p : removedProcessors)
		p->deleteLater();

	if(changed)
		Q_EMIT blocksChanged();
}

// --- Stream descriptors ------------------------------------------------------

QList<Block *> AcquisitionEngine::blockSnapshot() const
{
	QMutexLocker lk(&m_blockMutex);
	QList<Block *> blocks;
	blocks.reserve(m_sources.size() + m_processors.size());
	for(SourceBlock *s : m_sources)
		blocks.append(s);
	for(ProcessorBlock *p : m_processors)
		blocks.append(p);
	return blocks;
}

std::optional<StreamInfo> AcquisitionEngine::streamInfo(const DataKey &key) const
{
	if(key == indexRampKey()) {
		StreamInfo info;
		info.label = QStringLiteral("sample index");
		info.unit = QStringLiteral("samples");
		info.kind = ReprKind::Hidden;
		return info;
	}

	// Keys are namespaced by producer, so at most one block answers.
	for(Block *b : blockSnapshot()) {
		if(std::optional<StreamInfo> info = b->streamInfo(key))
			return info;
	}
	return std::nullopt;
}

QList<DataKey> AcquisitionEngine::declaredKeys() const
{
	QSet<DataKey> all;
	for(Block *b : blockSnapshot()) {
		const QList<DataKey> out = b->outputKeys();
		for(const DataKey &k : out)
			all.insert(k);
	}
	all.insert(indexRampKey());
	return all.values();
}

QList<SourceBlock *> AcquisitionEngine::sources() const
{
	QMutexLocker lk(&m_blockMutex);
	return m_sources;
}

QList<ProcessorBlock *> AcquisitionEngine::processors() const
{
	QMutexLocker lk(&m_blockMutex);
	return m_processors;
}

// --- Configuration -----------------------------------------------------------

bool AcquisitionEngine::isRunning() const { return m_running; }
void AcquisitionEngine::setBufferSize(std::size_t size) { m_bufferSize = size; }
std::size_t AcquisitionEngine::bufferSize() const { return m_bufferSize; }
void AcquisitionEngine::setMaxFPS(unsigned int fps) { m_maxFPS.store(fps); }
unsigned int AcquisitionEngine::maxFPS() const { return m_maxFPS.load(); }
void AcquisitionEngine::setMode(Mode m) { m_mode.store(m); }
AcquisitionEngine::Mode AcquisitionEngine::mode() const { return m_mode.load(); }
void AcquisitionEngine::setMinReportSeverity(Severity sev) { m_minSeverity.store(sev); }
Severity AcquisitionEngine::minReportSeverity() const { return m_minSeverity.load(); }

// --- The sample-index ramp ---------------------------------------------------

DataKey AcquisitionEngine::indexRampKey()
{
	return DataKey::withStage(QStringLiteral("engine"), QStringLiteral("sample"), QStringLiteral("index"));
}

std::size_t AcquisitionEngine::indexRampLength() const
{
	QMutexLocker lk(&m_rampMutex);
	return static_cast<std::size_t>(m_indexRamp.size());
}

void AcquisitionEngine::setIndexRampLength(std::size_t n) { publishIndexRamp(n); }

void AcquisitionEngine::publishIndexRamp(std::size_t n)
{
	if(!m_store)
		return;

	n = std::max<std::size_t>(1, n);

	QVector<float> ramp;
	{
		QMutexLocker lk(&m_rampMutex);
		if(static_cast<std::size_t>(m_indexRamp.size()) != n) {
			m_indexRamp.resize(static_cast<int>(n));
			std::iota(m_indexRamp.begin(), m_indexRamp.end(), 0.0f);
		}
		ramp = m_indexRamp; // shared copy; written outside the lock
	}

	// Unconditional: also re-establishes the chunk after DataStore::clear().
	m_store->write(indexRampKey(), ramp);
}

// --- Reporting ---------------------------------------------------------------

void AcquisitionEngine::reportInfo(const QString &id, const QString &msg)
{
	if(Severity::Info >= m_minSeverity.load())
		Q_EMIT error(static_cast<int>(Severity::Info), id, msg);
}

void AcquisitionEngine::reportWarningOnce(const QString &id, const QString &msg)
{
	if(Severity::Warning < m_minSeverity.load())
		return;
	QString &last = m_lastWarning[id];
	if(last == msg)
		return;
	last = msg;
	Q_EMIT error(static_cast<int>(Severity::Warning), id, msg);
}

template <class Fn>
bool AcquisitionEngine::guarded(const QString &id, const QString &what, Fn &&fn)
{
	try {
		fn();
		return true;
	} catch(const std::exception &e) {
		Q_EMIT error(static_cast<int>(Severity::Critical), id, QString::fromStdString(e.what()));
	} catch(...) {
		Q_EMIT error(static_cast<int>(Severity::Critical), id, QStringLiteral("unknown exception in ") + what);
	}
	return false;
}

// --- Run control -------------------------------------------------------------

void AcquisitionEngine::run() { startLoop(0); }
void AcquisitionEngine::single(unsigned int count) { startLoop(static_cast<int>(count)); }

void AcquisitionEngine::startLoop(int acqCount)
{
	if(m_threadAlive)
		return;

	// A previous self-terminated run may have left a finished thread behind.
	joinThread();
	syncBlocks();
	publishIndexRamp(indexRampLength());

	m_acqCount = acqCount;
	m_faultStop = false;
	m_cyclePending = false;

	// Set before the thread exists, so a re-entrant run() sees a live run. Sources are
	// started on the worker, never here: onStart() may block on a device.
	m_running = true;
	m_threadAlive = true;
	m_thread = QThread::create([this] { loop(); });
	m_thread->start();
}

void AcquisitionEngine::stop()
{
	m_running = false;
	if(m_threadAlive) {
		for(Block *b : blockSnapshot())
			b->requestStop();
	}
	joinThread();
	// Applies anything removed during the run's teardown tail, which the loop no
	// longer looked at.
	syncBlocks();
}

void AcquisitionEngine::joinThread()
{
	if(!m_thread)
		return;
	m_thread->wait();
	delete m_thread;
	m_thread = nullptr;
}

void AcquisitionEngine::stopSource(SourceBlock *src)
{
	if(!m_startedSources.remove(src))
		return;
	guarded(src->id(), QStringLiteral("onStop()"), [src] { src->onStop(); });
}

void AcquisitionEngine::stopAllSources()
{
	const QList<SourceBlock *> started = m_startedSources.values();
	for(SourceBlock *src : started)
		stopSource(src);
}

bool AcquisitionEngine::syncSourceStates()
{
	for(SourceBlock *src : std::as_const(m_sources)) {
		const bool started = m_startedSources.contains(src);
		if(src->isEnabled() && !started) {
			src->setBufferSize(m_bufferSize);
			if(!guarded(src->id(), QStringLiteral("onStart()"), [src] { src->onStart(); }))
				return false;
			m_startedSources.insert(src);
		} else if(!src->isEnabled() && started) {
			stopSource(src);
		}
	}
	return true;
}

void AcquisitionEngine::postCycleComplete()
{
	// At most one in flight: a GUI slower than the worker sees the newest cycle
	// instead of draining a queue of stale ones.
	if(m_cyclePending.exchange(true))
		return;
	QMetaObject::invokeMethod(
		this,
		[this] {
			m_cyclePending = false;
			Q_EMIT cycleComplete();
		},
		Qt::QueuedConnection);
}

void AcquisitionEngine::loop()
{
	m_lastWarning.clear();
	m_startedSources.clear();

	for(ProcessorBlock *proc : std::as_const(m_processors))
		proc->reset();

	if(!syncSourceStates()) {
		m_faultStop = true;
		m_running = false;
	} else {
		Q_EMIT started();
		if(Severity::Info >= m_minSeverity.load()) {
			QStringList srcNames, procNames;
			for(SourceBlock *s : std::as_const(m_sources))
				srcNames << s->id();
			for(ProcessorBlock *p : std::as_const(m_processors))
				procNames << p->name();
			reportInfo(QStringLiteral("engine"),
				   QStringLiteral("started | sources: [%1] | processors: [%2] | buffer: %3")
					   .arg(srcNames.join(", "),
						procNames.isEmpty() ? QStringLiteral("none") : procNames.join(", "))
					   .arg(m_bufferSize.load()));
		}
	}

	const int acqCount = m_acqCount;
	int cyclesDone = 0;
	QElapsedTimer cycleTimer, heartbeatTimer;
	heartbeatTimer.start();

	while(m_running) {
		cycleTimer.start();
		syncBlocks();
		if(!syncSourceStates()) {
			m_faultStop = true;
			break;
		}
		m_store->beginCycle();

		bool aborted = false;
		for(SourceBlock *src : std::as_const(m_sources)) {
			if(!m_startedSources.contains(src))
				continue;

			const quint64 before = m_store->writeCount();
			if(!guarded(src->id(), QStringLiteral("acquire()"), [&] { src->acquire(m_store); })) {
				m_faultStop = true;
				aborted = true;
				break;
			}
			if(m_store->writeCount() == before)
				reportWarningOnce(src->id(), QStringLiteral("acquire() wrote no data"));
			else
				m_lastWarning.remove(src->id());
			if(!m_running) {
				aborted = true;
				break;
			}
		}
		if(aborted)
			break;

		if(!runProcessors())
			break;

		const unsigned int fps = m_maxFPS.load();
		if(m_mode.load() == Mode::Triggered) {
			postCycleComplete();
			if(!m_running)
				break;
			if(fps > 0) {
				const qint64 minMs = 1000LL / fps;
				const qint64 spent = cycleTimer.elapsed();
				if(spent < minMs)
					QThread::msleep(static_cast<unsigned long>(minMs - spent));
			}
		} else {
			const qint64 hbMs = fps > 0 ? 1000LL / fps : 16;
			if(heartbeatTimer.elapsed() >= hbMs) {
				postCycleComplete();
				heartbeatTimer.restart();
			}
		}

		if(acqCount > 0 && ++cyclesDone >= acqCount)
			break;
	}

	m_running = false;

	// Publish whatever the last cycle produced: continuous mode gates notifications
	// on the heartbeat.
	if(!m_faultStop)
		postCycleComplete();

	stopAllSources();

	reportInfo(QStringLiteral("engine"), QStringLiteral("stopped"));
	if(m_faultStop)
		Q_EMIT forceStopped();
	else
		Q_EMIT stopped();

	// Last statement: stop() may return as soon as this clears.
	m_threadAlive = false;
}

bool AcquisitionEngine::runProcessors()
{
	if(m_processors.isEmpty())
		return true;

	// One snapshot per processor per cycle: the GUI may change them mid-cycle.
	QHash<ProcessorBlock *, QList<DataKey>> keysOf;
	keysOf.reserve(m_processors.size());
	for(ProcessorBlock *proc : std::as_const(m_processors))
		keysOf.insert(proc, proc->watchedKeys());

	QSet<ProcessorBlock *> executed;
	QSet<DataKey> dirty = m_store->cycleKeys();

	// Re-scan until a pass changes nothing; run-last processors get a final pass of
	// their own so they observe everything the others wrote.
	for(bool deferred : {false, true}) {
		bool progress = true;
		while(progress) {
			progress = false;
			for(ProcessorBlock *proc : std::as_const(m_processors)) {
				if(executed.contains(proc) || !proc->isEnabled() || proc->runsLast() != deferred)
					continue;

				const QList<DataKey> &keys = keysOf[proc];
				const bool ready = std::all_of(keys.cbegin(), keys.cend(),
							       [&](const DataKey &k) { return dirty.contains(k); });
				if(!ready)
					continue;

				if(!guarded(proc->name(), QStringLiteral("process()"),
					    [&] { proc->process(m_store); })) {
					m_faultStop = true;
					return false;
				}
				executed.insert(proc);
				progress = true;
			}
			if(progress)
				dirty = m_store->cycleKeys();
		}
	}

	// A processor can legitimately idle for many cycles, so warn once, not per cycle.
	for(ProcessorBlock *proc : std::as_const(m_processors)) {
		if(!proc->isEnabled())
			continue;
		if(executed.contains(proc)) {
			m_lastWarning.remove(proc->name());
			continue;
		}
		QStringList missing;
		for(const DataKey &k : keysOf[proc])
			if(!dirty.contains(k))
				missing << k.toString();
		reportWarningOnce(proc->name(), QStringLiteral("skipped — missing input keys: ") + missing.join(", "));
	}
	return true;
}

} // namespace acq
} // namespace scopy

#include "moc_acquisitionengine.cpp"
