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

#ifndef ACQAXIS_H
#define ACQAXIS_H

#include <core/acq_engine/AcquisitionEngine.h>
#include <core/acq_engine/DataKey.h>
// Not forward-declared: plotAxis() dereferences a QPointer<PlotAxis> inline, and
// QPointer::data() needs the complete type to downcast from QObject.
#include <gui/plotaxis.h>

#include <QObject>
#include <QPointer>
#include <QString>
#include <QtGlobal>

class QWidget;

namespace scopy {
class InstrumentTemplate;
class MenuCombo;
class MenuOnOffSwitch;
class PlotChannel;

namespace gui {
class MenuPlotAxisRangeControl;
class PlotAutoscaler;
} // namespace gui

namespace acq {
class DataStore;
}

namespace adc {

// One axis of one channel — and there is deliberately no difference between X and Y.
//
// Both sides are this class, allocated at a different QwtAxis position, with the same
// four controls behind them: where the values come from, autoscale, a manual
// min/max, and the unit that labels it. A caller that wants to
// know which side it is holding asks isHorizontal(); nothing here branches on it
// except the one autoscaler flag that PlotAutoscaler itself needs.
//
// THE AXIS IS BORROWED, NEVER OWNED. The PlotAxis comes from AcqPlot's pool and goes
// back to it when the channel detaches, because Qwt has no axis-removal path:
// PlotWidget's removePlotAxis is commented out as "not supported by Qwt"
// (gui/include/gui/plotwidget.h:73), PlotAxis's destructor does not unregister it
// from PlotWidget::m_plotAxis, and PlotNavigator has addAxis but no removeAxis. So an
// axis is created once, hidden and reused for the channel's whole plot's life. This
// object owns none of it and must never delete it.
//
// The *source* is mutable where the channel's own key is not. A channel is its Y key
// for life (that is what lets its settings page be built once), but which stream
// supplies the numbers an axis is scaled against is a view decision, and retargeting
// it is a re-claim rather than a rebuild — hence sourceChanged(), which the manager
// answers by moving the depth claim.
class AcqAxis : public QObject
{
	Q_OBJECT
public:
	// Where an axis's values come from.
	//
	// All three modes read a real store stream — the sample index is
	// AcquisitionEngine::indexRampKey(), written by the engine — so the *key* no longer
	// distinguishes them and an empty key no longer means "sample index". What the mode
	// selects is how the numbers are interpreted on the way to the axis:
	//
	//   Stream      — the samples, as written.
	//   SampleIndex — the ramp, rebased so a narrow window reads 0..plotSize-1. The range is
	//                 the plot window itself, so it is pinned to 0..plotSize-1 whatever the
	//                 window happens to hold, rather than following the data.
	//   Time        — the same ramp divided by the *channel's* sample rate. Per channel
	//                 rather than a shared stream on purpose: all sources share one
	//                 engine buffer size, so one index ramp is right for everyone, but
	//                 two channels can legitimately carry different rates.
	struct Source
	{
		enum class Mode { Stream, SampleIndex, Time };

		scopy::acq::DataKey key;
		// Shown on the axis. Follows the source when it changes, unless the reader has
		// typed one of their own.
		QString unit;
		Mode mode{Mode::Stream};

		static Source sampleIndex();
		static Source time();
		static Source stream(const scopy::acq::DataKey &k, const QString &u)
		{
			return Source{k, u, Mode::Stream};
		}

		bool isSampleIndex() const { return mode == Mode::SampleIndex; }
		bool isTime() const { return mode == Mode::Time; }
		// Either derived mode: reads the ramp rather than acquired data.
		bool isIndexBased() const { return mode != Mode::Stream; }
	};

	// The combo payload for the "time (s)" entry. Unlike sample index, time genuinely is
	// not a stream, so it keeps a sentinel — defined here once and reused by
	// AcqPlotManager's ADD CHANNEL picker rather than duplicated per file.
	static const QString &timeComboData();

	// AcquisitionEngine::indexRampKey().toString(), cached. Both pickers use it as the
	// "sample index" entry's payload and to skip the ramp when listing streams.
	static const QString &rampKeyString();

	// `axis` is a pooled PlotAxis owned by the AcqPlot, already registered with the
	// PlotWidget. `store` is only read to populate the source picker; `engine` supplies
	// the declared-key list and the units the producers named.
	AcqAxis(PlotAxis *axis, const Source &src, scopy::acq::DataStore *store,
		scopy::acq::AcquisitionEngine *engine, QObject *parent = nullptr);
	~AcqAxis() override;

	PlotAxis *plotAxis() const { return m_axis.data(); }
	// QwtAxis::XBottom/YLeft/... — the position the pool allocated it at.
	int position() const;
	bool isHorizontal() const;

	const Source &source() const { return m_source; }
	bool isSampleIndex() const { return m_source.isSampleIndex(); }
	bool isTime() const { return m_source.isTime(); }
	Source::Mode mode() const { return m_source.mode; }
	// Retargets the axis. Emits sourceChanged() so the manager can move the depth
	// claim off the old key and onto the new one; a no-op when nothing changed.
	void setSource(const Source &src);

	// Whether the reader may retarget this axis at all. False for a side that is
	// structurally fixed — a digital track's X is the sample index because
	// DigitalCurveItem takes no X array — in which case the picker is shown disabled
	// with `reason` as its tooltip rather than hidden, so the constraint is visible.
	void setSourceFixed(bool fixed, const QString &reason = QString());
	bool isSourceFixed() const { return m_sourceFixed; }

	// The PlotChannel the autoscaler measures. Null for a kind with no PlotChannel
	// (digital, annotation, waterfall), in which case the AUTOSCALE switch is not
	// built at all — an autoscaler with nothing to measure is a control that does
	// nothing.
	void setAutoscaleChannel(PlotChannel *ch);

	// Whether the target currently contributes to the autoscaler. Separate from
	// setAutoscaleChannel(nullptr) on purpose: a channel the reader switched off must
	// stop dragging the axis — PlotChannel::disable() only detaches the curve, its
	// samples stay, and PlotAutoscaler::autoscale() reads them without checking
	// isEnabled() — but the AUTOSCALE switch must not vanish from the settings page
	// just because the channel happened to be off when the page was built.
	void setAutoscaleChannelActive(bool active);

	bool autoscale() const;
	void setAutoscale(bool on);
	// One pass now, for the final frame after a stop. A no-op while autoscale is off.
	void autoscaleOnce();

	double min() const;
	double max() const;
	void setInterval(double min, double max);

	// Set the interval from data, as a stream-driven axis's channel does every read.
	//
	// Distinct from setInterval() in two ways that both exist to keep the reader in
	// control: it does nothing while autoscale is on (the autoscaler owns the range
	// then), and it drops a request identical to the last *request* rather than to
	// the axis's current interval — so a pan or zoom performed since is not mistaken
	// for a change that needs correcting.
	void requestInterval(double min, double max);

	QString unit() const { return m_source.unit; }
	void setUnit(const QString &u);

	// The settings section for this side: source · autoscale · min/max · unit.
	// Identical for X and Y — `title` is the only thing that differs, and it is the
	// caller's word ("X AXIS" / "Y AXIS").
	QWidget *createSettings(InstrumentTemplate *it, const QString &title, QWidget *parent = nullptr);

	// Repopulate the source picker from the store. Called when the store's key set
	// changes: a stream that appeared after the page was built must be selectable
	// without rebuilding the page.
	void refreshSourceChoices();

Q_SIGNALS:
	// The source was retargeted. Carries both keys because the manager has to release
	// the claim on the one that dropped out, not merely claim the new one.
	void sourceChanged(scopy::acq::DataKey oldKey, scopy::acq::DataKey newKey);
	void intervalChanged(double min, double max);

private:
	// Unit for `k` as its producer declared it, falling back to the key string — on an
	// axis that is still more informative than nothing.
	QString unitForKey(const scopy::acq::DataKey &k) const;
	void applyUnit();

	QPointer<PlotAxis> m_axis;
	QPointer<scopy::acq::DataStore> m_store;
	// For stream descriptors and the declared-key list. Asked on demand — a producer
	// that re-declares its unit needs no notification path.
	QPointer<scopy::acq::AcquisitionEngine> m_engine;
	Source m_source;

	bool m_sourceFixed{false};
	QString m_fixedReason;
	// The reader typed a unit, so a source change no longer overwrites it.
	bool m_unitOverridden{false};

	PlotChannel *m_autoscaleTarget{nullptr}; // not owned
	// Whether the target is currently in the autoscaler's list. False while the channel
	// is switched off; the target pointer is kept either way, because it is also what
	// decides whether the AUTOSCALE switch is built at all.
	bool m_autoscaleActive{true};

	// Built by createSettings and parented into the returned widget, which the menu
	// stack owns — hence QPointer: the stack can outlive this object on teardown.
	QPointer<scopy::gui::PlotAutoscaler> m_autoscaler;
	QPointer<scopy::gui::MenuPlotAxisRangeControl> m_rangeCtrl;
	QPointer<MenuOnOffSwitch> m_autoBtn;
	// The picker, kept so refreshSourceChoices() can repopulate it in place.
	QPointer<MenuCombo> m_sourceCombo;

	// Last interval requestInterval() applied, for its idempotence check. NaN so the
	// first request always applies.
	double m_reqMin{qQNaN()}, m_reqMax{qQNaN()};

	bool m_autoscale{false};
};

} // namespace adc
} // namespace scopy

#endif // ACQAXIS_H
