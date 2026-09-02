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

#include "plotaxis.h"

#include "insidescaledraw.h"

#include <QDebug>
#include <QwtPlotLayout>
#include <qwt_scale_widget.h>
#include <edgelessplot.h>
#include <pluginbase/preferences.h>
#include <style.h>

using namespace scopy;

namespace {
/*
 * A QwtPlotScaleItem with a borderDistance >= 0 is pinned to a canvas edge using the
 * *opposite* of its alignment — a BottomScale lands on canvasRect.top(), a RightScale on
 * canvasRect.left(). So asking for the mirrored alignment is what draws the labels inward.
 */
QwtScaleDraw::Alignment insideAlignment(int position)
{
	switch(position) {
	case QwtAxis::XBottom:
		return QwtScaleDraw::TopScale;
	case QwtAxis::XTop:
		return QwtScaleDraw::BottomScale;
	case QwtAxis::YLeft:
		return QwtScaleDraw::RightScale;
	case QwtAxis::YRight:
	default:
		return QwtScaleDraw::LeftScale;
	}
}
} // namespace

PlotAxis::PlotAxis(int position, QwtPlot *plot, int axisIndex, QPen pen, QObject *parent)
	: QObject(parent)
	, m_plotWidget(nullptr)
	, m_plot(plot)
	, m_position(position)
	, m_axisId(QwtAxisId(position, axisIndex))
	, m_units("")
{
	m_min = -1;
	m_max = 1;
	m_divs = 11.0;
	m_id = axisIndex;

	m_plot->setAxesCount(position, m_id + 1);

	updateAxisScale();

	m_formatter = new MetricPrefixFormatter();
	m_formatter->setTrimZeroes(true);
	m_formatter->setTwoDecimalMode(false);
	m_scaleDraw = new BasicScaleDraw(m_formatter, m_units);

	m_scaleDraw->setColor(pen.color());
	m_plot->setAxisScaleDraw(m_axisId, m_scaleDraw);

	m_scaleEngine = new OscScaleEngine();
	m_scaleEngine->setMajorTicksCount(m_divs);
	m_plot->setAxisScaleEngine(m_axisId, (QwtScaleEngine *)m_scaleEngine);

	// no addPlotAxis call — caller (WaterfallPlotWidget) tracks axes itself

	setupAxisScale();
	setVisible(false);

	connect(this, &PlotAxis::minChanged, this, &PlotAxis::updateAxisScale);
	connect(this, &PlotAxis::maxChanged, this, &PlotAxis::updateAxisScale);
	setUnitsVisible(false);
}

PlotAxis::PlotAxis(int position, PlotWidget *p, QPen pen, QObject *parent)
	: QObject(parent)
	, m_plotWidget(p)
	, m_plot(p->plot())
	, m_position(position)
	, m_axisId(QwtAxisId(position))
	, m_units("")
{
	m_min = -1;
	m_max = 1;
	m_divs = (isHorizontal()) ? 11.0 : 11.0;

	m_id = m_plotWidget->plotAxis(m_position).count();
	m_axisId = QwtAxisId(m_position, m_id);
	m_plot->setAxesCount(position, m_id + 1);

	updateAxisScale();

	m_formatter = new MetricPrefixFormatter();
	m_formatter->setTrimZeroes(true);
	m_formatter->setTwoDecimalMode(false);
	m_scaleDraw = new BasicScaleDraw(m_formatter, m_units);

	m_scaleDraw->setColor(pen.color());
	m_plot->setAxisScaleDraw(m_axisId, m_scaleDraw);

	m_scaleEngine = new OscScaleEngine();
	m_scaleEngine->setMajorTicksCount(m_divs);
	m_plot->setAxisScaleEngine(m_axisId, (QwtScaleEngine *)m_scaleEngine);

	m_plotWidget->addPlotAxis(this);

	setupAxisScale();
	setVisible(false);

	connect(this, &PlotAxis::minChanged, this, &PlotAxis::updateAxisScale);
	connect(this, &PlotAxis::maxChanged, this, &PlotAxis::updateAxisScale);
	setUnitsVisible(false);
}

PlotAxis::~PlotAxis() {}

// The inside labels are drawn off a second BasicScaleDraw, so every formatting setter has to
// reach it as well or the two scales disagree about units and precision.
BasicScaleDraw *PlotAxis::insideScaleDraw() const
{
	return m_labelsInside ? dynamic_cast<BasicScaleDraw *>(m_scaleItem->scaleDraw()) : nullptr;
}

void PlotAxis::setUnitsVisible(bool visible)
{
	m_scaleDraw->setUnitsEnabled(visible);
	if(BasicScaleDraw *draw = insideScaleDraw()) {
		draw->setUnitsEnabled(visible);
	}
}

void PlotAxis::setScaleEn(bool en)
{
	if(en) {
		m_scaleItem->attach(m_plot);
	} else {
		m_scaleItem->detach();
	}
}

void PlotAxis::setupAxisScale()
{
	QwtScaleDraw::Alignment scale = static_cast<QwtScaleDraw::Alignment>(m_position);
	m_scaleItem = new EdgelessPlotScaleItem(scale);

	m_scaleItem->scaleDraw()->setAlignment(scale);
	m_scaleItem->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Backbone, false);
	m_scaleItem->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Labels, false);
	m_scaleItem->setFont(m_plot->axisWidget(0)->font());

	QPalette palette = m_scaleItem->palette();
	palette.setBrush(QPalette::WindowText, QColor(0x6E6E6F));
	palette.setBrush(QPalette::Text, QColor(0x6E6E6F));
	m_scaleItem->setPalette(palette);
	m_scaleItem->setBorderDistance(0);
	m_scaleItem->setZ(200);

	setLabelsInside(Preferences::get("plot_labels_inside").toBool());
	connect(Preferences::GetInstance(), &Preferences::preferenceChanged, this,
		[this](QString preference, QVariant value) {
			if(preference == "plot_labels_inside") {
				setLabelsInside(value.toBool());
			}
		});
}

bool PlotAxis::labelsInside() const { return m_labelsInside; }

void PlotAxis::setLabelsInside(bool inside)
{
	if(m_labelsInside == inside) {
		return;
	}

	m_labelsInside = inside;

	// The item's scale draw cannot be m_scaleDraw: that one belongs to the external
	// QwtScaleWidget, and setScaleDraw() takes ownership and deletes whatever it held.
	if(inside) {
		auto *draw = new InsideScaleDraw(m_formatter, m_units);
		draw->setUnitsEnabled(m_scaleDraw->unitsEnabled());
		draw->setFloatPrecision(m_scaleDraw->getFloatPrecison());
		m_scaleItem->setScaleDraw(draw);
	} else {
		// A plain QwtScaleDraw, not a BasicScaleDraw — the latter's ctor disables Ticks,
		// which would silently kill the tick-marks decoration this item draws in outside
		// mode. Detach too, so outside mode is byte-for-byte the pre-feature behaviour.
		m_scaleItem->setScaleDraw(new QwtScaleDraw());
		m_scaleItem->detach();
	}

	QwtScaleDraw *draw = m_scaleItem->scaleDraw();
	draw->setAlignment(inside ? insideAlignment(m_position) : static_cast<QwtScaleDraw::Alignment>(m_position));
	draw->enableComponent(QwtAbstractScaleDraw::Backbone, false);
	draw->enableComponent(QwtAbstractScaleDraw::Labels, inside);

	// The end major ticks carry the interval bounds, so inside mode needs them back —
	// without this a 0..1 axis reads "0.2 0.4 0.6 0.8".
	m_scaleItem->setEdgeless(!inside);

	// Inside labels sit over the traces, so they get the high-contrast content colour
	// rather than the muted one the external scales use. A few pixels of border keep the
	// vertical labels off the canvas frame.
	QColor labelColor = inside ? QColor(Style::getAttribute(json::theme::content_default)) : QColor(0x6E6E6F);
	QPalette palette = m_scaleItem->palette();
	palette.setBrush(QPalette::WindowText, labelColor);
	palette.setBrush(QPalette::Text, labelColor);
	m_scaleItem->setPalette(palette);
	m_scaleItem->setBorderDistance(inside && isVertical() ? 4 : 0);

	if(auto *basicDraw = dynamic_cast<BasicScaleDraw *>(draw)) {
		basicDraw->setColor(labelColor);
	}

	// Bind the item to this axis; otherwise it tracks Qwt's default index 0 and shows
	// numbers off the wrong scale as soon as a second axis shares this position — the
	// normal case for AcqPlot's pooled axes.
	if(isHorizontal()) {
		m_scaleItem->setXAxis(m_axisId);
	} else {
		m_scaleItem->setYAxis(m_axisId);
	}

	setVisible(m_visible);
}

int PlotAxis::position() { return m_position; }

bool PlotAxis::isHorizontal() { return (m_position == QwtAxis::XBottom || m_position == QwtAxis::XTop); }

bool PlotAxis::isVertical() { return (m_position == QwtAxis::YLeft || m_position == QwtAxis::YRight); }

double PlotAxis::divs() const { return (m_divs - 1); }

void PlotAxis::setFormatter(PrefixFormatter *formatter)
{
	m_formatter = formatter;
	m_scaleDraw->setFormatter(m_formatter);
	if(BasicScaleDraw *draw = insideScaleDraw()) {
		draw->setFormatter(m_formatter);
	}

	Q_EMIT formatterChanged(formatter);
}

PrefixFormatter *PlotAxis::getFormatter() { return m_formatter; }

void PlotAxis::setUnits(QString units)
{
	m_units = units;
	m_scaleDraw->setUnitType(m_units);
	if(BasicScaleDraw *draw = insideScaleDraw()) {
		draw->setUnitType(m_units);
	}

	Q_EMIT unitsChanged(units);
}

QString PlotAxis::getUnits() { return m_units; }

void PlotAxis::setDivs(double divs)
{
	m_divs = divs;
	updateAxisScale();
}

void PlotAxis::setInterval(double min, double max)
{
	setMin(min);
	setMax(max);
	//	updateAxisScale();
}

void PlotAxis::setVisible(bool val)
{
	m_visible = val;

	if(m_labelsInside) {
		// Kept unconditionally hidden: that is what stops QwtPlotLayout reserving the
		// strip beside the canvas, which is the whole point of the feature.
		m_plot->setAxisVisible(m_axisId, false);
		if(val) {
			m_scaleItem->attach(m_plot);
		} else {
			m_scaleItem->detach();
		}
		// Attaching a plot item does not force a repaint the way setAxisVisible() does.
		m_plot->replot();
		return;
	}

	m_plot->setAxisVisible(m_axisId, val);
}

void PlotAxis::updateAxisScale()
{
	m_plot->setAxisScale(m_axisId, m_min, m_max, (m_max - m_min) / m_divs); // set Divs, limits
	m_plot->replot();
	Q_EMIT axisScaleUpdated();
}

void PlotAxis::setMin(double newMin)
{
	if(qFuzzyCompare(m_min, newMin))
		return;
	m_min = newMin;
	emit minChanged(newMin);
}

void PlotAxis::setMax(double newMax)
{
	if(qFuzzyCompare(m_max, newMax))
		return;
	m_max = newMax;
	emit maxChanged(newMax);
}

BasicScaleDraw *PlotAxis::scaleDraw() const { return m_scaleDraw; }

OscScaleEngine *PlotAxis::scaleEngine() const { return m_scaleEngine; }

double PlotAxis::max() const { return m_max; }

double PlotAxis::visibleMin() const { return m_plot->axisScaleDiv(axisId()).lowerBound(); }

double PlotAxis::visibleMax() const { return m_plot->axisScaleDiv(axisId()).upperBound(); }

double PlotAxis::min() const { return m_min; }

const QwtAxisId &PlotAxis::axisId() const { return m_axisId; }

#include "moc_plotaxis.cpp"
