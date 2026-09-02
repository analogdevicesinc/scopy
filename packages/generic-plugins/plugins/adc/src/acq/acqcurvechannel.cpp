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

#include "acqcurvechannel.h"

#include "acqaxis.h"
#include "acqchannelregistry.h"
#include "acqplot.h"

#include <core/acq_engine/DataStore.h>

#include <gui/plotaxis.h>
#include <gui/plotchannel.h>
#include <gui/plotwidget.h>
#include <gui/widgets/menuplotchannelcurvestylecontrol.h>

#include <QPen>
#include <QVBoxLayout>

using namespace scopy;
using namespace scopy::adc;

// Any stream: toFloatView converts every numeric type, and returns empty for an
// annotation one — which readData below treats as "nothing to draw yet", so a curve
// pointed at a decoder's output is an empty curve rather than a refusal.
REGISTER_ACQ_CHANNEL_KIND(AcqCurveChannel, scopy::acq::ReprKind::Curve)

AcqCurveChannel::AcqCurveChannel(const Args &args)
	: AcqChannel(args)
	, m_kZero(0.0f)
{
}

AcqCurveChannel::~AcqCurveChannel() { detach(); }

void AcqCurveChannel::attachTo(AcqPlot *plot)
{
	if(!plot->plot() || m_ch) {
		return;
	}
	m_plot = plot->plot();

	// This channel's own axes, out of the plot's pool — acquired by the base before it
	// called us. Two curves on one plot can therefore be drawn against two different X
	// sources, which is the whole reason the pool exists; see the axis lifetime note in
	// acqplot.h for why nothing here may delete one.
	PlotAxis *x = xAxis() ? xAxis()->plotAxis() : m_plot->xAxis();
	PlotAxis *y = yAxis() ? yAxis()->plotAxis() : m_plot->yAxis();
	m_ch = new PlotChannel(name(), QPen(color()), x, y, nullptr);

	// addPlotChannel() calls init(), which is what creates the QwtPlotCurve; the
	// PlotChannel constructor leaves it uninitialised. So nothing may call
	// setSamples() before this line.
	m_plot->addPlotChannel(m_ch);
}

void AcqCurveChannel::detachFrom()
{
	// The plot may already be gone: it and this channel are siblings under the
	// manager, so their destruction order is unspecified. Once the QwtPlot is
	// destroyed Qwt has auto-detached its items and touching them is a
	// use-after-free.
	if(m_plot.isNull()) {
		m_ch = nullptr;
		return;
	}
	if(!m_ch) {
		return;
	}

	// No autoscaler deregistration here any more: the autoscalers belong to the two
	// AcqAxis objects, and their destructors drop this channel — which the base runs
	// after this, in releaseAxes().

	PlotChannel *ch = m_ch;
	m_ch = nullptr; // before the call, so a re-entrant detach() is a no-op

	m_plot->removePlotChannel(ch);
	delete ch;

	// No showAxisLabels() re-assert here any more: removePlotChannel() now only hides
	// the Y axis when no remaining channel uses it (gui/src/plotwidget.cpp), so a
	// sibling's removal no longer takes the shared axis's labels away.
}

DepthNeed AcqCurveChannel::depthNeeded(int plotSize) const
{
	return DepthNeed::samples(static_cast<std::size_t>(qMax(1, plotSize)));
}

void AcqCurveChannel::readData(scopy::acq::DataStore *store, int plotSize)
{
	if(!m_ch || m_plot.isNull()) {
		return;
	}

	// Assign the member, then view it. Both m_live and m_scratch outlive the view,
	// and the view does not outlive this function.
	m_live = store->window(key(), plotSize);
	const scopy::acq::FloatView y = scopy::acq::toFloatView(m_live, m_scratch);
	if(y.size <= 0) {
		return;
	}

	// One read for every mode. Sample index and time are no longer a special case: both
	// read the engine's ramp stream, which is written once per length change and so costs
	// this path nothing. Owned, not viewed — a second FloatView would share m_scratch
	// with the Y read above and one would overwrite the other.
	m_xData = store->windowFloat(xKey(), plotSize);
	if(m_xData.isEmpty()) {
		return;
	}

	// The window's own origin, taken before the right-anchor below narrows the view of it.
	// The index-based modes are measured from *this* — the start of the plot window — not
	// from the first sample that happens to have arrived, which is what makes the axis read
	// 0..plotSize-1 for the whole run instead of shrinking to the amount of data on hand.
	const float windowFirst = m_xData.constData()[0];

	// Right-anchor. A partial window comes back shorter rather than left-padded
	// (SampleBuffer::window), so with X at full width and Y still filling, the samples
	// that pair with Y are the *newest* ones. Without this the qMin below would take the
	// first y.size of X and draw the curve flush left. Applies to any X stream longer
	// than Y, not only the ramp.
	//
	// An offset rather than a remove(): the trim only picks *which* window slots the
	// samples occupy, and rebasing from windowFirst above keeps those slot numbers, so a
	// partial window's samples land in the right half of the window. Where the *axis* sits
	// is decided separately, at the bottom of this function. Not mutating m_xData also
	// spares a memmove of the whole window per frame.
	const int off = qMax(0, m_xData.size() - y.size);
	const float *xPtr = m_xData.constData() + off;
	int xCount = m_xData.size() - off;

	// Hoisted above the copy below because the axis range at the bottom needs the same
	// scale: one factor, one place it is decided.
	const AcqAxis::Source::Mode mode = xAxis() ? xAxis()->mode() : AcqAxis::Source::Mode::Stream;
	const double rate = sampleRate() > 0.0 ? sampleRate() : 1.0;
	const float scale = mode == AcqAxis::Source::Mode::Time ? static_cast<float>(1.0 / rate) : 1.0f;

	// The two index-based modes are affine transforms of that window, cached so the pass
	// runs on a width/rate change rather than per frame.
	if(mode != AcqAxis::Source::Mode::Stream &&
	   !(mode == AcqAxis::Source::Mode::SampleIndex && windowFirst == 0.0f)) {
		// The common sample-index case — plotSize == ramp length, so the window already
		// starts at 0 — is excluded above and uses the store's vector with no copy. Note
		// that this now covers a partially-filled window too: the tail of a ramp starting
		// at 0 already carries the slot numbers we want.
		//
		// Time is the same window over the rate, so it is measured from the start of the
		// plot window as well: a filling window reads (plotSize-n)/rate..(plotSize-1)/rate.
		if(m_derivedMode != static_cast<int>(mode) || m_derivedSize != xCount ||
		   m_derivedFirst != windowFirst || !qFuzzyCompare(m_derivedRate, rate)) {
			m_derivedX.resize(xCount);
			for(int i = 0; i < xCount; ++i) {
				m_derivedX[i] = (xPtr[i] - windowFirst) * scale;
			}
			m_derivedMode = static_cast<int>(mode);
			m_derivedSize = xCount;
			m_derivedFirst = windowFirst;
			m_derivedRate = rate;
		}
		xPtr = m_derivedX.constData();
		xCount = m_derivedX.size();
	}

	const int n = qMin(y.size, xCount);
	if(n <= 0) {
		return;
	}

	// copy=true: Qwt keeps its own buffer, so no FloatView outlives this call and
	// nothing can paint a prior run's tail. Same choice and same reasoning as
	// src/sim/siminstrumentcontroller.cpp:1077-1081.
	m_ch->setSamples(xPtr, y.data, static_cast<size_t>(n), true);

	// The X range. Set here rather than by the manager for every mode, sample index
	// included, which is what fixes retargeting an axis *back* to sample index: that used
	// to leave the previous stream's range in place until the reader touched the plot-width
	// spinbox. Its own axis now, not a shared one, so two curves against two different X
	// sources on one plot each get their own scale. requestInterval() drops repeats,
	// degenerate ranges and everything while autoscale is on, so this cannot undo the
	// reader's zoom or fight the autoscaler, and costs one comparison per frame once the
	// range settles.
	if(xAxis()) {
		if(mode == AcqAxis::Source::Mode::Stream) {
			// Only the stream's contents can say what range it covers. First and last
			// rather than min/max: an FFT's bins are monotonic, and scanning every
			// sample per frame to learn that would cost more than the axis is worth.
			xAxis()->requestInterval(xPtr[0], xPtr[n - 1]);
		} else {
			// The window, deliberately not the data in it: for the index-based modes
			// the range *is* the plot window, so it is known without reading a sample
			// and does not move while history fills. A partial window then leaves the
			// left of the plot empty and the curve grows leftwards, rather than the
			// axis rescaling on every frame.
			//
			// m_xData's size, not plotSize: this is the untrimmed window, so if the
			// ramp is ever shorter than the plot asked for, the axis matches the X
			// values that actually exist instead of promising a range no sample reaches.
			xAxis()->requestInterval(0.0, static_cast<double>(m_xData.size() - 1) * scale);
		}
	}
}

void AcqCurveChannel::reset()
{
	// Drop every cached window so no prior-run buffer is reachable on the next cycle.
	m_live = scopy::acq::SampleVariant{};
	m_scratch.clear();
	m_xData.clear();
	m_derivedX.clear();
	m_derivedMode = -1;

	if(!m_ch || m_plot.isNull()) {
		return;
	}
	// Blank the curve rather than leave the previous run's tail on screen. A dummy
	// pointer with size 0, not nullptr: some Qwt paths dereference the data pointer
	// even for empty ranges (src/sim/siminstrumentcontroller.cpp:798-803).
	m_ch->setSamples(&m_kZero, &m_kZero, 0, true);
}

QWidget *AcqCurveChannel::createKindSettings(QWidget *parent)
{
	// attach() must have run: the curve-style control binds to the PlotChannel.
	// AcqChannel::createSettingsPage is called after attach, so this holds — but a null
	// return is better than half a page.
	//
	// No Y range control, no AUTOSCALE switch and no PlotAutoscaler here any more: all
	// three are the same four controls on both sides, so they live in AcqAxis and the
	// base puts them on the page for X and Y alike.
	if(!m_ch) {
		return nullptr;
	}

	QWidget *w = new QWidget(parent);
	QVBoxLayout *lay = new QVBoxLayout(w);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(10);

	scopy::gui::MenuPlotChannelCurveStyleControl *style = new scopy::gui::MenuPlotChannelCurveStyleControl(w);
	style->addChannels(m_ch);

	lay->addWidget(style);

	return w;
}

void AcqCurveChannel::onEnabledChanged(bool en)
{
	if(!m_ch || m_plot.isNull()) {
		return;
	}
	if(en) {
		m_ch->enable();
	} else {
		m_ch->disable();
	}

	// Deregister from both autoscalers while disabled. PlotChannel::disable() only
	// detaches the curve — its samples stay (gui/src/plotchannel.cpp:63-83) — and
	// PlotAutoscaler::autoscale() walks its channel list reading curve data without
	// checking isEnabled(), so a switched-off curve would otherwise keep dragging the
	// axis to its last window. This keeps the target pointer — so the AUTOSCALE switch
	// still exists on the settings page — and only takes it out of the autoscaler's
	// list.
	if(xAxis()) {
		xAxis()->setAutoscaleChannelActive(en);
	}
	if(yAxis()) {
		yAxis()->setAutoscaleChannelActive(en);
	}
}

void AcqCurveChannel::onColorChanged(const QColor &c)
{
	if(m_ch && !m_plot.isNull()) {
		m_ch->setColor(c);
	}
}

void AcqCurveChannel::onNameChanged(const QString &n)
{
	if(m_ch && !m_plot.isNull()) {
		m_ch->setName(n);
	}
}

void AcqCurveChannel::onSampleRateChanged(double sr)
{
	Q_UNUSED(sr)
	// The divisor of the time axis, so the cached seconds vector is now stale. The tuple
	// check in readData() would catch this on its own; invalidating here keeps the cache's
	// inputs explicit rather than relying on the rate being compared.
	m_derivedMode = -1;
}

#include "moc_acqcurvechannel.cpp"
