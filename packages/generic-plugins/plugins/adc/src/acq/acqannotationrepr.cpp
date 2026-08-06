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

#include "acqannotationrepr.h"

#include "acqplotrow.h"

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

AnnotationRepr::AnnotationRepr() = default;

AnnotationRepr::~AnnotationRepr() { detach(); }

void AnnotationRepr::attach(AcqPlotRow *row, const QString &name, const QColor &color)
{
	if(!row || !row->plot() || m_item) {
		return;
	}
	PlotAxis *yAxis = row->digitalAxis();
	if(!yAxis) {
		return;
	}

	m_plot = row->plot();
	m_row = row;
	m_color = color;

	m_handle = new PlotAxisHandle(m_plot.data(), yAxis);
	m_handle->handle()->setBarVisibility(BarVisibility::ON_HOVER);
	m_handle->handle()->setColor(color.isValid() ? color : Style::getColor(json::theme::content_silent));
	// SOUTH_OR_EAST, i.e. the right edge, where DigitalRepr uses the left. A decode
	// band is usually stacked directly under the logic track it decodes, and two
	// handles at the same height on the same side would overlap.
	m_handle->handle()->setHandlePos(HandlePos::SOUTH_OR_EAST);
	m_plot->addPlotAxisHandle(m_handle);

	// See DigitalRepr::attach — the handle parents itself to the canvas and filters
	// its events, so nothing here may reparent, resize or raise it.
	m_handle->handle()->setVisible(true);
	m_handle->setPosition(row->nextDigitalSlot());

	QObject::connect(m_handle.data(), &PlotAxisHandle::scalePosChanged, m_handle.data(), [this](double) {
		if(!m_plot.isNull() && m_plot->plot()) {
			m_plot->plot()->replot();
		}
	});

	m_item = new AnnotationCurve(name, m_plot->xAxis(), yAxis, m_handle.data());
	m_item->setVisible(m_enabled);
	m_item->attach(m_plot->plot());
}

void AnnotationRepr::detach()
{
	// Same discipline as DigitalRepr: the plot and the channel owning this repr are
	// siblings, so the QwtPlot may already have gone and auto-detached its items.
	if(m_plot.isNull()) {
		m_item = nullptr;
		m_handle = nullptr;
		m_row = nullptr;
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

	m_row = nullptr;
}

std::size_t AnnotationRepr::claimDepth(int plotSize, std::size_t bufferSize) const
{
	Q_UNUSED(plotSize)
	Q_UNUSED(bufferSize)
	// One chunk, whatever the window. An annotation read is latestAs<>() — and even
	// SampleBuffer::window() returns an annotation stream from the newest chunk alone,
	// because the offsets in each record are relative to the window the decoder ran
	// on and stitching would need shiftAnnotations() to re-anchor them. Claiming more
	// would retain history nothing can read.
	return 1u;
}

void AnnotationRepr::pull(scopy::acq::DataStore *store, const scopy::acq::DataKey &key, int plotSize)
{
	if(!store || !m_item || m_plot.isNull()) {
		return;
	}

	// latestAs, not window(): narrowing to the annotation alternative is the check for
	// "this key is not an annotation stream" as well as the read, so a channel pointed
	// at a numeric key by mistake draws nothing rather than misreading it.
	const std::optional<QVector<scopy::acq::Annotation>> anns =
		store->latestAs<QVector<scopy::acq::Annotation>>(key);
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

void AnnotationRepr::reset()
{
	if(m_item && !m_plot.isNull()) {
		m_item->clear();
	}
}

void AnnotationRepr::setEnabled(bool en)
{
	m_enabled = en;
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

void AnnotationRepr::setColor(const QColor &c)
{
	m_color = c;
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
