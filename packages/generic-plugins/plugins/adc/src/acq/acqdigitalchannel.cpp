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

#include "acqdigitalchannel.h"

#include "DigitalCurveItem.h"
#include "acqaxis.h"
#include "acqchannelregistry.h"
#include "acqplot.h"

#include <core/acq_engine/DataStore.h>

#include <gui/axishandle.h>
#include <gui/plotaxis.h>
#include <gui/plotaxishandle.h>
#include <gui/plotwidget.h>
#include <gui/style.h>
#include <gui/style_attributes.h>

#include <qwt_plot.h>

using namespace scopy;
using namespace scopy::adc;

// Any stream, and deliberately not only the 8-bit types: toBits() maps any non-zero to
// 1, so a comparator writing Int32 is as valid a logic source as a UInt8 line. Which of
// them is a logic line is the producer's statement (StreamInfo::kind), never an
// inference from the width.
REGISTER_ACQ_CHANNEL_KIND(AcqDigitalChannel, scopy::acq::ReprKind::Digital)

AcqDigitalChannel::AcqDigitalChannel(const Args &args)
	: AcqChannel(args)
{
}

AcqDigitalChannel::~AcqDigitalChannel() { detach(); }

PlotAxis *AcqDigitalChannel::ownYAxis(AcqPlot *plot) { return plot ? plot->digitalAxis() : nullptr; }

void AcqDigitalChannel::attachTo(AcqPlot *plot)
{
	if(!plot->plot() || m_item) {
		return;
	}
	// The base already wrapped digitalAxis() for us — through ownYAxis, not the pool, so
	// every logic track on this plot shares one scale and their handles are comparable.
	PlotAxis *digAxis = yAxis() ? yAxis()->plotAxis() : plot->digitalAxis();
	if(!digAxis) {
		return;
	}

	m_plot = plot->plot();

	// Index-only, so say so on the picker rather than leaving a control that would be
	// silently ignored. The reason is DigitalCurveItem's: see readData below.
	if(xAxis()) {
		xAxis()->setSourceFixed(true, tr("Logic tracks are drawn against the sample index"));
	}

	// A draggable handle on the shared digital axis, one per track. The item reads
	// its band position off the handle every draw, so dragging the handle is what
	// reorders tracks — there is no stored ordering to keep in sync.
	m_handle = new PlotAxisHandle(m_plot.data(), digAxis);
	m_handle->handle()->setBarVisibility(BarVisibility::ON_HOVER);
	m_handle->handle()->setColor(color().isValid() ? color() : Style::getColor(json::theme::content_silent));
	// NORTH_OR_WEST on a YLeft axis is the left edge, where the analog Y labels are.
	// Annotation bands use the east side, so the two never collide.
	m_handle->handle()->setHandlePos(HandlePos::NORTH_OR_WEST);
	m_plot->addPlotAxisHandle(m_handle);

	// AxisHandle parents itself to the canvas and installs an event filter there to
	// resize itself. Do not reparent, resize or raise it — that breaks the filter and
	// the handle gets clipped on a horizontal resize. Only the explicit show is
	// needed, the same call PlotCursors makes.
	m_handle->handle()->setVisible(true);
	m_handle->setPosition(plot->nextDigitalSlot());

	// A drag moves the band, but the item only repositions on a draw, so the plot has
	// to be told. The manager's frame timer would eventually do it, but not while
	// stopped — which is exactly when a reader reorders tracks.
	connect(m_handle.data(), &PlotAxisHandle::scalePosChanged, this, [this](double) {
		if(!m_plot.isNull() && m_plot->plot()) {
			m_plot->plot()->replot();
		}
	});

	// The pooled X axis, not the widget's built-in one: the band is laid out across
	// whichever horizontal scale this channel was given, so that a logic track selected
	// alongside an analog curve shows a scale that means the same thing.
	PlotAxis *x = xAxis() ? xAxis()->plotAxis() : m_plot->xAxis();
	m_item = new DigitalCurveItem(name(), x, digAxis, m_handle.data());
	m_item->setColor(color());
	m_item->attach(m_plot->plot());
}

void AcqDigitalChannel::detachFrom()
{
	// The plot, and with it the canvas that owns the handle, may already be gone —
	// this channel and the plot are siblings under the manager. Once the QwtPlot is
	// destroyed Qwt has auto-detached its items, so detaching again is a
	// use-after-free. Same discipline as DigitalTrackManager's destructor.
	if(m_plot.isNull()) {
		m_item = nullptr;
		m_handle = nullptr;
		return;
	}

	if(m_item) {
		DigitalCurveItem *item = m_item;
		m_item = nullptr; // before the calls, so a re-entrant detach is a no-op
		item->detach();
		delete item;
	}

	if(!m_handle.isNull()) {
		PlotAxisHandle *h = m_handle.data();
		m_handle = nullptr;
		m_plot->removePlotAxisHandle(h);
		// deleteLater, not delete: the handle's own event filter may be mid-dispatch
		// on the canvas when a channel is removed from a UI callback.
		h->deleteLater();
	}
}

DepthNeed AcqDigitalChannel::depthNeeded(int plotSize) const
{
	// Same window as a curve: a digital track is one bit per sample over the same
	// visible span.
	return DepthNeed::samples(static_cast<std::size_t>(qMax(1, plotSize)));
}

void AcqDigitalChannel::readData(scopy::acq::DataStore *store, int plotSize)
{
	if(!m_item || m_plot.isNull()) {
		return;
	}

	// toBits owns its result, so unlike AcqCurveChannel's FloatView there is no
	// aliasing to manage here — and setSamples copies into the item anyway. The cost
	// is one allocation per cycle, against a quint8 per sample.
	const QVector<quint8> bits = scopy::acq::toBits(store->window(key(), plotSize));
	if(bits.isEmpty()) {
		return;
	}

	// The count the item lays samples out against, not the count it has. Telling it
	// plotSize rather than bits.size() is what keeps a partial window (a fresh run,
	// or plotSize > one buffer) aligned with the analog curves instead of stretched to
	// fill the axis.
	//
	// Index-only by construction: DigitalCurveItem places bands by sample number and
	// takes no X array, so a StreamInfo::xKey cannot be honoured here — which is why
	// attachTo() pins the X axis's source rather than leaving a picker whose selection
	// would be silently dropped.
	m_item->setSampleCount(static_cast<quint64>(qMax(1, plotSize)));
	m_item->setSamples(bits);
}

void AcqDigitalChannel::reset()
{
	if(m_item && !m_plot.isNull()) {
		m_item->clear();
	}
}

void AcqDigitalChannel::onEnabledChanged(bool en)
{
	if(m_plot.isNull()) {
		return;
	}
	if(m_item) {
		m_item->setVisible(en);
	}
	// The handle goes with the band: a lone handle for an invisible track is a control
	// with nothing to control.
	if(!m_handle.isNull() && m_handle->handle()) {
		m_handle->handle()->setVisible(en);
	}
}

void AcqDigitalChannel::onColorChanged(const QColor &c)
{
	if(m_plot.isNull()) {
		return;
	}
	if(m_item) {
		m_item->setColor(c);
	}
	if(!m_handle.isNull() && m_handle->handle()) {
		m_handle->handle()->setColor(c);
	}
}

#include "moc_acqdigitalchannel.cpp"
