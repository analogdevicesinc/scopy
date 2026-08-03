#pragma once

#include "scopy-core_export.h"

#include "DataKey.h"
#include "ProcessorBlock.h"
#include "SampleBuffer.h"

#include <QList>
#include <QMap>
#include <QMutex>
#include <QMutexLocker>
#include <QString>
#include <QVector>

namespace scopy {
namespace acq {

// One rule: a key + how to test its samples in the current cycle chunk.
struct TriggerCondition
{
	enum class Edge { Rising, Falling, Either };

	// A sample counts as high when v > 0, which works for both unsigned
	// digital lines and bipolar analog zero-crossings.
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
// hit. Never blocks downstream processors — they still run every cycle.
//
// With a window size set (setWindowSize), a fire is *latched* rather than emitted
// immediately: the processor waits until enough post-trigger samples have
// accumulated to fill triggerPosition() of the window, then emits one complete
// window centred on the firing sample. Further edges are ignored while latched
// (holdoff), so every emitted window is centred on the fire that produced it.
// Both fired()'s index and targetSample are plot-window indices in this mode.
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

	// Samples to assemble per fire, spanning as many chunks as the stream's
	// claimed depth allows. 0 = emit the newest chunk only (legacy behaviour,
	// no latching).
	void setWindowSize(int n);
	int  windowSize() const;

	// Where the firing sample sits in the emitted window, 0 = left edge,
	// 1 = right edge. Determines the pre/post-trigger split. Default 0.5.
	void   setTriggerPosition(double frac);
	double triggerPosition() const;

	// ProcessorBlock overrides.
	void     process(DataStore *store) override;
	void     reset() override;
	QWidget *createSettingsWidget(QWidget *parent = nullptr) override;

Q_SIGNALS:
	// Emitted from the engine worker thread; consumers must use QueuedConnection.
	// snapshot holds one assembled window per DataStore key, captured at emit
	// time, so the GUI plots exactly the fire's window even if the free-running
	// worker has already advanced past it. sampleIndex indexes into those
	// windows (or into the newest chunk when windowSize() == 0).
	void fired(quint32 sampleIndex, QMap<QString, scopy::acq::SampleVariant> snapshot);
	void skipped();
	void conditionsChanged();
	void combineRuleChanged();
	void sampleSpecificChanged(bool on);
	void targetSampleChanged(quint32 s);
	void sampleToleranceChanged(quint32 tol);

private:
	void rebuildWatchedKeysLocked();

	// Assigns under m_mutex, returning true if the value actually changed, so
	// setters emit only on a real edit.
	template<class T>
	bool assign(T &field, const T &value)
	{
		QMutexLocker lk(&m_mutex);
		if(field == value)
			return false;
		field = value;
		return true;
	}

	template<class T>
	T get(const T &field) const
	{
		QMutexLocker lk(&m_mutex);
		return field;
	}

	mutable QMutex          m_mutex;
	QList<TriggerCondition> m_conditions;
	CombineRule             m_rule;
	bool                    m_sampleSpecific{false};
	quint32                 m_targetSample{0};
	quint32                 m_sampleTolerance{0};
	int                     m_windowSize{0};
	double                  m_triggerPosition{0.5};

	// Latched fire, worker thread only. -1 = idle; otherwise the count of
	// post-trigger samples still needed before the window can be emitted.
	int m_postWanted{-1};
};

} // namespace acq
} // namespace scopy
