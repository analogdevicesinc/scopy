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
#include <qwt_scale_map.h>

#include <QMouseEvent>
#include <QToolTip>

using namespace scopy;
using namespace scopy::adc;

// Any stream. Only an annotation one has anything to show here: readData's latestAs<>()
// narrows to the annotation alternative, so a numeric key clears the bands rather than
// being misread.
REGISTER_ACQ_CHANNEL_KIND(AcqAnnotationChannel, scopy::acq::ReprKind::Annotations)

AcqAnnotationChannel::AcqAnnotationChannel(const Args &args)
	: AcqChannel(args)
{
	m_hoverTimer.setSingleShot(true);
	m_hoverTimer.setInterval(250);
	connect(&m_hoverTimer, &QTimer::timeout, this, &AcqAnnotationChannel::showPendingTooltip);

	// Grey, not a palette slot: AnnotationCurve hashes a hue per annotation class, so a
	// channel colour here would name a colour found nowhere in the band itself.
	setColor(Style::getColor(json::theme::content_silent));
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
	// The left edge, the same side AcqDigitalChannel uses.
	m_handle->handle()->setHandlePos(HandlePos::NORTH_OR_WEST);
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

	PlotAxis *x = xAxis() ? xAxis()->plotAxis() : m_plot->xAxis();
	m_item = new AnnotationCurve(name(), x, digAxis, m_handle.data());
	m_item->attach(m_plot->plot());

	// Required for the hover tooltips below: without tracking, Qt delivers MouseMove only
	// while a button is held. Same as DecoderOverlay (src/sim/DecoderOverlay.cpp:45).
	if(m_plot->plot() && m_plot->plot()->canvas()) {
		m_plot->plot()->canvas()->setMouseTracking(true);
	}

	connect(m_plot.data(), &PlotWidget::mouseMove, this, [this](const QMouseEvent *ev) {
		if(ev) {
			onCanvasMouseMove(ev->position().toPoint(), ev->globalPosition().toPoint());
		}
	});
}

void AcqAnnotationChannel::detachFrom()
{
	// On both paths: a pending fire would hit-test the curve this deletes.
	m_hoverTimer.stop();
	hideTip();

	// Same discipline as AcqDigitalChannel: the plot and this channel are siblings, so
	// the QwtPlot may already have gone and auto-detached its items.
	if(m_plot.isNull()) {
		m_item = nullptr;
		m_handle = nullptr;
		return;
	}

	// A lambda cannot ask for Qt::UniqueConnection, so a re-attach to another plot would
	// leave this channel listening to both canvases.
	disconnect(m_plot.data(), &PlotWidget::mouseMove, this, nullptr);

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
	// One chunk whatever the window: an annotation stream is only ever read from the newest
	// chunk, because the offsets in a record are relative to the window it was decoded on.
	return DepthNeed::chunks(1);
}

void AcqAnnotationChannel::readData(scopy::acq::DataStore *store, int plotSize)
{
	// The reads go through the base, which serves a trigger fire's own window when there is
	// one and falls back to this store otherwise.
	Q_UNUSED(store)
	if(!m_item || m_plot.isNull()) {
		return;
	}

	// Narrowing to the annotation alternative is the check for "this key is not an annotation
	// stream" as well as the read.
	const std::optional<QVector<scopy::acq::Annotation>> anns =
		latestAsFor<QVector<scopy::acq::Annotation>>(key());
	if(!anns) {
		// Absent, empty, or the wrong type — a stale decode on screen would read as current.
		m_item->clear();
		return;
	}

	// acq::Annotation -> gui::AnnotationSpan. The drawing side keeps its own struct so gui/
	// does not depend on scopy-core; severity and value have no visual yet and are dropped.
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

	// plotSize, not spans.size(): the count is the span the offsets are laid out against,
	// which is what aligns a decode with the curves above it.
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
		// Explicitly: QwtPlotItem::setVisible only routes to autoRefresh(), which does nothing
		// while autoReplot is off — and it is off here, so the band would stay on screen.
		if(m_plot->plot()) {
			m_plot->plot()->replot();
		}
	}
	if(!m_handle.isNull() && m_handle->handle()) {
		m_handle->handle()->setVisible(en);
	}
	if(!en) {
		// A tip left up would describe a band that is no longer drawn.
		m_hoverTimer.stop();
		hideTip();
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

void AcqAnnotationChannel::hideTip()
{
	if(m_lastTip.isEmpty()) {
		return;
	}
	QToolTip::hideText();
	m_lastTip.clear();
	m_lastTipGlobal = QPoint();
}

void AcqAnnotationChannel::onCanvasMouseMove(QPoint canvasPos, QPoint globalPos)
{
	m_pendingCanvasPos = canvasPos;
	m_pendingGlobalPos = globalPos;

	// >2 px, not any move: a tooltip appearing or closing synthesises a MouseMove at the
	// unchanged position, and treating that as a move starts a feedback loop.
	const int dx = globalPos.x() - m_lastTipGlobal.x();
	const int dy = globalPos.y() - m_lastTipGlobal.y();
	if(!m_lastTip.isEmpty() && (dx * dx + dy * dy) > 4) {
		hideTip();
	}

	m_hoverTimer.start();
}

void AcqAnnotationChannel::showPendingTooltip()
{
	if(!m_item || m_plot.isNull() || !m_plot->plot()) {
		return;
	}
	QWidget *cv = m_plot->plot()->canvas();
	// underMouse, because there is no Leave to hook: PlotWidget re-emits MouseMove only.
	if(!cv || !cv->underMouse()) {
		hideTip();
		return;
	}
	if(!m_item->isVisible()) {
		hideTip();
		return;
	}

	const QRectF canvasRect(0, 0, cv->width(), cv->height());
	// The item's own axes, not the plot's: channels here draw against a per-source X axis.
	const std::optional<AnnotationSpan> hit =
		m_item->hitTest(m_pendingCanvasPos, m_plot->plot()->canvasMap(m_item->xAxis()),
				m_plot->plot()->canvasMap(m_item->yAxis()), canvasRect);
	if(!hit) {
		hideTip();
		return;
	}

	QString tip;
	if(!hit->text.isEmpty()) {
		tip += QStringLiteral("<b>%1</b>").arg(hit->text.toHtmlEscaped());
	}
	if(!hit->klass.isEmpty()) {
		if(!tip.isEmpty()) {
			tip += QStringLiteral("<br/>");
		}
		tip += QStringLiteral("<i>%1</i>").arg(hit->klass.toHtmlEscaped());
	}
	QString range;
	if(hit->startSample == hit->endSample) {
		range = tr("sample %1").arg(hit->startSample);
	} else {
		range = tr("samples %1 – %2 (%3)")
				.arg(hit->startSample)
				.arg(hit->endSample)
				.arg(hit->endSample - hit->startSample);
	}
	if(!tip.isEmpty()) {
		tip += QStringLiteral("<br/>");
	}
	tip += QStringLiteral("<span style=\"color:gray;\">%1 · %2</span>").arg(name().toHtmlEscaped(), range);

	// Re-showing an unchanged tip would move it to the cursor on every fire.
	if(tip == m_lastTip) {
		return;
	}

	QToolTip::showText(m_pendingGlobalPos, tip, cv);
	m_lastTip = tip;
	m_lastTipGlobal = m_pendingGlobalPos;
}

#include "moc_acqannotationchannel.cpp"
