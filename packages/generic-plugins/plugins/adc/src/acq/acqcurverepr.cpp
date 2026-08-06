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

#include "acqcurverepr.h"

#include "acqplotrow.h"
#include "measurementcontroller.h"

#include <core/acq_engine/DataStore.h>

#include <gui/plotautoscaler.h>
#include <gui/plotaxis.h>
#include <gui/plotchannel.h>
#include <gui/plotwidget.h>
#include <gui/widgets/menucombo.h>
#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menuplotaxisrangecontrol.h>
#include <gui/widgets/menuplotchannelcurvestylecontrol.h>

#include <QComboBox>
#include <QPen>
#include <QSignalBlocker>
#include <QVBoxLayout>

#include <algorithm>

using namespace scopy;
using namespace scopy::adc;

const scopy::acq::DataKey &CurveRepr::sampleIndexKey()
{
	// '$' so it cannot collide with a real key: DataKey parses "source_channel_stage"
	// and no source publishes a leading '$'. Function-local static rather than a
	// namespace-scope one to keep it out of static-init order entirely.
	static const scopy::acq::DataKey k(QStringLiteral("$sample_index"));
	return k;
}

bool CurveRepr::isSampleIndexKey(const scopy::acq::DataKey &k)
{
	// Empty counts: a channel constructed without an X key draws against sample index,
	// which is the behaviour every caller before the sentinel existed relied on.
	return k.key.isEmpty() || k == sampleIndexKey();
}

CurveRepr::CurveRepr()
	: m_xKey(sampleIndexKey())
{
}

CurveRepr::~CurveRepr() { detach(); }

void CurveRepr::attach(AcqPlotRow *row, const QString &name, const QColor &color)
{
	if(!row || !row->plot() || m_ch) {
		return;
	}
	m_plot = row->plot();
	m_row = row;

	// The row's shared X and Y axes, never a per-channel one — see the axis
	// lifetime note in acqplotrow.h.
	m_ch = new PlotChannel(name, QPen(color), m_plot->xAxis(), m_plot->yAxis(), nullptr);

	// addPlotChannel() calls init(), which is what creates the QwtPlotCurve; the
	// PlotChannel constructor leaves it uninitialised. So nothing may call
	// setSamples() before this line.
	m_plot->addPlotChannel(m_ch);
	m_ch->setEnabled(m_enabled);

	// Here rather than in the constructor: initMeasure() stamps the pen colour onto
	// every label it later creates, and the colour arrives with the row.
	m_measureMgr = std::make_unique<TimeMeasureManager>();
	m_measureMgr->initMeasure(QPen(color));
	m_measureMgr->getModel()->setSampleRate(m_sampleRate);
}

void CurveRepr::detach()
{
	// The plot may already be gone: it and the channel owning this repr are
	// siblings under the manager, so their destruction order is unspecified. Once
	// the QwtPlot is destroyed Qwt has auto-detached its items and touching them is
	// a use-after-free.
	if(m_plot.isNull()) {
		m_ch = nullptr;
		m_row = nullptr;
		return;
	}
	if(!m_ch) {
		return;
	}

	// Before the delete: PlotAutoscaler::autoscale() dereferences every channel in
	// its list on a timer, so a deleted channel left registered is a use-after-free
	// within one timeout.
	if(m_autoscaler) {
		m_autoscaler->removeChannels(m_ch);
	}

	PlotChannel *ch = m_ch;
	m_ch = nullptr; // before the call, so a re-entrant detach() is a no-op
	m_row = nullptr;

	// Idempotence matters beyond tidiness: PlotTracker::removeChannel leaves its
	// `toRemove` pointer uninitialised when the channel is not in its list
	// (gui/src/plottracker.cpp:66-76), so a second removePlotChannel() for the same
	// channel is a wild delete.
	m_plot->removePlotChannel(ch);
	delete ch;

	// removePlotChannel() hides the removed channel's Y axis unconditionally
	// (gui/src/plotwidget.cpp:181). With a shared axis that hides it for every
	// remaining channel, and it is only restored if the removed channel happened to
	// be the selected one. Re-assert it.
	m_plot->showAxisLabels();
}

std::size_t CurveRepr::claimDepth(int plotSize, std::size_t bufferSize) const
{
	return scopy::acq::DataStore::depthForWindow(static_cast<std::size_t>(qMax(1, plotSize)), bufferSize);
}

bool CurveRepr::xDataRange(double &min, double &max) const
{
	// Both conditions, not just the flag: setXKey clears the flag when retargeting to the
	// sentinel, but a caller asking mid-retarget deserves the key's answer rather than
	// the flag's.
	if(!m_xRangeValid || isSampleIndexKey(m_xKey)) {
		return false;
	}
	min = m_xMin;
	max = m_xMax;
	return true;
}

QList<scopy::acq::DataKey> CurveRepr::extraKeys() const
{
	// The sentinel is not a stored stream, so claiming depth on it would create an empty
	// buffer in the store and put "$sample_index" in the key list and the picker.
	if(isSampleIndexKey(m_xKey)) {
		return {};
	}
	return {m_xKey};
}

void CurveRepr::setXKey(const scopy::acq::DataKey &k)
{
	// Normalise empty to the sentinel so xKey() has one representation of "index ramp"
	// and the combo can match on it.
	const scopy::acq::DataKey next = k.key.isEmpty() ? sampleIndexKey() : k;
	if(next == m_xKey) {
		return;
	}
	m_xKey = next;

	// The X window is stale for the new key, and a partial read against the old one
	// would truncate the first frame. Cheaper than it looks: pull() overwrites it.
	m_liveX = scopy::acq::SampleVariant{};
	m_scratchX.clear();

	// And so is the span. Dropped rather than kept until the next pull: retargeting from
	// a frequency key to sample index has to release the row's axis immediately, or it
	// stays on the old MHz range with nothing but an index ramp drawn on it.
	m_xRangeValid = false;

	// The new key needs a depth claim and the old one needs releasing, both of which
	// need plotSize and bufferSize — numbers only the manager has. This is exactly what
	// the reclaim notifier is for, and the reason it is not waterfall-specific.
	requestReclaim();
}

void CurveRepr::pull(scopy::acq::DataStore *store, const scopy::acq::DataKey &key, int plotSize)
{
	if(!store || !m_ch || m_plot.isNull()) {
		return;
	}

	// Assign the member, then view it. Both m_live and m_scratch outlive the view,
	// and the view does not outlive this function.
	m_live = store->window(key, plotSize);
	const scopy::acq::FloatView y = scopy::acq::toFloatView(m_live, m_scratch);
	if(y.size <= 0) {
		return;
	}

	const float *xPtr = nullptr;
	int n = 0;

	if(!isSampleIndexKey(m_xKey)) {
		m_liveX = store->window(m_xKey, plotSize);
		const scopy::acq::FloatView x = scopy::acq::toFloatView(m_liveX, m_scratchX);
		if(x.size <= 0) {
			return;
		}
		n = qMin(x.size, y.size);
		xPtr = x.data;

		// The row's X axis has to cover this span, and only the manager can decide how
		// (the axis is shared, so it needs the union over the row). Recorded here
		// because the samples are already in hand; std::minmax_element rather than
		// assuming x[0]..x[n-1] — a frequency key is monotonic, but nothing in the
		// interface promises an X stream is sorted, and reading a reversed one as
		// (first, last) would give an inverted interval Qwt draws mirrored.
		const auto mm = std::minmax_element(xPtr, xPtr + n);
		m_xMin = static_cast<double>(*mm.first);
		m_xMax = static_cast<double>(*mm.second);
		m_xRangeValid = true;
	} else {
		if(!m_indexSrc || m_indexSrc->isEmpty()) {
			return;
		}
		// A partial window is shorter, not left-padded (SampleBuffer::window), so
		// right-anchor the shared ramp against it rather than starting at 0.
		const int off = qMax(0, m_indexSrc->size() - y.size);
		xPtr = m_indexSrc->constData() + off;
		n = qMin(y.size, m_indexSrc->size() - off);
	}

	if(n <= 0) {
		return;
	}

	// copy=true: Qwt keeps its own buffer, so no FloatView outlives this call and
	// nothing can paint a prior run's tail. Same choice and same reasoning as
	// src/sim/siminstrumentcontroller.cpp:1077-1081.
	m_ch->setSamples(xPtr, y.data, static_cast<size_t>(n), true);

	// Measured on the same window that was just drawn, so a label can never disagree
	// with the curve above it. MeasureModel::setDataSource keeps the bare pointer and
	// measure() reads it synchronously, which is safe for the same reason the view is:
	// it aliases m_live or m_scratch, both members, so it stays valid until the next
	// pull replaces them. Same call pair as
	// src/time/grtimechannelcomponent.cpp:439-441.
	if(m_measureMgr) {
		MeasureModel *model = m_measureMgr->getModel();
		model->setDataSource(y.data, static_cast<size_t>(n));
		model->measure();
	}
}

void CurveRepr::reset()
{
	// Drop every cached window so no prior-run buffer is reachable on the next
	// cycle.
	// Before the buffers go: the measure model holds a bare pointer into m_live or
	// m_scratch from the last pull, and clearing those without telling it leaves it
	// pointing at freed memory for any measure() before the next pull.
	if(m_measureMgr) {
		m_measureMgr->getModel()->setDataSource(nullptr, 0);
	}

	m_live = scopy::acq::SampleVariant{};
	m_liveX = scopy::acq::SampleVariant{};
	m_scratch.clear();
	m_scratchX.clear();

	// m_xRangeValid is deliberately *not* cleared. This runs on every Run, and the X key
	// is unchanged — the same frequency span is about to be written again one cycle
	// later. Dropping it would snap the row's axis to 0..plotSize-1 and back on every
	// start, which reads as a glitch. setXKey() is the event that genuinely invalidates
	// the span, and it clears it there.

	if(!m_ch || m_plot.isNull()) {
		return;
	}
	// Blank the curve rather than leave the previous run's tail on screen. A dummy
	// pointer with size 0, not nullptr: some Qwt paths dereference the data pointer
	// even for empty ranges (src/sim/siminstrumentcontroller.cpp:798-803).
	static const float kZero = 0.0f;
	m_ch->setSamples(&kZero, &kZero, 0, true);
}

QWidget *CurveRepr::createSettingsWidget(QWidget *parent)
{
	// attach() must have run: every control here binds to the PlotChannel or the
	// row's axis. AcqChannel::createSettingsPage is called after attach, so this
	// holds — but a null return is better than half a page.
	if(!m_ch || m_row.isNull() || !m_row->plot()) {
		return nullptr;
	}

	QWidget *w = new QWidget(parent);
	QVBoxLayout *lay = new QVBoxLayout(w);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(10);

	// The second key. A curve reads two streams — Y from the channel's own key, X from
	// this one — and unlike the Y key the X key is editable, because it is a choice
	// about how to draw rather than about what to draw: the same magnitude stream is a
	// spectrum against a frequency key and a meaningless ramp against sample index.
	m_xCombo = new MenuCombo(QObject::tr("X source"), w);
	// Connected before the first fill so nothing is missed, and every fill blocks
	// signals — see refreshKeySources(), which the manager calls as keys appear.
	QObject::connect(m_xCombo->combo(), &QComboBox::currentIndexChanged, m_xCombo.data(), [this](int idx) {
		if(idx < 0 || m_xCombo.isNull()) {
			return;
		}
		setXKey(scopy::acq::DataKey(m_xCombo->combo()->itemData(idx).toString()));
	});
	refreshKeySources();
	lay->addWidget(m_xCombo);

	// The row's shared Y axis, not a per-channel one. So this control moves the range
	// for every curve on the row — which is the documented consequence of Qwt having
	// no axis-removal path (see acqplotrow.h). N channels each building their own
	// control over the same axis is fine: MenuPlotAxisRangeControl is a view, and it
	// follows PlotAxis::min/maxChanged, so they stay in step with each other.
	PlotAxis *yAxis = m_row->plot()->yAxis();
	m_yCtrl = new scopy::gui::MenuPlotAxisRangeControl(yAxis, w);

	MenuOnOffSwitch *autoBtn = new MenuOnOffSwitch(QObject::tr("AUTOSCALE"), w, false);

	m_autoscaler = new scopy::gui::PlotAutoscaler(w);
	syncAutoscalerChannel();
	QObject::connect(m_autoscaler.data(), &scopy::gui::PlotAutoscaler::newMin, m_yCtrl.data(),
			 &scopy::gui::MenuPlotAxisRangeControl::setMin);
	QObject::connect(m_autoscaler.data(), &scopy::gui::PlotAutoscaler::newMax, m_yCtrl.data(),
			 &scopy::gui::MenuPlotAxisRangeControl::setMax);

	QObject::connect(autoBtn->onOffswitch(), &QAbstractButton::toggled, m_autoscaler.data(), [this](bool on) {
		m_autoscaleEnabled = on;
		// The manual spinboxes and the autoscaler write the same axis; leaving both
		// live means the reader's value is overwritten a timeout later.
		if(m_yCtrl) {
			m_yCtrl->setEnabled(!on);
		}
		if(!m_autoscaler) {
			return;
		}
		// start() is not optional: PlotAutoscaler::onNewData and autoscale() both
		// return immediately while its timer is stopped
		// (gui/src/plotautoscaler.cpp:59-63), so an autoscaler that is never started
		// silently does nothing.
		if(on) {
			m_autoscaler->start();
		} else {
			m_autoscaler->stop();
		}
	});

	scopy::gui::MenuPlotChannelCurveStyleControl *style = new scopy::gui::MenuPlotChannelCurveStyleControl(w);
	style->addChannels(m_ch);

	lay->addWidget(autoBtn);
	lay->addWidget(m_yCtrl);
	lay->addWidget(style);

	// The measurement selector — two collapsible sections of checkbox pairs
	// (measure · stat) which drive the panels the manager wired up. Built here rather
	// than by the manager because it is per-curve, and it is the same widget the
	// existing time channel puts in its menu
	// (src/time/grtimechannelcomponent.cpp:215).
	if(m_measureMgr) {
		lay->addWidget(m_measureMgr->createMeasurementMenu(w));
	}

	return w;
}

void CurveRepr::refreshKeySources()
{
	if(m_xCombo.isNull()) {
		return; // no settings page built yet
	}
	QComboBox *box = m_xCombo->combo();

	// Blocked throughout: rebuilding drops the current index, and the resulting
	// currentIndexChanged(-1) then (0) would call setXKey with whatever landed first and
	// silently retarget the curve on every refresh.
	QSignalBlocker blocker(box);
	box->clear();

	// Sample index first and always present. It is the only X source that exists before
	// the engine has ever run, so a curve is never left with an empty combo.
	box->addItem(QObject::tr("Sample index"), sampleIndexKey().toString());

	QList<scopy::acq::DataKey> keys;
	if(m_keySource) {
		keys = m_keySource();
	}
	for(const scopy::acq::DataKey &k : std::as_const(keys)) {
		box->addItem(k.toString(), k.toString());
	}

	// Restore the selection by key, not by index — the list is rebuilt and a key's
	// position moves as others appear. A key that has vanished from the store stays
	// selected as a stale entry rather than snapping the curve back to sample index:
	// window() on a missing key returns empty, which draws nothing, and the key may well
	// come back on the next run.
	int idx = box->findData(m_xKey.toString());
	if(idx < 0) {
		box->addItem(QObject::tr("%1 (missing)").arg(m_xKey.toString()), m_xKey.toString());
		idx = box->count() - 1;
	}
	box->setCurrentIndex(idx);
}

MeasureManagerInterface *CurveRepr::measureManager() const { return m_measureMgr.get(); }

void CurveRepr::setSampleRate(double sr)
{
	if(sr <= 0.0) {
		return;
	}
	m_sampleRate = sr;
	// Tolerates being called before attach(): the value is replayed there.
	if(m_measureMgr) {
		m_measureMgr->getModel()->setSampleRate(sr);
	}
}

void CurveRepr::syncAutoscalerChannel()
{
	if(!m_autoscaler || !m_ch) {
		return;
	}
	if(m_enabled) {
		m_autoscaler->addChannels(m_ch);
	} else {
		m_autoscaler->removeChannels(m_ch);
	}
}

void CurveRepr::setEnabled(bool en)
{
	m_enabled = en;
	if(!m_ch || m_plot.isNull()) {
		return;
	}
	if(en) {
		m_ch->enable();
	} else {
		m_ch->disable();
	}
	syncAutoscalerChannel();
}

void CurveRepr::setColor(const QColor &c)
{
	if(m_ch && !m_plot.isNull()) {
		m_ch->setColor(c);
	}
}

void CurveRepr::setName(const QString &n)
{
	if(m_ch && !m_plot.isNull()) {
		m_ch->setName(n);
	}
}
