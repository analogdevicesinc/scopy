#include "AcquisitionEngine.h"

#include <algorithm>
#include <exception>
#include <numeric>
#include <QElapsedTimer>
#include <QMutexLocker>
#include <QStringList>

namespace scopy {
namespace acq {

using Severity = AcquisitionError::Severity;

AcquisitionEngine::AcquisitionEngine(DataStore *store, QObject *parent)
	: QObject(parent)
	, m_store(store)
	, m_defaultIndexRampLength(1024)
{
	qRegisterMetaType<scopy::acq::DataKey>("scopy::acq::DataKey");

	// Written in the ctor: a channel can be created and read before the first
	// cycle, and an X source with no samples draws nothing at all. The matching
	// descriptor needs no publishing — streamInfo() answers for this key itself.
	if(m_store)
		publishIndexRamp(m_defaultIndexRampLength);
}

AcquisitionEngine::~AcquisitionEngine()
{
	stop();
}

// --- Block registration ------------------------------------------------------
//
// Requests are queued and applied by the worker between cycles (syncBlocks), so
// a mid-run add/remove can never race the worker's iteration. While stopped,
// syncBlocks() runs inline so the lists are immediately observable.

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
		m_pendingRemoveProcessors.append(proc);
	}
	if(!m_threadAlive)
		syncBlocks();
}

void AcquisitionEngine::setRunLast(ProcessorBlock *proc)
{
	QMutexLocker lk(&m_blockMutex);
	m_runLast.insert(proc);
}

void AcquisitionEngine::syncBlocks()
{
	bool changed;
	{
		QMutexLocker lk(&m_blockMutex);

		// Called once per cycle from loop(), so most calls have nothing
		// pending — only report a change when something was actually applied.
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

		m_pendingAddSources.clear();
		m_pendingRemoveSources.clear();
		m_pendingAddProcessors.clear();
		m_pendingRemoveProcessors.clear();
	}

	// Emitted with m_blockMutex released: a directly-connected consumer that
	// reached back into sources()/processors() would deadlock on it.
	if(changed)
		Q_EMIT blocksChanged();
}

// --- Stream descriptors ------------------------------------------------------
//
// Both take a snapshot of the block lists under m_blockMutex and then call into
// the blocks with it released: a block takes its own lock in there (SourceBlock
// its channel mutex, GenalyzerFFTProcessor the genalyzer one), and holding two at
// once is how deadlocks are built.

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
	// The ramp is the engine's own stream, not any block's, so it is answered here.
	// First, not last: no block produces this key, so asking them all would be a
	// walk that cannot succeed. Hidden, like every stream that exists to be read as
	// an X axis and is not a trace.
	if(key == indexRampKey()) {
		StreamInfo info;
		info.label = QStringLiteral("sample index");
		info.unit  = QStringLiteral("samples");
		info.kind  = ReprKind::Hidden;
		return info;
	}

	// First answer wins, which is not a real contest: keys are namespaced by
	// producer name, so at most one block recognises any given one.
	const QList<Block *> blocks = blockSnapshot();
	for(Block *b : blocks) {
		if(const std::optional<StreamInfo> info = b->streamInfo(key))
			return info;
	}

	return std::nullopt;
}

QList<DataKey> AcquisitionEngine::declaredKeys() const
{
	// outputKeys(), not a second per-block enumeration: a block's outputs *are* the
	// keys it can describe, and the descriptor lookup above answers nullopt for
	// anything else. One list, so there is nothing to drift.
	QSet<DataKey> all;
	const QList<Block *> blocks = blockSnapshot();
	for(Block *b : blocks) {
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

bool         AcquisitionEngine::isRunning() const { return m_running; }
void         AcquisitionEngine::setBufferSize(std::size_t size) { m_bufferSize = size; }
std::size_t  AcquisitionEngine::bufferSize() const { return m_bufferSize; }

// --- The sample-index ramp ---------------------------------------------------

DataKey AcquisitionEngine::indexRampKey()
{
	// Neither component may contain an underscore: DataKey parses right-anchored
	// on '_', so "sample_index" as one component would reparse as channel
	// "sample" / stage "index" and sourceId() would answer wrongly.
	return DataKey::withStage(QStringLiteral("engine"), QStringLiteral("sample"),
				  QStringLiteral("index"));
}

std::size_t AcquisitionEngine::indexRampLength() const
{
	QMutexLocker lk(&m_rampMutex);
	return static_cast<std::size_t>(m_indexRamp.size());
}

void AcquisitionEngine::setIndexRampLength(std::size_t n)
{
	publishIndexRamp(n);
}

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
		// Copied under the lock, written outside it: DataStore::write takes its
		// own mutex, and holding two at once is how deadlocks are built. The copy
		// is a refcount bump — the resize above is what detaches.
		ramp = m_indexRamp;
	}

	// Unconditional, even when the length did not change: this is also the path
	// that re-establishes the chunk after DataStore::clear() dropped it.
	m_store->write(indexRampKey(), ramp);
}

// --- Configuration, continued ------------------------------------------------

void         AcquisitionEngine::setMaxFPS(unsigned int fps) { m_maxFPS.store(fps); }
unsigned int AcquisitionEngine::maxFPS() const { return m_maxFPS.load(); }
void         AcquisitionEngine::setMode(Mode m) { m_mode.store(m); }
AcquisitionEngine::Mode AcquisitionEngine::mode() const { return m_mode.load(); }

void AcquisitionEngine::setMinReportSeverity(Severity sev) { m_minSeverity.store(sev); }
Severity AcquisitionEngine::minReportSeverity() const { return m_minSeverity.load(); }

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

template<class Fn>
bool AcquisitionEngine::guarded(const QString &id, const QString &what, Fn &&fn)
{
	try {
		fn();
		return true;
	} catch(const std::exception &e) {
		Q_EMIT error(static_cast<int>(Severity::Critical), id,
			     QString::fromStdString(e.what()));
	} catch(...) {
		Q_EMIT error(static_cast<int>(Severity::Critical), id,
			     QStringLiteral("unknown exception in ") + what);
	}
	return false;
}

// --- Run control -------------------------------------------------------------

void AcquisitionEngine::run()                      { startLoop(0); }
void AcquisitionEngine::single(unsigned int count) { startLoop(static_cast<int>(count)); }

void AcquisitionEngine::startLoop(int acqCount)
{
	if(m_threadAlive)
		return;

	// A previous self-terminated run may have left a finished thread behind.
	joinThread();

	syncBlocks();

	// Unconditional: a previous run's clear()/reset() drops the ramp's chunk, so it
	// is re-established here rather than only when its length changes.
	publishIndexRamp(indexRampLength());

	m_acqCount  = acqCount;
	m_faultStop = false;

	for(SourceBlock *src : m_sources) {
		if(!src->isEnabled())
			continue;
		src->setBufferSize(m_bufferSize);
		if(!guarded(src->id(), QStringLiteral("onStart()"), [src] { src->onStart(); }))
			m_faultStop = true;
	}

	// Release whatever the partially-started sources already claimed.
	if(m_faultStop) {
		for(SourceBlock *src : m_sources)
			if(src->isEnabled())
				guarded(src->id(), QStringLiteral("onStop()"),
					[src] { src->onStop(); });
		Q_EMIT forceStopped();
		return;
	}

	for(ProcessorBlock *proc : m_processors)
		proc->reset();

	m_running     = true;
	m_threadAlive = true;
	// Owned, not self-deleting: startLoop() and stop() both need to wait() on
	// it after a self-terminated run, so it must outlive loop() returning.
	m_thread      = QThread::create([this] { loop(); });
	m_thread->start();
}

void AcquisitionEngine::stop()
{
	m_running = false;
	joinThread();
}

// Waits for the worker to finish and destroys the QThread. Safe to call when no
// thread exists or when the worker already self-terminated.
void AcquisitionEngine::joinThread()
{
	if(!m_thread)
		return;
	m_thread->wait();
	delete m_thread;
	m_thread = nullptr;
}

void AcquisitionEngine::loop()
{
	Q_EMIT started();

	{
		QStringList srcNames, procNames;
		for(SourceBlock *s : m_sources)       srcNames  << s->id();
		for(ProcessorBlock *p : m_processors) procNames << p->name();
		reportInfo(QStringLiteral("engine"),
			   QStringLiteral("started | sources: [%1] | processors: [%2] | buffer: %3")
				   .arg(srcNames.join(", "),
					procNames.isEmpty() ? QStringLiteral("none")
							    : procNames.join(", "))
				   .arg(m_bufferSize));
	}

	m_lastWarning.clear();

	int           cyclesDone = 0;
	QElapsedTimer cycleTimer, heartbeatTimer;
	heartbeatTimer.start();

	while(m_running) {
		cycleTimer.start();
		syncBlocks();
		m_store->beginCycle();

		bool aborted = false;
		for(SourceBlock *src : m_sources) {
			if(!src->isEnabled())
				continue;

			const quint64 before = m_store->writeCount();
			if(!guarded(src->id(), QStringLiteral("acquire()"),
				    [&] { src->acquire(m_store); })) {
				m_faultStop = true;
				aborted     = true;
				break;
			}
			if(m_store->writeCount() == before)
				reportWarningOnce(src->id(),
						  QStringLiteral("acquire() wrote no data"));
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

		if(m_mode.load() == Mode::Triggered) {
			Q_EMIT cycleComplete();
			if(!m_running)
				break;
			// Throttle so every cycle gets a chance to be displayed. Read
			// once: a concurrent setMaxFPS() must not change the divisor
			// between the guard and the division.
			const unsigned int fps = m_maxFPS.load();
			if(fps > 0) {
				const qint64 minMs = 1000LL / fps;
				const qint64 spent = cycleTimer.elapsed();
				if(spent < minMs)
					QThread::msleep(static_cast<unsigned long>(minMs - spent));
			}
		} else {
			// Free-run; only the notification is rate-limited.
			const unsigned int fps = m_maxFPS.load();
			const qint64       hbMs = fps > 0 ? 1000LL / fps : 16;
			if(heartbeatTimer.elapsed() >= hbMs) {
				Q_EMIT cycleComplete();
				heartbeatTimer.restart();
			}
		}

		if(m_acqCount > 0 && ++cyclesDone >= m_acqCount)
			break;
	}

	m_running = false;

	// Continuous mode gates cycleComplete on the heartbeat, so emit a final
	// one to publish whatever the last cycle produced.
	if(m_mode.load() == Mode::Continuous)
		Q_EMIT cycleComplete();

	// Sole owner of onStop(): stop() only clears the flag and waits, so device
	// teardown always happens exactly once, on this thread.
	for(SourceBlock *src : m_sources)
		if(src->isEnabled())
			guarded(src->id(), QStringLiteral("onStop()"), [src] { src->onStop(); });

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

	QSet<ProcessorBlock *> executed;
	QSet<DataKey>          dirty = m_store->cycleKeys();
	QSet<ProcessorBlock *> runLast;
	{
		QMutexLocker lk(&m_blockMutex);
		runLast = m_runLast;
	}

	// A processor that runs writes new keys, which may make another processor
	// runnable — so re-scan until a pass changes nothing. Deferred processors
	// are held back to a final pass so they observe everything the others wrote.
	for(bool deferred : {false, true}) {
		bool progress = true;
		while(progress) {
			progress = false;
			for(ProcessorBlock *proc : m_processors) {
				if(executed.contains(proc) || !proc->isEnabled())
					continue;
				if(runLast.contains(proc) != deferred)
					continue;

				bool ready = true;
				for(const DataKey &k : proc->watchedKeys())
					if(!dirty.contains(k)) {
						ready = false;
						break;
					}
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

	// A processor can legitimately idle for many cycles (e.g. a stacked decoder
	// whose upstream has produced nothing yet), so dedupe rather than warn at
	// cycle rate.
	for(ProcessorBlock *proc : m_processors) {
		if(!proc->isEnabled())
			continue;
		if(executed.contains(proc)) {
			m_lastWarning.remove(proc->name());
			continue;
		}
		QStringList missing;
		for(const DataKey &k : proc->watchedKeys())
			if(!dirty.contains(k))
				missing << k.key;
		reportWarningOnce(proc->name(),
				  QStringLiteral("skipped — missing input keys: ") + missing.join(", "));
	}
	return true;
}

} // namespace acq
} // namespace scopy
