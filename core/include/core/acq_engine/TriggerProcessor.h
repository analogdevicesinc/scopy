#pragma once

#include "scopy-core_export.h"

#include "DataKey.h"
#include "ProcessorBlock.h"
#include "SampleBuffer.h"

#include <QList>
#include <QMap>
#include <QMutex>
#include <QString>
#include <QVector>

namespace scopy {
namespace acq {

// One rule: a key + how to test its samples in the current cycle chunk.
struct TriggerCondition
{
	enum class Edge { Rising, Falling, Either };

	// Edge detection uses the predicate high(v) = v > 0. This works for
	// both unsigned digital lines (quint8 0/1) and signed analog signals
	// (float, bipolar zero-crossing).
	//   Rising : first i in [1,n) with !high(s[i-1]) &&  high(s[i])
	//   Falling: first i in [1,n) with  high(s[i-1]) && !high(s[i])
	//   Either : union of the two
	DataKey key;
	Edge    edge{Edge::Rising};
	bool    enabled{true};
	QString label;
};

// How to combine multiple conditions.
struct CombineRule
{
	enum class Op { AND, OR };
	Op      op{Op::AND};
	quint32 coincidenceWindow{1}; // samples; AND only
};

// ProcessorBlock that evaluates trigger conditions on the newest chunk of each
// watched key. Each acquisition buffer is evaluated in isolation — no history,
// no cross-chunk edge carry. Every condition scan short-circuits at the first
// hit. On fire, emits fired(sampleIndex). Never blocks downstream processors —
// they still run every cycle.
class SCOPY_CORE_EXPORT TriggerProcessor : public ProcessorBlock
{
	Q_OBJECT
public:
	explicit TriggerProcessor(const QString &name, QObject *parent = nullptr);
	~TriggerProcessor() override = default;

	// Condition CRUD. All are thread-safe wrt process().
	void                    addCondition(const TriggerCondition &c);
	void                    removeCondition(int index);
	void                    setCondition(int index, const TriggerCondition &c);
	void                    setConditionEnabled(int index, bool en);
	QList<TriggerCondition> conditions() const;

	void        setCombineRule(CombineRule r);
	CombineRule combineRule() const;

	// Sample-specific mode: instead of scanning the full chunk for hits,
	// evaluate every condition at exactly one sample index within the
	// chunk. Fires iff the combined rule is satisfied at that index (AND =
	// every enabled condition matches at S; OR = at least one matches).
	// Coincidence window is ignored in this mode.
	void    setSampleSpecific(bool on);
	bool    sampleSpecific() const;
	void    setTargetSample(quint32 s);
	quint32 targetSample() const;

	// Sample tolerance for sample-specific mode: fire if any sample in
	// [target - tol, target + tol] satisfies the condition. Reported fire
	// index is always the target (not the actual match), so the trigger
	// cursor stays put. Default 0 = strict single-sample match.
	void    setSampleTolerance(quint32 tol);
	quint32 sampleTolerance() const;

	// ProcessorBlock overrides.
	void     process(DataStore *store) override;
	QWidget *createSettingsWidget(QWidget *parent = nullptr) override;

Q_SIGNALS:
	// Emitted from the engine worker thread; consumers must use QueuedConnection.
	// snapshot contains the newest chunk of every key in the DataStore at
	// the moment of fire, so the GUI can plot the exact fire cycle even if
	// the free-running worker has already advanced past it.
	void fired(quint32 sampleIndex, QMap<QString, scopy::acq::SampleVariant> snapshot);
	void skipped();
	void conditionsChanged();
	void combineRuleChanged();
	void sampleSpecificChanged(bool on);
	void targetSampleChanged(quint32 s);
	void sampleToleranceChanged(quint32 tol);

private:
	void rebuildWatchedKeysLocked();

	mutable QMutex          m_mutex;
	QList<TriggerCondition> m_conditions;
	CombineRule             m_rule;
	bool                    m_sampleSpecific{false};
	quint32                 m_targetSample{0};
	quint32                 m_sampleTolerance{0};
};

} // namespace acq
} // namespace scopy
