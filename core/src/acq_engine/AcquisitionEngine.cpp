#include "AcquisitionEngine.h"

#include <exception>
#include <QElapsedTimer>
#include <QStringList>

namespace scopy {
namespace acq {

AcquisitionEngine::AcquisitionEngine(DataStore *store, QObject *parent)
	: QObject(parent)
	, m_store(store)
{
	qRegisterMetaType<scopy::acq::DataKey>("scopy::acq::DataKey");
}

AcquisitionEngine::~AcquisitionEngine()
{
	stop();
}

void AcquisitionEngine::addSource(SourceBlock *src)    { m_sources.append(src); }
void AcquisitionEngine::removeSource(SourceBlock *src) { m_sources.removeOne(src); }
const QList<SourceBlock *> &AcquisitionEngine::sources() const { return m_sources; }

void AcquisitionEngine::addProcessor(ProcessorBlock *proc)    { m_processors.append(proc); }
void AcquisitionEngine::removeProcessor(ProcessorBlock *proc) { m_processors.removeOne(proc); }
const QList<ProcessorBlock *> &AcquisitionEngine::processors() const { return m_processors; }

bool        AcquisitionEngine::isRunning()  const { return m_running; }
void        AcquisitionEngine::setBufferSize(std::size_t size) { m_bufferSize = size; }
std::size_t AcquisitionEngine::bufferSize() const { return m_bufferSize; }
void        AcquisitionEngine::setMaxFPS(unsigned int fps) { m_maxFPS = fps; }
unsigned int AcquisitionEngine::maxFPS()   const { return m_maxFPS; }
void             AcquisitionEngine::setMode(Mode m) { m_mode.store(m); }
AcquisitionEngine::Mode AcquisitionEngine::mode()  const { return m_mode.load(); }

void AcquisitionEngine::single(unsigned int count) { startLoop(static_cast<int>(count)); }
void AcquisitionEngine::run()                      { startLoop(0); }

void AcquisitionEngine::stop()
{
	if(!m_running)
		return;

	m_running = false;

	for(SourceBlock *src : m_sources) {
		if(!src->isEnabled())
			continue;
		safeOnStop(src);
	}

	if(m_thread) {
		m_thread->wait();
		m_thread = nullptr;
	}
}

void AcquisitionEngine::startLoop(int acqCount)
{
	if(m_running)
		return;

	m_acqCount  = acqCount;
	m_faultStop = false;

	for(SourceBlock *src : m_sources) {
		if(!src->isEnabled())
			continue;
		src->setBufferSize(m_bufferSize);
		try {
			src->onStart();
		} catch(const std::exception &e) {
			Q_EMIT error(static_cast<int>(AcquisitionError::Severity::Critical),
				     src->id(), QString::fromStdString(e.what()));
			m_faultStop = true;
		} catch(...) {
			Q_EMIT error(static_cast<int>(AcquisitionError::Severity::Critical),
				     src->id(), QStringLiteral("unknown exception in onStart()"));
			m_faultStop = true;
		}
	}

	if(m_faultStop) {
		Q_EMIT forceStopped();
		return;
	}

	for(ProcessorBlock *proc : m_processors)
		proc->reset();

	m_running = true;
	m_thread  = QThread::create([this] { loop(); });
	connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
	m_thread->start();
}

void AcquisitionEngine::loop()
{
	Q_EMIT started();

	{
		QStringList srcNames, procNames;
		for(SourceBlock *s : m_sources)       srcNames  << s->id();
		for(ProcessorBlock *p : m_processors) procNames << p->name();
		Q_EMIT error(static_cast<int>(AcquisitionError::Severity::Info),
			     QStringLiteral("engine"),
			     QString("started | sources: [%1] | processors: [%2] | buffer: %3")
				     .arg(srcNames.join(", "),
					  procNames.isEmpty() ? "none" : procNames.join(", "))
				     .arg(m_bufferSize));
	}

	int cyclesDone = 0;
	QElapsedTimer cycleTimer;
	QElapsedTimer heartbeatTimer;
	heartbeatTimer.start();

	while(m_running) {
		cycleTimer.start();

		m_store->beginCycle();

		bool aborted = false;
		for(SourceBlock *src : m_sources) {
			if(!src->isEnabled())
				continue;
			const QSet<DataKey> beforeAcq = m_store->cycleKeys();
			try {
				src->acquire(m_store);
			} catch(const std::exception &e) {
				Q_EMIT error(static_cast<int>(AcquisitionError::Severity::Critical),
					     src->id(), QString::fromStdString(e.what()));
				m_faultStop = true;
				aborted = true;
				break;
			} catch(...) {
				Q_EMIT error(static_cast<int>(AcquisitionError::Severity::Critical),
					     src->id(), QStringLiteral("unknown exception in acquire()"));
				m_faultStop = true;
				aborted = true;
				break;
			}
			if(m_store->cycleKeys() == beforeAcq)
				Q_EMIT error(static_cast<int>(AcquisitionError::Severity::Warning),
					     src->id(), QStringLiteral("acquire() wrote no data this cycle"));
			if(!m_running) { aborted = true; break; }
		}

		if(aborted) { m_running = false; break; }

		if(m_store->cycleKeys().isEmpty())
			Q_EMIT error(static_cast<int>(AcquisitionError::Severity::Warning),
				     QStringLiteral("engine"),
				     QStringLiteral("no source produced any data this cycle"));

		if(runProcessors()) { m_running = false; break; }

		if(m_mode.load() == Mode::Triggered) {
			Q_EMIT cycleComplete();
			if(!m_running)
				break;
			if(m_maxFPS > 0) {
				const qint64 minMs   = 1000LL / static_cast<qint64>(m_maxFPS);
				const qint64 elapsed = cycleTimer.elapsed();
				if(elapsed < minMs)
					QThread::msleep(static_cast<unsigned long>(minMs - elapsed));
			}
		} else {
			const qint64 hbMs = (m_maxFPS > 0)
				? 1000LL / static_cast<qint64>(m_maxFPS)
				: 16;
			if(heartbeatTimer.elapsed() >= hbMs) {
				Q_EMIT cycleComplete();
				heartbeatTimer.restart();
			}
			if(!m_running)
				break;
		}

		++cyclesDone;
		if(m_acqCount > 0 && cyclesDone >= m_acqCount)
			m_running = false;
	}

	if(m_mode.load() == Mode::Continuous)
		Q_EMIT cycleComplete();

	for(SourceBlock *src : m_sources) {
		if(!src->isEnabled())
			continue;
		safeOnStop(src);
	}

	if(m_faultStop)
		Q_EMIT forceStopped();
	else
		Q_EMIT stopped();
	Q_EMIT error(static_cast<int>(AcquisitionError::Severity::Info),
		     QStringLiteral("engine"), QStringLiteral("stopped"));
}

bool AcquisitionEngine::runProcessors()
{
	if(m_processors.isEmpty())
		return false;

	QSet<ProcessorBlock *> executed;
	bool progress;

	do {
		progress = false;
		const QSet<DataKey> dirty = m_store->cycleKeys();

		for(ProcessorBlock *proc : m_processors) {
			if(executed.contains(proc) || !proc->isEnabled())
				continue;

			bool ready = true;
			for(const DataKey &k : proc->watchedKeys()) {
				if(!dirty.contains(k)) { ready = false; break; }
			}
			if(!ready)
				continue;

			try {
				proc->process(m_store);
			} catch(const std::exception &e) {
				Q_EMIT error(static_cast<int>(AcquisitionError::Severity::Critical),
					     proc->name(), QString::fromStdString(e.what()));
				m_faultStop = true;
				return true;
			} catch(...) {
				Q_EMIT error(static_cast<int>(AcquisitionError::Severity::Critical),
					     proc->name(), QStringLiteral("unknown exception in process()"));
				m_faultStop = true;
				return true;
			}

			executed.insert(proc);
			progress = true;
		}
	} while(progress);

	const QSet<DataKey> finalDirty = m_store->cycleKeys();
	for(ProcessorBlock *proc : m_processors) {
		if(executed.contains(proc) || !proc->isEnabled())
			continue;
		QStringList missing;
		for(const DataKey &k : proc->watchedKeys())
			if(!finalDirty.contains(k))
				missing << k.key;
		Q_EMIT error(static_cast<int>(AcquisitionError::Severity::Warning),
			     proc->name(),
			     "skipped — missing input keys: " + missing.join(", "));
	}
	return false;
}

void AcquisitionEngine::safeOnStop(SourceBlock *src)
{
	try {
		src->onStop();
	} catch(const std::exception &e) {
		Q_EMIT error(static_cast<int>(AcquisitionError::Severity::Critical),
			     src->id(), QString::fromStdString(e.what()));
	} catch(...) {
		Q_EMIT error(static_cast<int>(AcquisitionError::Severity::Critical),
			     src->id(), QStringLiteral("unknown exception in onStop()"));
	}
}

} // namespace acq
} // namespace scopy
