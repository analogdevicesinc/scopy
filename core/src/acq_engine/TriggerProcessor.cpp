#include "TriggerProcessor.h"
#include "TriggerProcessorWidget.h"

#include "DataStore.h"

#include <climits>
#include <type_traits>
#include <variant>
#include <QSet>
#include <QVarLengthArray>
#include <QVBoxLayout>
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
	Q_EMIT combineRuleChanged();
}

CombineRule TriggerProcessor::combineRule() const
{
	QMutexLocker lk(&m_mutex);
	return m_rule;
}

void TriggerProcessor::setSampleSpecific(bool on)
{
	{
		QMutexLocker lk(&m_mutex);
		if(m_sampleSpecific == on)
			return;
		m_sampleSpecific = on;
	}
	Q_EMIT sampleSpecificChanged(on);
}

bool TriggerProcessor::sampleSpecific() const
{
	QMutexLocker lk(&m_mutex);
	return m_sampleSpecific;
}

void TriggerProcessor::setTargetSample(quint32 s)
{
	{
		QMutexLocker lk(&m_mutex);
		if(m_targetSample == s)
			return;
		m_targetSample = s;
	}
	Q_EMIT targetSampleChanged(s);
}

quint32 TriggerProcessor::targetSample() const
{
	QMutexLocker lk(&m_mutex);
	return m_targetSample;
}

void TriggerProcessor::setSampleTolerance(quint32 tol)
{
	{
		QMutexLocker lk(&m_mutex);
		if(m_sampleTolerance == tol)
			return;
		m_sampleTolerance = tol;
	}
	Q_EMIT sampleToleranceChanged(tol);
}

quint32 TriggerProcessor::sampleTolerance() const
{
	QMutexLocker lk(&m_mutex);
	return m_sampleTolerance;
}

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
// Single edge detector, same algorithm for both analog and digital streams.
// Predicate high(v) = v > 0 (works for both unsigned quint8 0/1 and signed
// bipolar float zero-crossing). Operates ONLY on the current acquisition
// buffer, stops at the first hit, no allocations, no cross-chunk state.
//
//   Rising : first i in [1,n) with !high(s[i-1]) &&  high(s[i])
//   Falling: first i in [1,n) with  high(s[i-1]) && !high(s[i])
//   Either : union of the two

namespace {

template<class Vec>
int firstEdge(const Vec &s, TriggerCondition::Edge edge)
{
	const int n = s.size();
	using T = typename Vec::value_type;
	auto high = [](T v) -> bool { return v > T{0}; };
	switch(edge) {
	case TriggerCondition::Edge::Rising:
		for(int i = 1; i < n; ++i)
			if(!high(s[i - 1]) &&  high(s[i])) return i;
		return -1;
	case TriggerCondition::Edge::Falling:
		for(int i = 1; i < n; ++i)
			if( high(s[i - 1]) && !high(s[i])) return i;
		return -1;
	case TriggerCondition::Edge::Either:
		for(int i = 1; i < n; ++i)
			if(high(s[i - 1]) != high(s[i])) return i;
		return -1;
	}
	return -1;
}

// Dispatch on the SampleVariant alternative, run the typed loop.
int evalConditionFirstHit(const TriggerCondition &c, const SampleVariant &v)
{
	return std::visit([&](const auto &vec) -> int {
		using VecT = std::decay_t<decltype(vec)>;
		if constexpr(std::is_same_v<VecT, QVector<Annotation>>)
			return -1; // annotations are not triggerable
		else
			return firstEdge(vec, c.edge);
	}, v);
}

// Evaluate at a specific sample index S. Edge condition requires S >= 1.
bool evalConditionAt(const TriggerCondition &c, const SampleVariant &v, int S)
{
	return std::visit([&](const auto &vec) -> bool {
		using VecT = std::decay_t<decltype(vec)>;
		if constexpr(std::is_same_v<VecT, QVector<Annotation>>) {
			return false;
		} else {
			const int n = vec.size();
			if(S < 1 || S >= n) return false;
			using T = typename VecT::value_type;
			auto high = [](T v) -> bool { return v > T{0}; };
			const bool p = high(vec[S - 1]), cu = high(vec[S]);
			switch(c.edge) {
			case TriggerCondition::Edge::Rising:  return !p &&  cu;
			case TriggerCondition::Edge::Falling: return  p && !cu;
			case TriggerCondition::Edge::Either:  return  p != cu;
			}
			return false;
		}
	}, v);
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
	{
		QMutexLocker lk(&m_mutex);
		conds           = m_conditions;
		rule            = m_rule;
		sampleSpecific  = m_sampleSpecific;
		targetSample    = m_targetSample;
		sampleTolerance = m_sampleTolerance;
	}

	auto emitFire = [&](quint32 atSample) {
		// Capture the newest chunk of every DataStore key right now, on
		// the worker thread, before the next cycle overwrites anything.
		QMap<QString, SampleVariant> snapshot;
		for(const DataKey &k : store->keys()) {
			const SampleBuffer buf = store->read(k);
			if(!buf.empty())
				snapshot.insert(k.key, buf.sample(0));
		}
		Q_EMIT fired(atSample, std::move(snapshot));
	};

	if(sampleSpecific) {
		// Match if any sample in [S - tol, S + tol] satisfies the
		// condition. Fire index reported is always the target S so the
		// plot handle stays put.
		const int S   = static_cast<int>(targetSample);
		const int tol = static_cast<int>(sampleTolerance);
		const int lo  = S - tol < 0 ? 0 : S - tol;
		const int hi  = S + tol; // upper bound is index-clamped inside evalConditionAt
		int enabledCount = 0;
		int matchCount   = 0;
		bool anyChunkAvailable = false;
		for(const TriggerCondition &c : conds) {
			if(!c.enabled) continue;
			++enabledCount;
			const SampleBuffer buf = store->read(c.key);
			if(buf.empty()) continue;
			anyChunkAvailable = true;
			bool matched = false;
			for(int i = lo; i <= hi; ++i) {
				if(evalConditionAt(c, buf.sample(0), i)) {
					matched = true;
					break;
				}
			}
			if(matched)
				++matchCount;
		}
		const bool fireNow =
			(enabledCount > 0) && anyChunkAvailable &&
			(rule.op == CombineRule::Op::AND
				? (matchCount == enabledCount)
				: (matchCount > 0));
		if(fireNow) emitFire(targetSample);
		else        Q_EMIT skipped();
		return;
	}

	// Scan mode: first hit per enabled condition.
	QVarLengthArray<int, 8> firsts;
	int enabledCount = 0;
	for(const TriggerCondition &c : conds) {
		if(!c.enabled) continue;
		++enabledCount;
		const SampleBuffer buf = store->read(c.key);
		if(buf.empty()) { firsts.append(-1); continue; }
		firsts.append(evalConditionFirstHit(c, buf.sample(0)));
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

	if(fireAt >= 0) emitFire(static_cast<quint32>(fireAt));
	else            Q_EMIT skipped();
}

QWidget *TriggerProcessor::createSettingsWidget(QWidget *parent)
{
	auto *w   = new QWidget(parent);
	auto *lay = new QVBoxLayout(w);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(4);
	lay->addWidget(ProcessorBlock::createSettingsWidget(w));
	lay->addWidget(new TriggerProcessorWidget(this, w));
	return w;
}

} // namespace acq
} // namespace scopy
