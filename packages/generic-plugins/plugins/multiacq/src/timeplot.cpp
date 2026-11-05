/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#include "timeplot.h"

#include <core/acq_engine/datastore.h>

#include <gui/basicscaledraw.h>
#include <gui/plotaxis.h>
#include <gui/plotchannel.h>
#include <gui/plotnavigator.hpp>
#include <gui/plotwidget.h>
#include <gui/widgets/plotlegend.h>

#include <QDateTime>
#include <QTimeZone>
#include <qwt_interval.h>
#include <qwt_scale_div.h>
#include <qwt_plot_curve.h>

#include <algorithm>
#include <cmath>

using namespace scopy;
using namespace scopy::multiacq;

namespace {

// Chunks the store keeps per key for us: the cycles that may pass between two frames.
constexpr std::size_t kRetainChunks = 256;

// Per curve, points kept at full detail: 10 minutes of a 1 kHz buffer. A faster channel keeps its whole
// history too, its older part thinned to fit (see TimeCurve::trim).
constexpr int kMaxPoints = 2000000;

// About this many labelled ticks across the axis: a time label is wide.
constexpr double kTargetTicks = 6;

// Round tick steps in ms, each with how many minor intervals it splits into.
const QList<QPair<double, int>> kSteps{
	{1, 5},	      {2, 4},	   {5, 5},	 {10, 5},     {20, 4},	    {50, 5},	 {100, 5},
	{200, 4},     {500, 5},	   {1e3, 5},	 {2e3, 4},    {5e3, 5},	    {10e3, 5},	 {15e3, 3},
	{30e3, 6},    {60e3, 6},   {120e3, 4},	 {300e3, 5},  {600e3, 5},   {900e3, 3},	 {1800e3, 6},
	{3600e3, 6},  {7200e3, 4}, {10800e3, 3}, {21600e3, 6}, {43200e3, 6}, {86400e3, 6}};

double nowMs() { return static_cast<double>(QDateTime::currentMSecsSinceEpoch()); }

} // namespace

TimeFormatter::TimeFormatter(QObject *parent)
	: PrefixFormatter({{QString(), 1.0}}, parent)
	, m_format(QStringLiteral("hh:mm:ss"))
{}

QString TimeFormatter::format(double value, QString unitType, int precision) const
{
	Q_UNUSED(unitType)
	Q_UNUSED(precision)
	return text(value, m_reference, m_format);
}

QString TimeFormatter::text(double value, double reference, const QString &format)
{
	const qint64 ms = qRound64(value);
	if(reference <= 0) {
		return QDateTime::fromMSecsSinceEpoch(ms).toString(format);
	}
	// Elapsed time, formatted as a UTC time of day so "hh" counts hours from zero.
	const qint64 elapsed = ms - qRound64(reference);
	const QString t = QDateTime::fromMSecsSinceEpoch(std::abs(elapsed), QTimeZone::UTC).toString(format);
	return elapsed < 0 ? QStringLiteral("-") + t : t;
}

double TimeScaleEngine::stepFor(double widthMs)
{
	for(const auto &[step, minors] : kSteps) {
		if(widthMs / step <= kTargetTicks) {
			return step;
		}
	}
	return kSteps.last().first;
}

QwtScaleDiv TimeScaleEngine::divideScale(double x1, double x2, int maxMajorSteps, int maxMinorSteps,
					 double stepSize) const
{
	Q_UNUSED(maxMajorSteps)
	Q_UNUSED(maxMinorSteps)
	Q_UNUSED(stepSize)
	const QwtInterval interval = QwtInterval(x1, x2).normalized();
	if(interval.width() <= 0) {
		return QwtScaleDiv();
	}

	const double step = stepFor(interval.width());
	int minors = 5;
	for(const auto &[s, m] : kSteps) {
		if(s == step) {
			minors = m;
		}
	}
	// Ticks fall where (t - origin) is a whole number of steps: on the reference, or on
	// the local clock (a whole minute here, not in UTC).
	const double origin = m_reference > 0
		? m_reference
		: -1000.0 * QDateTime::currentDateTime().offsetFromUtc();
	const double first = origin + std::ceil((interval.minValue() - origin) / step) * step;

	QList<double> ticks[QwtScaleDiv::NTickTypes];
	const double minorStep = step / minors;
	for(double t = first - step; t <= interval.maxValue(); t += step) {
		if(t >= interval.minValue()) {
			ticks[QwtScaleDiv::MajorTick].append(t);
		}
		for(int m = 1; m < minors; ++m) {
			const double mt = t + m * minorStep;
			if(mt >= interval.minValue() && mt <= interval.maxValue()) {
				ticks[QwtScaleDiv::MinorTick].append(mt);
			}
		}
	}
	QwtScaleDiv div(interval, ticks);
	if(x1 > x2) {
		div.invert();
	}
	return div;
}

TimeCurve::TimeCurve(const acq::DataKey &yKey, const acq::DataKey &tKey, const QString &name, const QColor &color,
		     PlotWidget *plot, acq::DataStore *store, QObject *parent)
	: QObject(parent)
	, m_yKey(yKey)
	, m_tKey(tKey)
	, m_claimant(QStringLiteral("multiacq:curve:%1").arg(reinterpret_cast<quintptr>(this)))
	, m_store(store)
	, m_plot(plot)
{
	m_ch = new PlotChannel(name, QPen(color, 1), plot->xAxis(), plot->yAxis(), this);
	plot->addPlotChannel(m_ch);
	// enable(), not setEnabled(): only enable() sets the flag isEnabled() reads, which is
	// what decides whether this curve counts for autoscale.
	m_ch->enable();
	// A buffered channel puts thousands of samples on every pixel column; drawing one point
	// per column keeps it cheap. Needs X in order, which the history always is.
	m_ch->curve()->setPaintAttribute(QwtPlotCurve::FilterPointsAggressive, true);
	refresh();

	// Claims may precede the keys; the store applies them on the first write.
	if(m_store) {
		m_store->claimDepth(m_yKey, m_claimant, acq::DataStore::Claim::chunks(kRetainChunks));
		m_store->claimDepth(m_tKey, m_claimant, acq::DataStore::Claim::chunks(kRetainChunks));
	}
}

TimeCurve::~TimeCurve()
{
	if(m_store) {
		m_store->releaseClaimant(m_claimant);
	}
	if(m_plot) {
		m_plot->removePlotChannel(m_ch);
	}
}

QString TimeCurve::name() const { return m_ch->name(); }

void TimeCurve::setVisible(bool visible)
{
	visible ? m_ch->enable() : m_ch->disable();
	if(m_plot) {
		m_plot->replot();
	}
}

QColor TimeCurve::color() const { return m_ch->pen().color(); }

void TimeCurve::pull()
{
	if(!m_store) {
		return;
	}
	// The two keys are written one after the other, so a write landing between the two
	// snapshots would pair values with another cycle's times. The write count catches it.
	acq::SampleBuffer tb, yb;
	bool consistent = false;
	for(int attempt = 0; attempt < 3 && !consistent; ++attempt) {
		const quint64 before = m_store->writeCount();
		tb = m_store->snapshot(m_tKey);
		yb = m_store->snapshot(m_yKey);
		consistent = m_store->writeCount() == before;
	}
	if(!consistent) {
		return; // next frame
	}

	// Oldest chunk first, so the history stays in time order.
	const int depth = static_cast<int>(std::min(tb.depth(), yb.depth()));
	for(int i = depth - 1; i >= 0; --i) {
		const auto *t = std::get_if<QVector<double>>(&tb.sample(i));
		if(!t || t->isEmpty() || t->last() <= m_last) {
			continue;
		}
		const QVector<float> y = acq::toFloat(yb.sample(i));
		const int n = std::min(t->size(), y.size());
		for(int s = 0; s < n; ++s) {
			if(t->at(s) > m_last) {
				m_x.append(t->at(s));
				m_y.append(y.at(s));
			}
		}
		m_last = std::max(m_last, t->last());
	}
}

void TimeCurve::trim(double minTime, int maxPoints)
{
	const int old = static_cast<int>(std::lower_bound(m_x.cbegin(), m_x.cend(), minTime) - m_x.cbegin());
	if(old > 0) {
		m_x.remove(0, old);
		m_y.remove(0, old);
	}
	// Over the point budget, a fast buffered channel is thinned rather than cut short, so the whole
	// history stays: the older half keeps the min and max of every 4 samples (peaks survive), the
	// newer half keeps full detail. Each pass removes a quarter of the points.
	while(m_x.size() > maxPoints && m_x.size() >= 8) {
		const int half = static_cast<int>(m_x.size() / 2) & ~3;
		int w = 0;
		for(int r = 0; r < half; r += 4) {
			int lo = r, hi = r;
			for(int k = r + 1; k < r + 4; ++k) {
				if(m_y[k] < m_y[lo]) {
					lo = k;
				}
				if(m_y[k] > m_y[hi]) {
					hi = k;
				}
			}
			// In time order, so X stays sorted.
			const int a = std::min(lo, hi), b = std::max(lo, hi);
			m_x[w] = m_x[a];
			m_y[w++] = m_y[a];
			if(b != a) {
				m_x[w] = m_x[b];
				m_y[w++] = m_y[b];
			}
		}
		m_x.remove(w, half - w);
		m_y.remove(w, half - w);
	}
}

void TimeCurve::clear()
{
	m_x.clear();
	m_y.clear();
	refresh();
}

void TimeCurve::refresh() { m_ch->curve()->setRawSamples(m_x.constData(), m_y.constData(), static_cast<int>(m_x.size())); }

bool TimeCurve::range(double x0, double x1, double &lo, double &hi) const
{
	if(!m_ch->isEnabled()) {
		return false;
	}
	auto first = std::lower_bound(m_x.cbegin(), m_x.cend(), x0);
	auto last = std::upper_bound(first, m_x.cend(), x1);
	if(first == last) {
		return false;
	}
	const int a = static_cast<int>(first - m_x.cbegin());
	const int b = static_cast<int>(last - m_x.cbegin());
	const auto [mn, mx] = std::minmax_element(m_y.cbegin() + a, m_y.cbegin() + b);
	lo = *mn;
	hi = *mx;
	return true;
}

TimePlot::TimePlot(const QString &name, quint32 uuid, acq::DataStore *store, QWidget *parent)
	: QObject(parent)
	, m_store(store)
	, m_uuid(uuid)
	, m_name(name)
{
	m_plot = new PlotWidget(parent);
	m_plot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_plot->xAxis()->setVisible(true);
	m_plot->yAxis()->setVisible(true);
	m_plot->setShowXAxisLabels(true);
	m_plot->setShowYAxisLabels(true);
	m_plot->showAxisLabels();
	m_plot->setAlignCanvasToScales(false);
	m_plot->yAxis()->setInterval(-1, 1);
	// Our window moves every frame; that must not reset a zoom the reader made.
	m_plot->navigator()->setResetOnNewBase(false);

	m_formatter = new TimeFormatter(this);
	m_plot->xAxis()->setFormatter(m_formatter);
	m_engine = new TimeScaleEngine();
	m_plot->xAxis()->setScaleEngine(m_engine);

	m_startTime = nowMs();
	applyLabels();
	applyXWindow();
	setShowLegend(m_showLegend);
}

TimePlot::~TimePlot()
{
	qDeleteAll(m_curves);
	m_curves.clear();
}

QString TimePlot::menuId() const { return QStringLiteral("multiacq:plot:%1").arg(m_uuid); }

void TimePlot::setName(const QString &name)
{
	if(m_name == name) {
		return;
	}
	m_name = name;
	Q_EMIT nameChanged(name);
}

TimeCurve *TimePlot::addCurve(const acq::DataKey &yKey, const acq::DataKey &tKey, const QString &name,
			      const QColor &color)
{
	TimeCurve *c = new TimeCurve(yKey, tKey, name, color, m_plot, m_store, this);
	m_curves.append(c);
	m_plot->replot();
	return c;
}

void TimePlot::removeCurve(TimeCurve *curve)
{
	if(m_curves.removeOne(curve)) {
		delete curve;
		m_plot->replot();
	}
}

void TimePlot::setRelative(bool relative)
{
	m_relative = relative;
	applyLabels();
	m_plot->replot();
}

void TimePlot::setStartTime(double ms)
{
	m_startTime = ms;
	applyLabels();
	m_plot->replot();
}

void TimePlot::setTimeFormat(const QString &format)
{
	m_format = format.isEmpty() ? QStringLiteral("hh:mm:ss") : format;
	applyLabels();
	m_plot->replot();
}

void TimePlot::setSpan(double seconds)
{
	m_span = std::max(0.001, seconds);
	m_history = std::max(m_history, m_span);
	applyLabels();
	applyXWindow();
	m_plot->replot();
}

void TimePlot::setHistory(double seconds) { m_history = std::max(seconds, m_span); }

void TimePlot::setLive(bool live)
{
	m_live = live;
	applyXWindow();
	m_plot->replot();
}

void TimePlot::setAutoscale(bool on)
{
	m_autoscale = on;
	applyAutoscale();
	m_plot->replot();
}

double TimePlot::yMin() const { return m_plot->yAxis()->min(); }

double TimePlot::yMax() const { return m_plot->yAxis()->max(); }

void TimePlot::setYRange(double min, double max)
{
	if(min >= max) {
		return;
	}
	m_plot->yAxis()->setInterval(min, max);
	m_plot->replot();
}

void TimePlot::setShowLegend(bool on)
{
	m_showLegend = on;
	if(on && m_legend.isNull()) {
		m_legend = new PlotLegend(m_plot, m_plot);
	}
	if(!m_legend.isNull()) {
		m_legend->setVisible(on);
	}
}

void TimePlot::update()
{
	const double minTime = nowMs() - m_history * 1000.0;
	for(TimeCurve *c : std::as_const(m_curves)) {
		c->pull();
		c->trim(minTime, kMaxPoints);
		c->refresh();
	}
	applyXWindow();
	applyAutoscale();
	m_plot->replot();
}

void TimePlot::clear()
{
	for(TimeCurve *c : std::as_const(m_curves)) {
		c->clear();
	}
	m_plot->replot();
}

void TimePlot::applyLabels()
{
	// Ticks closer than a second need the milliseconds to tell them apart.
	const bool ms = TimeScaleEngine::stepFor(m_span * 1000.0) < 1000.0 && !m_format.contains('z');
	m_formatter->setFormat(ms ? m_format + QStringLiteral(".zzz") : m_format);
	const double reference = m_relative ? m_startTime : 0.0;
	m_formatter->setReference(reference);
	m_engine->setReference(reference);
	m_plot->xAxis()->scaleDraw()->invalidateCache();
}

bool TimePlot::zoomed() const
{
	// Every axis change we make becomes the navigator's zoom base, so moving the axes while
	// the reader is zoomed would wipe the zoom and leave nothing to reset to.
	return m_plot->navigator()->isZoomed();
}

void TimePlot::applyXWindow()
{
	if(!m_live || zoomed()) {
		return;
	}
	const double now = nowMs();
	m_plot->xAxis()->setInterval(now - m_span * 1000.0, now);
}

void TimePlot::applyAutoscale()
{
	if(!m_autoscale || zoomed()) {
		return;
	}
	const double x0 = m_plot->xAxis()->visibleMin();
	const double x1 = m_plot->xAxis()->visibleMax();
	bool any = false;
	double lo = 0, hi = 0;
	for(TimeCurve *c : std::as_const(m_curves)) {
		double clo, chi;
		if(!c->range(std::min(x0, x1), std::max(x0, x1), clo, chi)) {
			continue;
		}
		lo = any ? std::min(lo, clo) : clo;
		hi = any ? std::max(hi, chi) : chi;
		any = true;
	}
	if(!any) {
		return;
	}
	// A flat line gets a band around it rather than a zero-height axis.
	const double pad = hi > lo ? (hi - lo) * 0.05 : std::max(1.0, std::abs(lo) * 0.05);
	if(!qFuzzyCompare(yMin(), lo - pad) || !qFuzzyCompare(yMax(), hi + pad)) {
		m_plot->yAxis()->setInterval(lo - pad, hi + pad);
		Q_EMIT yRangeChanged(lo - pad, hi + pad);
	}
}

#include "moc_timeplot.cpp"
