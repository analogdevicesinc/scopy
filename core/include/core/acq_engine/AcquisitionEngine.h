#pragma once

#include "scopy-core_export.h"

#include "AcquisitionError.h"
#include "DataStore.h"
#include "ProcessorBlock.h"
#include "SourceBlock.h"

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
// Each cycle: every enabled SourceBlock writes its chunks into the DataStore,
// then every enabled ProcessorBlock whose watchedKeys were all written this
// cycle runs. Processors are re-scanned until none becomes newly runnable, so
// chained processors resolve in dependency order without a declared graph.
//
// Threading: the public API is GUI-thread facing and safe to call while
// running; sources and processors are only ever invoked from the worker.
// Blocks added or removed mid-run take effect at the next cycle boundary, so
// the worker never sees a block list mutate underneath it. All signals are
// emitted from the worker thread — connect with Qt::QueuedConnection.
class SCOPY_CORE_EXPORT AcquisitionEngine : public QObject
{
	Q_OBJECT
public:
	// Continuous: cycleComplete() is rate-limited to maxFPS and the loop
	//   free-runs, so the GUI samples whatever the newest data is.
	// Triggered: one cycleComplete() per cycle, with the loop throttled to
	//   maxFPS, so no cycle goes unseen.
	enum class Mode { Continuous, Triggered };

	explicit AcquisitionEngine(DataStore *store, QObject *parent = nullptr);
	~AcquisitionEngine() override;

	void          addSource(SourceBlock *src);
	void          removeSource(SourceBlock *src);
	QList<SourceBlock *> sources() const;

	void          addProcessor(ProcessorBlock *proc);
	void          removeProcessor(ProcessorBlock *proc);
	QList<ProcessorBlock *> processors() const;

	// --- stream descriptors -----------------------------------------------
	//
	// What a key *means* — label, unit, rate, X source, how to draw it. Asked
	// of the producing block on every call rather than cached anywhere: a
	// block's declaration changes when its channel set changes or a settings
	// widget is touched, and a copy kept elsewhere would need re-syncing at
	// each of those points. Nothing in the engine reads these; they exist for
	// views, which is why this is a plain lookup and not a signal.
	//
	// First declaring block wins on a collision, which is not a real case:
	// keys are namespaced by producer name.
	std::optional<StreamInfo> streamInfo(const DataKey &key) const;

	// Every key any block produces, whether or not anything has been written to
	// it yet. Neither a subset nor a superset of DataStore::keys() — offering a
	// declared-but-unwritten stream is what lets a view create its channel, and
	// so register its depth claim, before the first cycle.
	//
	// The union of the blocks' outputKeys() plus the ramp. There is no separate
	// list of describable keys: streamInfo() above answers for exactly these and
	// nullopt for anything else.
	QList<DataKey> declaredKeys() const;

	// Marks `proc` to be ordered after every non-deferred processor within a
	// cycle, regardless of registration order. For processors that read what
	// the others produced — e.g. a trigger snapshotting the whole store.
	void setRunLast(ProcessorBlock *proc);

	void setMode(Mode m);
	Mode mode() const;

	void        setBufferSize(std::size_t size);
	std::size_t bufferSize() const;

	// --- the shared sample-index ramp -------------------------------------
	//
	// 0,1,2,...,L-1 as Float32, in the store under this key. A view drawing a
	// stream against "the sample index" reads it like any other X stream
	// rather than carrying its own vector and a special case for it.
	//
	// ONE CHUNK, REWRITTEN ONLY WHEN L CHANGES. This is a coordinate system,
	// not acquired data: it costs nothing per cycle, and because it is never
	// written during a cycle it never lands in cycleKeys() and triggers no
	// processor. Being a single chunk is also what makes it correct —
	// SampleBuffer::window() stitches chunks oldest-first, so a ramp written
	// per cycle would read back as a sawtooth for any window wider than one
	// buffer, and every curve on it would fold over itself.
	static DataKey indexRampKey();

	// L. Set by the view to the widest window any plot draws; a narrower plot
	// reads the tail of this one chunk and rebases to 0. Clamped to >= 1, and
	// a no-op when unchanged — which is what keeps the std::iota off the
	// per-frame path. Safe to call while running.
	void        setIndexRampLength(std::size_t n);
	std::size_t indexRampLength() const;

	// The cycleComplete() rate ceiling; 0 means "as fast as the pipeline runs".
	// Safe to call while running — the worker re-reads it every cycle, so a change
	// takes effect on the next one.
	void         setMaxFPS(unsigned int fps);
	unsigned int maxFPS() const;

	bool isRunning() const;

	// Reports below this severity are dropped without being emitted. Blocks
	// check it via Block::wantsReport() so per-cycle diagnostics cost nothing
	// when nobody is listening. Defaults to Warning.
	void                       setMinReportSeverity(AcquisitionError::Severity sev);
	AcquisitionError::Severity minReportSeverity() const;

	// Run until stop(); or for exactly `count` cycles.
	void run();
	void single(unsigned int count = 1);
	void stop();

Q_SIGNALS:
	void started();
	void stopped();
	// Emitted instead of stopped() when a block threw and aborted the run.
	void forceStopped();
	void cycleComplete();
	void error(int severity, const QString &id, const QString &message);
	// A queued add/remove was applied, so sources()/processors() changed. Fires
	// on the worker thread while running, on the caller's thread when stopped.
	void blocksChanged();

private:
	void startLoop(int acqCount);
	void loop();

	// Joins and destroys the worker thread. Idempotent.
	void joinThread();

	// Applies queued add/remove requests. Worker thread, between cycles.
	void syncBlocks();

	// Sources and processors as one flat list, copied under m_blockMutex so the
	// caller can iterate it — and call into the blocks — with the lock released.
	// A block takes its own lock in there, and holding two at once is how
	// deadlocks are built.
	QList<Block *> blockSnapshot() const;

	// Writes m_indexRamp into the store, rebuilding it first if `n` differs from
	// its current length. Called from setIndexRampLength() and from startLoop():
	// DataStore::clear() drops chunks and reset() drops the descriptor too, so
	// the ramp has to be re-established at every start.
	void publishIndexRamp(std::size_t n);

	// Runs every processor whose inputs are ready, repeating until no further
	// progress. Returns false if a processor threw.
	bool runProcessors();

	// Invokes fn, converting any exception into a Critical error report.
	// Returns false if it threw.
	template<class Fn>
	bool guarded(const QString &id, const QString &what, Fn &&fn);

	void reportInfo(const QString &id, const QString &msg);

	// Emits a Warning only when this (id, msg) pair differs from the last one
	// seen for `id`, so a condition that persists across cycles is reported
	// once rather than at cycle rate. Cleared at the start of each run.
	void reportWarningOnce(const QString &id, const QString &msg);

	DataStore *m_store;

	// Live lists, worker-owned; mutated only from syncBlocks().
	QList<SourceBlock *>    m_sources;
	QList<ProcessorBlock *> m_processors;

	// Pending mutations from other threads, guarded by m_blockMutex.
	mutable QMutex          m_blockMutex;
	QList<SourceBlock *>    m_pendingAddSources, m_pendingRemoveSources;
	QList<ProcessorBlock *> m_pendingAddProcessors, m_pendingRemoveProcessors;
	QSet<ProcessorBlock *>  m_runLast;

	QThread *m_thread{nullptr};

	// m_running gates the loop; m_threadAlive tracks whether the worker is
	// still executing. They differ while the worker runs its teardown tail
	// after a self-terminated run, which is exactly when stop() must wait.
	std::atomic<bool> m_running{false};
	std::atomic<bool> m_threadAlive{false};
	std::atomic<bool> m_faultStop{false};
	std::atomic<Mode> m_mode{Mode::Triggered};
	std::atomic<AcquisitionError::Severity> m_minSeverity{AcquisitionError::Severity::Warning};

	int         m_acqCount{0};
	std::size_t m_bufferSize{1024};
	// Atomic, unlike the two above: a target frame rate is something a reader
	// adjusts while watching the plot, so the worker re-reads it each cycle rather
	// than latching it at run().
	std::atomic<unsigned int> m_maxFPS{0};

	// Worker-only: last warning emitted per block id, for reportWarningOnce().
	QHash<QString, QString> m_lastWarning;

	// The ramp and its length, guarded by m_rampMutex: setIndexRampLength() is
	// called from the GUI thread while startLoop()'s re-publish can run from
	// either. Kept as a member so the store's copy is a refcount bump; the next
	// resize detaches it.
	mutable QMutex m_rampMutex;
	QVector<float> m_indexRamp;

	// Long enough that a view which never sets a length still draws: the default
	// plot window. Only the initial value — the view states its real requirement
	// through setIndexRampLength().
	const std::size_t m_defaultIndexRampLength;
};

} // namespace acq
} // namespace scopy
