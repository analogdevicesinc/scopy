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
#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menuplotaxisrangecontrol.h>
#include <gui/widgets/menuplotchannelcurvestylecontrol.h>

#include <QPen>
#include <QVBoxLayout>

using namespace scopy;
using namespace scopy::adc;

CurveRepr::CurveRepr() = default;

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

	if(!m_indexSrc || m_indexSrc->isEmpty()) {
		return;
	}
	// A partial window is shorter, not left-padded (SampleBuffer::window), so
	// right-anchor the shared ramp against it rather than starting at 0.
	const int off = qMax(0, m_indexSrc->size() - y.size);
	const float *xPtr = m_indexSrc->constData() + off;
	const int n = qMin(y.size, m_indexSrc->size() - off);

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
	m_scratch.clear();

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
