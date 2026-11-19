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

#include "acqplot.h"

#include "acqaxis.h"
#include "acqchannel.h"

#include <gui/cursorcontroller.h>
#include <gui/plotaxis.h>
#include <gui/plotchannel.h>
#include <gui/plotwidget.h>
#include <gui/style.h>
#include <gui/style_attributes.h>
#include <gui/widgets/cursorsettings.h>
#include <gui/widgets/plotlegend.h>

#include <qwt_axis.h>
#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_item.h>
#include <qwt_scale_map.h>

#include <QApplication>
#include <QDebug>
#include <QFontMetrics>
#include <QPainter>
#include <QPen>

using namespace scopy;
using namespace scopy::adc;

AcqPlot::AcqPlot(const QString &name, AcqPlotKind kind, quint32 uuid, QWidget *parent)
	: QObject(parent)
	, m_kind(kind)
	, m_uuid(uuid)
	, m_name(name)
	, m_kSlotPitchPx(26.0)
	, m_kTopMargin(0.10)
{
	// Which widget class a kind means is acqplotkind.cpp's business, not this class's —
	// which is what keeps every concrete widget type out of AcqPlot and AcqPlotManager.
	m_plot = createPlotWidget(kind, parent);

	// The built-in X and Y axes are deliberately left exactly as Qwt made them. X is
	// per channel now, so there is no plot-wide X unit to set; the built-in pair stays
	// as the fallback for a kind whose channels take no pooled axes (a waterfall).

	if(!m_plot.isNull()) {
		// Explicitly, both sides: PlotWidget's constructor never initialises
		// m_showXAxisLabels/m_showYAxisLabels, so until something calls a setter the
		// flags hold whatever was on the stack and the first showAxisLabels() is a coin
		// toss.
		m_plot->setShowXAxisLabels(m_showLabels);
		m_plot->setShowYAxisLabels(m_showLabels);
		m_plot->showAxisLabels();

		// PlotWidget::selectChannel() calls showAxisLabels() itself, which re-reads those
		// flags and re-shows the newly selected channel's pair. Harmless while labels are
		// on; with them off it would put back exactly what the reader hid, so the state
		// has to be re-asserted after every selection.
		connect(m_plot.data(), &PlotWidget::channelSelected, this, [this](PlotChannel *) {
			if(!m_showLabels && !m_plot.isNull()) {
				m_plot->hideAxisLabels();
			}
		});
	}
}

AcqPlot::~AcqPlot()
{
	// No axis is deleted here, neither the pool's nor m_digitalAxis. PlotWidget has no
	// removePlotAxis (gui/include/gui/plotwidget.h:73) and PlotAxis's destructor does
	// not unregister itself from PlotWidget::m_plotAxis, so deleting one would leave a
	// dangling entry for PlotNavigator to walk. They are parented to this object, so Qt
	// frees the QObjects — which is exactly as much as is safe.

	// A plot that went first has already deleted every item still attached to it.
	if(!m_plot.isNull()) {
		delete m_bufferBand;
	}
}

PlotWidget *AcqPlot::plot() const { return m_plot.data(); }

QString AcqPlot::menuId() const { return QStringLiteral("acqplot:%1").arg(m_uuid); }

void AcqPlot::setName(const QString &n)
{
	if(m_name == n) {
		return;
	}
	m_name = n;
	Q_EMIT nameChanged(m_name);
}

void AcqPlot::addChannelRef(AcqChannel *ch)
{
	if(!ch || m_channels.contains(ch)) {
		return;
	}
	m_channels.append(ch);
}

void AcqPlot::removeChannelRef(AcqChannel *ch)
{
	m_channels.removeOne(ch);
	// Its markers leave with it. The next source draws its own next frame.
	setBufferEdges(ch, {});
}

bool AcqPlot::supportsPerChannelAxes() const { return plotKindPoolsAxes(m_kind); }

PlotAxis *AcqPlot::axisForSource(int position, const QString &sourceId)
{
	if(m_plot.isNull()) {
		return nullptr;
	}

	const QPair<int, QString> id(position, sourceId);
	if(PlotAxis *existing = m_sourceAxes.value(id, nullptr)) {
		// Nothing is reset: the axis is still in use and its unit and interval describe
		// the source both channels read.
		return existing;
	}

	// The PlotWidget * overload specifically: it self-registers through addPlotAxis(this),
	// which grows the QwtPlot's axis count and hands the axis to the navigator. The pen
	// matches PlotWidget::setupAxes()'s built-in pair, which a waterfall shows directly.
	QPen pen(Style::getColor(json::theme::content_subtle));
	PlotAxis *ax = new PlotAxis(position, m_plot.data(), pen, this);
	m_sourceAxes.insert(id, ax);
	return ax;
}

PlotAxis *AcqPlot::digitalAxis()
{
	if(m_digitalAxis) {
		return m_digitalAxis;
	}
	if(m_plot.isNull()) {
		return nullptr;
	}

	// A stacked extra YLeft axis, so handles render on the left of the canvas next to
	// the main analog Y axis. Fixed [0,1] with ticks and labels hidden: items self-place
	// using pixel offsets from their handle, so the axis exists only to host them.
	QPen pen(Style::getColor(json::theme::content_silent));
	m_digitalAxis = new PlotAxis(QwtAxis::YLeft, m_plot.data(), pen, this);
	m_digitalAxis->setInterval(0.0, 1.0);
	m_digitalAxis->setVisible(false);
	return m_digitalAxis;
}

double AcqPlot::nextDigitalSlot()
{
	PlotAxis *ax = digitalAxis();
	if(!ax) {
		return 1.0;
	}

	if(!m_slotInit) {
		// Start below the top so the first band has headroom above it.
		const double span = ax->max() - ax->min();
		m_nextSlot = ax->max() - m_kTopMargin * span;
		m_slotInit = true;
	}

	const double pos = m_nextSlot;

	// Pitch pixels to scale delta. The axis is [0,1] across the canvas height, so the
	// conversion is exact once the canvas has a height; before first layout there is
	// nothing to measure and we fall back to a nominal 2% step, which later handle
	// drags can correct.
	double delta = 0.02;
	if(!m_plot.isNull() && m_plot->plot() && m_plot->plot()->canvas()) {
		const int h = m_plot->plot()->canvas()->height();
		if(h > 0) {
			delta = m_kSlotPitchPx / static_cast<double>(h);
		}
	}
	m_nextSlot = pos - delta;
	return pos;
}

void AcqPlot::setShowLabels(bool on)
{
	if(m_showLabels == on) {
		return;
	}
	m_showLabels = on;
	if(!m_plot.isNull()) {
		// The flags first: hideAxisLabels() ignores them and forces both axes down, but
		// showAxisLabels() reads them, and so does the selectChannel() path.
		m_plot->setShowXAxisLabels(on);
		m_plot->setShowYAxisLabels(on);
		if(on) {
			m_plot->showAxisLabels();
		} else {
			m_plot->hideAxisLabels();
		}
		m_plot->replot();
	}
	Q_EMIT showLabelsChanged(on);
}

void AcqPlot::setShowLegend(bool on)
{
	if(m_showLegend == on) {
		return;
	}
	m_showLegend = on;

	if(on && m_legend.isNull() && !m_plot.isNull()) {
		// Built against the plot widget, which it also parents its panel into. It
		// backfills the channels already there and follows addedChannel/removedChannel
		// from here on, so construction order against the channels does not matter.
		m_legend = new PlotLegend(m_plot.data(), m_plot.data());
	}
	if(!m_legend.isNull()) {
		// PlotLegend::setVisible is an override that also drives the HoverWidget holding
		// the panel; going through the base QWidget one would leave the overlay behind.
		m_legend->setVisible(on);
	}
	Q_EMIT showLegendChanged(on);
}

CursorController *AcqPlot::cursors()
{
	if(!m_cursors.isNull()) {
		return m_cursors.data();
	}
	if(m_plot.isNull()) {
		return nullptr;
	}
	m_cursors = new CursorController(m_plot.data(), this);
	// Off until asked for. The handles exist either way — this is what keeps them from
	// being drawn on a plot the reader has not turned cursors on for.
	m_cursors->setVisible(m_showCursors);
	return m_cursors.data();
}

CursorSettings *AcqPlot::cursorSettings(QWidget *parent)
{
	if(!m_cursorSettings.isNull()) {
		return m_cursorSettings.data();
	}
	CursorController *ctrl = cursors();
	if(!ctrl) {
		return nullptr;
	}
	m_cursorSettings = new CursorSettings(parent);
	// connectSignals wires the page's buttons to this controller and calls
	// updateSession(), which replays every button's current state — so the controller
	// comes up agreeing with the page rather than with its own defaults.
	ctrl->connectSignals(m_cursorSettings.data());
	// After connectSignals: updateSession() replays the X/Y enable switches, which turn
	// the cursors on as a side effect. This has the last word.
	ctrl->setVisible(m_showCursors);
	return m_cursorSettings.data();
}

void AcqPlot::setShowCursors(bool on)
{
	if(m_showCursors == on) {
		return;
	}
	m_showCursors = on;
	// Only builds the controller when turning them on: a plot switched off and never on
	// should not pay for four handles and a readout overlay.
	if(on) {
		if(CursorController *ctrl = cursors()) {
			ctrl->setVisible(true);
		}
	} else if(!m_cursors.isNull()) {
		m_cursors->setVisible(false);
	}
	Q_EMIT showCursorsChanged(on);
}

// The buffer markers. One item for both the lines and the strip, so they cannot drift
// apart. Opaque throughout — colours mixed into the background instead of alpha — so
// drawing them costs no blending.
class scopy::adc::AcqBufferBand : public QwtPlotItem
{
public:
	AcqBufferBand()
	{
		setItemAttribute(QwtPlotItem::AutoScale, false);
		setItemAttribute(QwtPlotItem::Legend, false);
		// Under the traces, which draw at z 20.
		setZ(19.0);
	}

	QVector<double> edges;
	QColor line, fill, fillAlt;

	void draw(QPainter *p, const QwtScaleMap &xMap, const QwtScaleMap &, const QRectF &canvas) const override
	{
		// Past this many buffers the blocks are slivers and the lines a wall.
		if(edges.size() < 2 || edges.size() > 513) {
			return;
		}
		const double bottom = canvas.bottom() - 3.0;
		const double top = bottom - 16.0;
		p->save();
		p->setClipRect(canvas);

		// Dashes in pen widths: 6 px dash, 10 px gap. Down to the strip, not through it.
		QPen pen(line, 2);
		pen.setDashPattern({3, 5});
		pen.setCapStyle(Qt::FlatCap);
		p->setPen(pen);
		for(int i = 1; i + 1 < edges.size(); ++i) {
			const double x = xMap.transform(edges[i]);
			p->drawLine(QPointF(x, canvas.top()), QPointF(x, top));
		}

		QFont f = QApplication::font();
		f.setPixelSize(10);
		p->setFont(f);
		const QFontMetrics fm(f);
		for(int i = 0; i + 1 < edges.size(); ++i) {
			const double x1 = xMap.transform(edges[i]);
			const double x2 = xMap.transform(edges[i + 1]);
			// 1 px in from each edge, so neighbours read as separate blocks.
			const QRectF r(QPointF(qMin(x1, x2) + 1.0, top), QPointF(qMax(x1, x2) - 1.0, bottom));
			if(r.width() < 1.0) {
				continue;
			}
			p->setPen(Qt::NoPen);
			p->setBrush(i % 2 ? fillAlt : fill);
			p->drawRoundedRect(r, 3.0, 3.0);
			const QString label = QStringLiteral("#%1").arg(i);
			if(fm.horizontalAdvance(label) + 6 <= r.width()) {
				p->setPen(Style::getColor(json::theme::content_default));
				p->drawText(r, Qt::AlignCenter, label);
			}
		}
		p->restore();
	}
};

void AcqPlot::setShowBufferStarts(bool on)
{
	if(m_showBufferStarts == on) {
		return;
	}
	m_showBufferStarts = on;
	// The markers arrive with the next frame's read, so turning this on while stopped
	// shows them on the next run.
	if(!on) {
		setBufferEdges(m_bufferFrom, {});
	}
	Q_EMIT showBufferStartsChanged(on);
}

bool AcqPlot::wantsBufferEdges(const AcqChannel *ch) const
{
	if(!m_showBufferStarts || !ch) {
		return false;
	}
	PlotChannel *sel = m_plot.isNull() ? nullptr : m_plot->selectedChannel();
	const AcqChannel *first = nullptr;
	for(const AcqChannel *c : m_channels) {
		// Only curves and logic tracks have buffers to mark.
		const QString kind = c->kindName();
		if(!c->isEnabled() || (kind != QLatin1String("CURVE") && kind != QLatin1String("DIGITAL"))) {
			continue;
		}
		if(sel && c->plotChannel() == sel) {
			return c == ch;
		}
		if(!first) {
			first = c;
		}
	}
	return first == ch;
}

void AcqPlot::setBufferEdges(const AcqChannel *from, const QVector<double> &edges)
{
	if(m_plot.isNull() || !from) {
		return;
	}
	const bool clear = edges.size() < 2;
	if(!(clear ? from == m_bufferFrom : wantsBufferEdges(from))) {
		return;
	}
	m_bufferFrom = clear ? nullptr : from;
	if(!m_bufferBand) {
		if(clear) {
			return;
		}
		m_bufferBand = new AcqBufferBand();
		m_bufferBand->attach(m_plot->plot());
	}
	m_bufferBand->edges = clear ? QVector<double>{} : edges;
	if(!clear) {
		// The channel colour pulled toward gray, then mixed into the theme's background
		// (the canvas is coloured by stylesheet, so QwtPlot::canvasBackground() is not it).
		QColor fg = (from->color().isValid() ? from->color() : Style::getColor(json::theme::content_silent))
				    .toHsl();
		fg.setHslF(fg.hslHueF(), fg.hslSaturationF() * 0.4, fg.lightnessF());
		fg = fg.toRgb();
		const QColor bg = Style::getColor(json::theme::background_plot);
		auto mix = [&](double k) {
			return QColor::fromRgbF(fg.redF() * k + bg.redF() * (1 - k),
						fg.greenF() * k + bg.greenF() * (1 - k),
						fg.blueF() * k + bg.blueF() * (1 - k));
		};
		m_bufferBand->line = mix(0.75);
		m_bufferBand->fill = mix(0.45);
		m_bufferBand->fillAlt = mix(0.3);
		// The channel's own X; the Y is unused, the strip being placed in pixels.
		PlotAxis *x = from->xAxis() && from->xAxis()->plotAxis() ? from->xAxis()->plotAxis() : m_plot->xAxis();
		m_bufferBand->setXAxis(x->axisId());
	}
	m_bufferBand->itemChanged();
}

QVector<double> AcqPlot::bufferEdges() const
{
	return m_bufferBand && !m_plot.isNull() ? m_bufferBand->edges : QVector<double>{};
}

void AcqPlot::replot()
{
	if(m_plot.isNull()) {
		return;
	}
	m_plot->replot();
}

#include "moc_acqplot.cpp"
