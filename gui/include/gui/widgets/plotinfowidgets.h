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

#ifndef PLOTINFOWIDGETS_H
#define PLOTINFOWIDGETS_H

#include <QLabel>
#include <QWidget>
#include <plot_utils.hpp>
#include <plotwidget.h>
#include <scopy-gui_export.h>

namespace scopy {

class SCOPY_GUI_EXPORT HDivInfo : public QLabel
{
	Q_OBJECT
public:
	HDivInfo(PlotWidget *plot, QWidget *parent = nullptr);
	virtual ~HDivInfo();

public Q_SLOTS:
	void update(double val, bool zoomed = false);
	void onRectChanged();

private:
	MetricPrefixFormatter *m_mpf;
	PlotWidget *m_plot;
};

class SCOPY_GUI_EXPORT TimeSamplingInfo : public QLabel
{
	Q_OBJECT
public:
	TimeSamplingInfo(QWidget *parent = nullptr);
	virtual ~TimeSamplingInfo();

public Q_SLOTS:
	void update(SamplingInfo info);

private:
	MetricPrefixFormatter *m_mpf;
};

class SCOPY_GUI_EXPORT FFTSamplingInfo : public QLabel
{
	Q_OBJECT
public:
	FFTSamplingInfo(QWidget *parent = nullptr);
	virtual ~FFTSamplingInfo();

public Q_SLOTS:
	void update(SamplingInfo info);

private:
	MetricPrefixFormatter *m_mpf;
};

class SCOPY_GUI_EXPORT FPSInfo : public QLabel
{
	Q_OBJECT
public:
	FPSInfo(PlotWidget *plot, QWidget *parent = nullptr);
	virtual ~FPSInfo();

public Q_SLOTS:
	void update(qint64 timestamp);

private:
	PlotWidget *m_plot;
	QList<qint64> *m_replotTimes;
	qint64 m_lastTimeStamp;
	int m_avgSize;
};

class SCOPY_GUI_EXPORT TimestampInfo : public QLabel
{
	Q_OBJECT
public:
	TimestampInfo(PlotWidget *plot, QWidget *parent = nullptr);
	virtual ~TimestampInfo();

private:
	PlotWidget *m_plot;
};

} // namespace scopy

#endif // PLOTINFOWIDGETS_H
