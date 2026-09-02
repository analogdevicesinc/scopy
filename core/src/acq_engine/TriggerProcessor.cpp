#include "TriggerProcessor.h"
#include "TriggerProcessorWidget.h"

#include "DataStore.h"

#include <algorithm>
#include <climits>
#include <cmath>
#include <optional>
#include <type_traits>
#include <variant>
#include <QSet>
#include <QVarLengthArray>
#include <QWidget>

namespace scopy {
namespace acq {

TriggerProcessor::TriggerProcessor(const QString &name, QObject *parent)
	: ProcessorBlock(name, parent)
{
	qRegisterMetaType<QMap<QString, scopy::acq::SampleVariant>>(
		"QMap<QString,scopy::acq::SampleVariant>");
}

void TriggerProcessor::addCondition(const TriggerCondition &c)
{
	{
		QMutexLocker lk(&m_mutex);
		m_conditions.append(c);
		rebuildWatchedKeysLocked();
	}
	Q_EMIT conditionsChanged();
}

void TriggerProcessor::removeCondition(int index)
{
	{
		QMutexLocker lk(&m_mutex);
		if(index < 0 || index >= m_conditions.size())
			return;
		m_conditions.removeAt(index);
		rebuildWatchedKeysLocked();
	}
	Q_EMIT conditionsChanged();
}

void TriggerProcessor::setCondition(int index, const TriggerCondition &c)
{
	{
		QMutexLocker lk(&m_mutex);
		if(index < 0 || index >= m_conditions.size())
			return;
		const bool keyChanged = (m_conditions[index].key != c.key);
		m_conditions[index] = c;
		if(keyChanged)
			rebuildWatchedKeysLocked();
	}
	Q_EMIT conditionsChanged();
}

void TriggerProcessor::setConditionEnabled(int index, bool en)
{
	{
		QMutexLocker lk(&m_mutex);
		if(index < 0 || index >= m_conditions.size())
			return;
		if(m_conditions[index].enabled == en)
			return;
		m_conditions[index].enabled = en;
	}
	Q_EMIT conditionsChanged();
}

QList<TriggerCondition> TriggerProcessor::conditions() const
{
	QMutexLocker lk(&m_mutex);
	return m_conditions;
}

void TriggerProcessor::setCombineRule(CombineRule r)
{
	{
		QMutexLocker lk(&m_mutex);
		m_rule = r;
	}
	Q_EMIT combineRuleChanged(); // CombineRule has no operator==, so always notify
}

void TriggerProcessor::setSampleSpecific(bool on)
{
	if(assign(m_sampleSpecific, on))
		Q_EMIT sampleSpecificChanged(on);
}

void TriggerProcessor::setTargetSample(quint32 s)
{
	if(assign(m_targetSample, s))
		Q_EMIT targetSampleChanged(s);
}

void TriggerProcessor::setSampleTolerance(quint32 tol)
{
	if(assign(m_sampleTolerance, tol))
		Q_EMIT sampleToleranceChanged(tol);
}

void TriggerProcessor::setWindowSize(int n)          { assign(m_windowSize, std::max(0, n)); }
void TriggerProcessor::setTriggerPosition(double f)  { assign(m_triggerPosition, std::clamp(f, 0.0, 1.0)); }

CombineRule TriggerProcessor::combineRule()     const { return get(m_rule); }
bool        TriggerProcessor::sampleSpecific()  const { return get(m_sampleSpecific); }
quint32     TriggerProcessor::targetSample()    const { return get(m_targetSample); }
quint32     TriggerProcessor::sampleTolerance() const { return get(m_sampleTolerance); }
int         TriggerProcessor::windowSize()      const { return get(m_windowSize); }
double      TriggerProcessor::triggerPosition() const { return get(m_triggerPosition); }

// Drops any latched fire so a new run never emits a window half-filled from the
// previous one. Called by AcquisitionEngine::startLoop.
void TriggerProcessor::reset() { m_postWanted = -1; }

void TriggerProcessor::rebuildWatchedKeysLocked()
{
	// Called under m_mutex. Deduplicate keys of enabled conditions.
	QList<DataKey> keys;
	QSet<QString>  seen;
	for(const TriggerCondition &c : m_conditions) {
		if(!c.enabled)
			continue;
		if(seen.contains(c.key.key))
			continue;
		seen.insert(c.key.key);
		keys.append(c.key);
	}
	m_watchedKeys = keys;
}

// --- Evaluation ----------------------------------------------------------
//
// One edge detector serves both analog and digital streams: the predicate
// high(v) = v > 0 covers unsigned 0/1 lines and bipolar float zero-crossings
// alike. Evaluation is confined to the current chunk — no allocations, no
// cross-chunk carry — and every scan stops at the first hit.

namespace {

// Does an edge of the requested kind occur between samples i-1 and i?
template<class Vec>
bool isEdgeAt(const Vec &s, int i, TriggerCondition::Edge edge)
{
	using T = typename Vec::value_type;
	const bool prev = s[i - 1] > T{0};
	const bool curr = s[i]     > T{0};
	switch(edge) {
	case TriggerCondition::Edge::Rising:  return !prev &&  curr;
	case TriggerCondition::Edge::Falling: return  prev && !curr;
	case TriggerCondition::Edge::Either:  return  prev != curr;
	}
	return false;
}

// Runs `fn` on the chunk as its concrete vector type. Annotation streams are
// not triggerable, so they yield `absent` without invoking fn.
template<class R, class Fn>
R withNumericChunk(const SampleVariant &v, R absent, Fn &&fn)
{
	return std::visit([&](const auto &vec) -> R {
		if constexpr(std::is_same_v<std::decay_t<decltype(vec)>, QVector<Annotation>>)
			return absent;
		else
			return fn(vec);
	}, v);
}

// Index of the first edge in the chunk, or -1.
int firstEdge(const SampleVariant &v, TriggerCondition::Edge edge)
{
	return withNumericChunk<int>(v, -1, [&](const auto &s) {
		for(int i = 1; i < s.size(); ++i)
			if(isEdgeAt(s, i, edge))
				return i;
		return -1;
	});
}

// Is there an edge anywhere in [lo, hi]? Indices outside the chunk are ignored.
bool edgeInRange(const SampleVariant &v, TriggerCondition::Edge edge, int lo, int hi)
{
	return withNumericChunk<bool>(v, false, [&](const auto &s) {
		const int from = std::max(1, lo);
		const int to   = std::min<int>(hi, s.size() - 1);
		for(int i = from; i <= to; ++i)
			if(isEdgeAt(s, i, edge))
				return true;
		return false;
	});
}

} // namespace

void TriggerProcessor::process(DataStore *store)
{
	// Snapshot config under mutex; do heavy work unlocked.
	QList<TriggerCondition> conds;
	CombineRule             rule;
	bool                    sampleSpecific;
	quint32                 targetSample;
	quint32                 sampleTolerance;
	int                     L;
	double                  frac;
	{
		QMutexLocker lk(&m_mutex);
		conds           = m_conditions;
		rule            = m_rule;
		sampleSpecific  = m_sampleSpecific;
		targetSample    = m_targetSample;
		sampleTolerance = m_sampleTolerance;
		L               = m_windowSize;
		frac            = m_triggerPosition;
	}

	// Window split: `pre` samples of history sit left of the firing sample,
	// `post` at or after it. Both are 0 when no window is configured.
	const int pre  = L > 0 ? std::clamp(static_cast<int>(std::lround(frac * L)), 0, L - 1) : 0;
	const int post = L - pre;

	// Samples this cycle contributed, taken from the first watched stream that
	// has data. Every source writes bufferSize samples per cycle, so one is
	// representative.
	int cycleLen = 0;
	for(const TriggerCondition &c : conds) {
		if(!c.enabled)
			continue;
		if(const auto ch = store->latest(c.key)) {
			cycleLen = std::visit(
				[](const auto &v) { return static_cast<int>(v.size()); }, *ch);
			break;
		}
	}

	// Capture on the worker thread, before the next cycle overwrites anything:
	// one assembled window per key, re-anchored so the firing sample lands at
	// `pre`. `extra` is how far the newest sample sits past the emitted window's
	// right edge.
	auto emitFire = [&](quint32 atSample, int extra) {
		QMap<QString, SampleVariant> snapshot;
		for(const DataKey &k : store->keys()) {
			if(L <= 0) {
				if(auto chunk = store->latest(k))
					snapshot.insert(k.key, std::move(*chunk));
			} else if(store->typeOf(k) == SampleType::Annotation) {
				// Annotations are produced against their own
				// right-anchored window of length L, so re-anchoring
				// shifts their offsets right rather than truncating.
				snapshot.insert(k.key,
					shiftAnnotations(store->window(k, L), L, extra));
			} else {
				snapshot.insert(k.key,
					truncateWindow(store->window(k, L + extra), L));
			}
		}
		m_postWanted = -1;
		Q_EMIT fired(atSample, std::move(snapshot));
	};

	// A latched fire is waiting for its post-trigger half. Ignore further edges
	// until it completes (holdoff), so every window is centred on the fire that
	// produced it.
	if(m_postWanted >= 0) {
		if(L <= 0) {
			m_postWanted = -1; // window disabled mid-wait
		} else {
			m_postWanted -= cycleLen;
			if(m_postWanted <= 0)
				emitFire(static_cast<quint32>(pre), -m_postWanted);
			else
				Q_EMIT skipped();
			return;
		}
	}

	if(sampleSpecific) {
		// targetSample is a window index, so evaluate over the window that
		// will be emitted rather than over the newest chunk.
		auto evalChunk = [&](const DataKey &k) -> std::optional<SampleVariant> {
			if(L <= 0)
				return store->latest(k);
			if(!store->contains(k))
				return std::nullopt;
			return store->window(k, L);
		};

		// Match if any sample in [S - tol, S + tol] satisfies the condition.
		// The reported fire index is always the target S, so the plot handle
		// stays put regardless of where inside the window the edge landed.
		const int S   = static_cast<int>(targetSample);
		const int tol = static_cast<int>(sampleTolerance);
		int  enabledCount      = 0;
		int  matchCount        = 0;
		bool anyChunkAvailable = false;
		for(const TriggerCondition &c : conds) {
			if(!c.enabled) continue;
			++enabledCount;
			const auto chunk = evalChunk(c.key);
			if(!chunk) continue;
			anyChunkAvailable = true;
			if(edgeInRange(*chunk, c.edge, S - tol, S + tol))
				++matchCount;
		}
		const bool fireNow =
			enabledCount > 0 && anyChunkAvailable &&
			(rule.op == CombineRule::Op::AND ? matchCount == enabledCount
							 : matchCount > 0);
		// The target is a fixed position in the right-anchored window, so
		// there is nothing to wait for: emit as-is.
		if(fireNow) emitFire(targetSample, 0);
		else        Q_EMIT skipped();
		return;
	}

	// Scan mode: first hit per enabled condition, over the newest chunk only.
	// Scanning the assembled window instead would re-detect the same historical
	// edge every cycle.
	QVarLengthArray<int, 8> firsts;
	int enabledCount = 0;
	for(const TriggerCondition &c : conds) {
		if(!c.enabled) continue;
		++enabledCount;
		const auto chunk = store->latest(c.key);
		firsts.append(chunk ? firstEdge(*chunk, c.edge) : -1);
	}

	if(enabledCount == 0) { Q_EMIT skipped(); return; }

	int fireAt = -1;
	if(rule.op == CombineRule::Op::OR) {
		for(int h : firsts)
			if(h >= 0 && (fireAt < 0 || h < fireAt))
				fireAt = h;
	} else {
		// AND with coincidence window W: fire iff every enabled
		// condition has a first-hit in the same [lo, lo+W) window.
		const quint32 W = rule.coincidenceWindow == 0 ? 1 : rule.coincidenceWindow;
		int hi = -1, lo = INT_MAX;
		bool all = true;
		for(int h : firsts) {
			if(h < 0) { all = false; break; }
			if(h < lo) lo = h;
			if(h > hi) hi = h;
		}
		if(all && static_cast<quint32>(hi - lo) < W)
			fireAt = lo;
	}

	if(fireAt < 0) {
		Q_EMIT skipped();
		return;
	}

	if(L <= 0) {
		emitFire(static_cast<quint32>(fireAt), 0);
		return;
	}

	// fireAt indexes the newest chunk, which sits at the window's right end, so
	// the samples already acquired at or after the fire are the chunk's tail.
	const int avail = cycleLen - fireAt;
	if(avail >= post) {
		emitFire(static_cast<quint32>(pre), avail - post);
	} else {
		// Latch: wait for the rest of the post-trigger half to arrive.
		m_postWanted = post - avail;
		Q_EMIT skipped();
	}
}

QWidget *TriggerProcessor::createSettingsWidget(QWidget *parent)
{
	return withBaseSettings(new TriggerProcessorWidget(this), parent);
}

} // namespace acq
} // namespace scopy

#include "moc_TriggerProcessor.cpp"
