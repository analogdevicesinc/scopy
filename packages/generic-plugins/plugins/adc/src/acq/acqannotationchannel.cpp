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

#include "acqannotationchannel.h"

#include "acqaxis.h"
#include "acqchannelregistry.h"
#include "acqplot.h"

#include <core/acq_engine/DataStore.h>

#include <gui/annotationcurve.h>
#include <gui/axishandle.h>
#include <gui/plotaxis.h>
#include <gui/plotaxishandle.h>
#include <gui/plotwidget.h>
#include <gui/style.h>
#include <gui/style_attributes.h>

#include <qwt_plot.h>

using namespace scopy;
using namespace scopy::adc;

// Any stream. Only an annotation one has anything to show here: readData's latestAs<>()
// narrows to the annotation alternative, so a numeric key clears the bands rather than
// being misread.
REGISTER_ACQ_CHANNEL_KIND(AcqAnnotationChannel, scopy::acq::ReprKind::Annotations)

AcqAnnotationChannel::AcqAnnotationChannel(const Args &args)
	: AcqChannel(args)
{
}

AcqAnnotationChannel::~AcqAnnotationChannel() { detach(); }

PlotAxis *AcqAnnotationChannel::ownYAxis(AcqPlot *plot) { return plot ? plot->digitalAxis() : nullptr; }

void AcqAnnotationChannel::attachTo(AcqPlot *plot)
{
	if(!plot->plot() || m_item) {
		return;
	}
	// Wrapped by the base through ownYAxis, so this and every logic track on the plot
	// share one scale — which is what lets a decode band sit under the line it decodes.
	PlotAxis *digAxis = yAxis() ? yAxis()->plotAxis() : plot->digitalAxis();
	if(!digAxis) {
		return;
	}

	m_plot = plot->plot();

	// AnnotationCurve places its spans by sample offset and takes no X array, so the
	// source picker would be a control with no effect. Disabled with the reason, not
	// hidden.
	if(xAxis()) {
		xAxis()->setSourceFixed(true, tr("Decode bands are drawn against the sample index"));
	}

	m_handle = new PlotAxisHandle(m_plot.data(), digAxis);
	m_handle->handle()->setBarVisibility(BarVisibility::ON_HOVER);
	m_handle->handle()->setColor(color().isValid() ? color() : Style::getColor(json::theme::content_silent));
	// SOUTH_OR_EAST, i.e. the right edge, where AcqDigitalChannel uses the left. A
	// decode band is usually stacked directly under the logic track it decodes, and
	// two handles at the same height on the same side would overlap.
	m_handle->handle()->setHandlePos(HandlePos::SOUTH_OR_EAST);
	m_plot->addPlotAxisHandle(m_handle);

	// See AcqDigitalChannel::attachTo — the handle parents itself to the canvas and
	// filters its events, so nothing here may reparent, resize or raise it.
	m_handle->handle()->setVisible(true);
	m_handle->setPosition(plot->nextDigitalSlot());

	connect(m_handle.data(), &PlotAxisHandle::scalePosChanged, this, [this](double) {
		if(!m_plot.isNull() && m_plot->plot()) {
			m_plot->plot()->replot();
		}
	});

	// This channel's own pooled X axis rather than the widget's built-in one, so the
	// spans are laid out across the same horizontal scale its rail neighbours use.
	PlotAxis *x = xAxis() ? xAxis()->plotAxis() : m_plot->xAxis();
	m_item = new AnnotationCurve(name(), x, digAxis, m_handle.data());
	m_item->attach(m_plot->plot());
}

void AcqAnnotationChannel::detachFrom()
{
	// Same discipline as AcqDigitalChannel: the plot and this channel are siblings, so
	// the QwtPlot may already have gone and auto-detached its items.
	if(m_plot.isNull()) {
		m_item = nullptr;
		m_handle = nullptr;
		return;
	}

	if(m_item) {
		AnnotationCurve *item = m_item;
		m_item = nullptr; // before the calls, so a re-entrant detach is a no-op
		item->detach();
		delete item;
	}

	if(!m_handle.isNull()) {
		PlotAxisHandle *h = m_handle.data();
		m_handle = nullptr;
		m_plot->removePlotAxisHandle(h);
		// deleteLater: the handle's event filter may be mid-dispatch on the canvas when
		// a channel is removed from a UI callback.
		h->deleteLater();
	}
}

DepthNeed AcqAnnotationChannel::depthNeeded(int plotSize) const
{
	Q_UNUSED(plotSize)
	// One chunk, whatever the window. An annotation read is latestAs<>() — and even
	// SampleBuffer::window() returns an annotation stream from the newest chunk alone,
	// because the offsets in each record are relative to the window the decoder ran
	// on and stitching would need shiftAnnotations() to re-anchor them. Claiming more
	// would retain history nothing can read.
	//
	// Chunks, not samples: "the newest one" is a chunk count and must stay 1 however
	// many annotation records that chunk happens to hold.
	return DepthNeed::chunks(1);
}

void AcqAnnotationChannel::readData(scopy::acq::DataStore *store, int plotSize)
{
	if(!m_item || m_plot.isNull()) {
		return;
	}

	// latestAs, not window(): narrowing to the annotation alternative is the check for
	// "this key is not an annotation stream" as well as the read, so a channel pointed
	// at a numeric key by mistake draws nothing rather than misreading it.
	const std::optional<QVector<scopy::acq::Annotation>> anns =
		store->latestAs<QVector<scopy::acq::Annotation>>(key());
	if(!anns) {
		// Absent, empty, or the wrong type. Clear rather than leave the last decode on
		// screen — a decoder that stopped producing has no annotations, and stale ones
		// would read as current.
		m_item->clear();
		return;
	}

	// acq::Annotation -> gui::AnnotationSpan, field by field. The drawing side keeps
	// its own struct so gui/ does not depend on scopy-core, which links it; severity
	// and value have no visual yet and are dropped. Same conversion as
	// DecoderOverlay::setAnnotations (src/sim/DecoderOverlay.cpp:118-127).
	QVector<AnnotationSpan> spans;
	spans.reserve(anns->size());
	for(const scopy::acq::Annotation &a : *anns) {
		AnnotationSpan s;
		s.startSample = a.startSample;
		s.endSample = a.endSample;
		s.klass = a.klass;
		s.text = a.text;
		spans.append(s);
	}

	// plotSize, not spans.size(): the count is the span the offsets are laid out
	// against, which is what aligns a decode with the curves above it. Without it the
	// sample offsets would be read as X scale values directly.
	m_item->setSampleCount(static_cast<quint64>(qMax(1, plotSize)));
	m_item->setAnnotations(spans);
}

void AcqAnnotationChannel::reset()
{
	if(m_item && !m_plot.isNull()) {
		m_item->clear();
	}
}

void AcqAnnotationChannel::onEnabledChanged(bool en)
{
	if(m_plot.isNull()) {
		return;
	}
	if(m_item) {
		m_item->setVisible(en);
	}
	if(!m_handle.isNull() && m_handle->handle()) {
		m_handle->handle()->setVisible(en);
	}
}

void AcqAnnotationChannel::onColorChanged(const QColor &c)
{
	if(m_plot.isNull()) {
		return;
	}
	// The handle only. AnnotationCurve colours per annotation class from a stable hash
	// of the class name, so there is no single colour to set on the band — and
	// overriding every class to one colour would make a decode unreadable.
	if(!m_handle.isNull() && m_handle->handle()) {
		m_handle->handle()->setColor(c);
	}
}

#include "moc_acqannotationchannel.cpp"
