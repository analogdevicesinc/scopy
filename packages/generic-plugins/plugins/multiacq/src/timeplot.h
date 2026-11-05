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

#ifndef MULTIACQ_TIMEPLOT_H
#define MULTIACQ_TIMEPLOT_H

#include <core/acq_engine/datakey.h>

#include <QColor>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QVector>
#include <gui/osc_scale_engine.h>
#include <gui/plot_utils.hpp>

namespace scopy {
class PlotChannel;
class PlotLegend;
class PlotWidget;
namespace acq {
class DataStore;
}

namespace multiacq {

// Tick labels for an axis in ms since epoch: time of day, or time elapsed since a reference.
// A formatter rather than a scale draw, so the PlotAxis keeps its own scale draw.
class TimeFormatter : public PrefixFormatter
{
	Q_OBJECT
public:
	explicit TimeFormatter(QObject *parent = nullptr);

	// Zero: time of day. Otherwise labels count from here, ms since epoch.
	void setReference(double ms) { m_reference = ms; }
	void setFormat(const QString &format) { m_format = format; }

	QString format(double value, QString unitType, int precision) const override;
	static QString text(double value, double reference, const QString &format);

private:
	double m_reference{0.0};
	QString m_format;
};

// Ticks on round times (1 s, 5 s, 1 min ...) from the reference or the local wall clock, so
// they scroll with the data instead of standing still under changing labels.
class TimeScaleEngine : public OscScaleEngine
{
public:
	// Zero: align to the local wall clock. Otherwise to this, ms since epoch.
	void setReference(double ms) { m_reference = ms; }

	// The tick step for an axis `widthMs` wide.
	static double stepFor(double widthMs);

	QwtScaleDiv divideScale(double x1, double x2, int maxMajorSteps, int maxMinorSteps,
				double stepSize = 0.0) const override;

private:
	double m_reference{0.0};
};

// One channel on a time plot: its own history of (time, value) pairs, built up from the
// chunks the store retains. Each pull appends only samples newer than the last one seen,
// so nothing is drawn twice and a frame that falls behind the engine loses nothing.
class TimeCurve : public QObject
{
	Q_OBJECT
public:
	TimeCurve(const scopy::acq::DataKey &yKey, const scopy::acq::DataKey &tKey, const QString &name,
		  const QColor &color, PlotWidget *plot, scopy::acq::DataStore *store, QObject *parent = nullptr);
	~TimeCurve() override;

	scopy::acq::DataKey yKey() const { return m_yKey; }
	QString name() const;
	QColor color() const;
	PlotChannel *plotChannel() const { return m_ch; }
	// Hidden curves are not drawn and do not count for autoscale.
	void setVisible(bool visible);

	void pull();
	// Drop everything older than `minTime`; past `maxPoints`, thin the older samples instead of
	// dropping them, so the whole history stays drawn.
	void trim(double minTime, int maxPoints);
	void clear();
	// Hands the history to the curve; call after pull()/trim().
	void refresh();
	// Value range over [x0, x1]. False when nothing is in it.
	bool range(double x0, double x1, double &lo, double &hi) const;

private:
	const scopy::acq::DataKey m_yKey;
	const scopy::acq::DataKey m_tKey;
	const QString m_claimant;
	QPointer<scopy::acq::DataStore> m_store;
	QPointer<PlotWidget> m_plot;
	PlotChannel *m_ch{nullptr};
	QVector<double> m_x;
	QVector<double> m_y;
	// Newest timestamp taken so far. Kept across clear(), so samples still in the store
	// are not taken again.
	double m_last{0.0};
};

// A plot with a wall-clock X axis that scrolls over a chosen span, labelled as time of day or
// as time since the start. Channels from any source share the one axis.
class TimePlot : public QObject
{
	Q_OBJECT
public:
	TimePlot(const QString &name, quint32 uuid, scopy::acq::DataStore *store, QWidget *parent = nullptr);
	~TimePlot() override;

	PlotWidget *plot() const { return m_plot; }
	quint32 uuid() const { return m_uuid; }
	QString menuId() const;
	QString name() const { return m_name; }
	void setName(const QString &name);

	TimeCurve *addCurve(const scopy::acq::DataKey &yKey, const scopy::acq::DataKey &tKey, const QString &name,
			    const QColor &color);
	void removeCurve(TimeCurve *curve);
	QList<TimeCurve *> curves() const { return m_curves; }

	bool relative() const { return m_relative; }
	void setRelative(bool relative);
	// The reference relative labels count from, ms since epoch.
	void setStartTime(double ms);
	QString timeFormat() const { return m_format; }
	void setTimeFormat(const QString &format);
	// Seconds shown across the plot.
	double span() const { return m_span; }
	void setSpan(double seconds);
	// Seconds of data kept, at least the span.
	double history() const { return m_history; }
	void setHistory(double seconds);
	// Follow the current time. Off, the axis stays put and can be panned and zoomed.
	bool live() const { return m_live; }
	void setLive(bool live);

	bool autoscale() const { return m_autoscale; }
	void setAutoscale(bool on);
	double yMin() const;
	double yMax() const;
	void setYRange(double min, double max);

	bool showLegend() const { return m_showLegend; }
	void setShowLegend(bool on);

	// One frame: take new samples, drop old ones, move the axes, repaint.
	void update();
	void clear();

Q_SIGNALS:
	void nameChanged(QString name);
	void yRangeChanged(double min, double max);

private:
	// Pushes format and reference to the formatter and drops the cached labels.
	void applyLabels();
	// While the reader is zoomed in, the axes are theirs: no following, no autoscale.
	bool zoomed() const;
	void applyXWindow();
	void applyAutoscale();

	PlotWidget *m_plot{nullptr};
	TimeFormatter *m_formatter{nullptr};
	TimeScaleEngine *m_engine{nullptr}; // owned by the qwt plot
	QPointer<PlotLegend> m_legend;
	QPointer<scopy::acq::DataStore> m_store;
	const quint32 m_uuid;
	QString m_name;
	QList<TimeCurve *> m_curves;

	bool m_relative{true};
	double m_startTime{0.0};
	QString m_format{QStringLiteral("hh:mm:ss")};
	double m_span{60.0};
	double m_history{600.0};
	bool m_live{true};
	bool m_autoscale{true};
	bool m_showLegend{true};
};

} // namespace multiacq
} // namespace scopy

#endif // MULTIACQ_TIMEPLOT_H
