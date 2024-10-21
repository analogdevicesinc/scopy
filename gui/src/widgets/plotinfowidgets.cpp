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

#include "plotinfowidgets.h"
#include "plotaxis.h"
#include "plotnavigator.hpp"
#include "plotwidget.h"
#include <QDateTime>
#include <style.h>
#include <stylehelper.h>
#include <pluginbase/preferences.h>

using namespace scopy;

HDivInfo::HDivInfo(PlotWidget *plot, QWidget *parent)
	: m_mpf(new MetricPrefixFormatter(this))
	, m_plot(plot)
{
	Style::setStyle(this, style::properties::label::plotInfo, true, true);
	m_mpf->setTrimZeroes(true);
	m_mpf->setTwoDecimalMode(false);
	connect(m_plot->navigator(), &PlotNavigator::rectChanged, this, &HDivInfo::onRectChanged);

	onRectChanged();
}

HDivInfo::~HDivInfo() {}

void HDivInfo::onRectChanged()
{
	PlotChannel *ch = m_plot->selectedChannel();
	if(!ch)
		return;

	PlotAxis *xAxis = ch->xAxis();
	double currMin, currMax, axisMax, axisMin, divs;
	bool zoomed;

	axisMax = xAxis->max();
	axisMin = xAxis->min();
	currMax = xAxis->visibleMax();
	currMin = xAxis->visibleMin();
	zoomed = axisMax != currMax || axisMin != currMin;
	divs = xAxis->divs();
	update(abs(currMax - currMin) / divs, zoomed);
}

void HDivInfo::update(double val, bool zoomed)
{
	QString units = m_plot->selectedChannel()->xAxis()->getUnits();
	if(units.isEmpty()) {
		setText(QString::number(val) + "/div" + (zoomed ? " (zoomed)" : ""));
	} else {
		setText(m_mpf->format(val, units, 2) + "/div" + (zoomed ? " (zoomed)" : ""));
	}
}

TimeSamplingInfo::TimeSamplingInfo(QWidget *parent)
	: m_mpf(new MetricPrefixFormatter(this))
{
	Style::setStyle(this, style::properties::label::plotInfo, true, true);
	m_mpf->setTrimZeroes(true);
	m_mpf->setTwoDecimalMode(false);
}

TimeSamplingInfo::~TimeSamplingInfo() {}

void TimeSamplingInfo::update(SamplingInfo info)
{
	QString text;
	text = QString("%1 samples").arg(QString::number(info.plotSize));
	//.arg(m_mpf->format(binfo.bufferSizes, "samples", 2));
	//	if(info.sampleRate != 1.0)
	if(info.sampleRate != 1) {
		text += QString(" at %2").arg(m_mpf->format(info.sampleRate, "sps", 2));
	}

	setText(text);
}

FFTSamplingInfo::FFTSamplingInfo(QWidget *parent)
	: m_mpf(new MetricPrefixFormatter(this))
{
	Style::setStyle(this, style::properties::label::plotInfo, true, true);
	m_mpf->setTrimZeroes(true);
	m_mpf->setTwoDecimalMode(false);
}

FFTSamplingInfo::~FFTSamplingInfo() {}

void FFTSamplingInfo::update(SamplingInfo info)
{
	QString text;
	text = QString("%1").arg(m_mpf->format(info.plotSize, "samples", 3));
	if(info.sampleRate != 1) {
		text += QString(" at %2").arg(m_mpf->format(info.sampleRate, "sps", 3));
	}
	if(info.freqOffset != 0) {
		text += QString("\nCenter Frequency: %1").arg(m_mpf->format(info.freqOffset, "Hz", 3));
	}

	setText(text);
}

FPSInfo::FPSInfo(PlotWidget *plot, QWidget *parent)
	: QLabel(parent)
	, m_plot(plot)
	, m_replotTimes(new QList<qint64>())
	, m_lastTimeStamp(0)
	, m_avgSize(10)
{
	Style::setStyle(this, style::properties::label::plotInfo, true, true);
	setVisible(Preferences::GetInstance()->get("general_show_plot_fps").toBool());

	connect(m_plot, &PlotWidget::newData, this, [=]() { update(QDateTime::currentMSecsSinceEpoch()); });
	connect(Preferences::GetInstance(), &Preferences::preferenceChanged, this,
		[=](QString preference, QVariant value) {
			if(preference == "general_show_plot_fps") {
				setVisible(value.toBool());
			}
		});
}

FPSInfo::~FPSInfo() {}

void FPSInfo::update(qint64 timestamp)
{
	if(m_lastTimeStamp == 0) {
		m_lastTimeStamp = timestamp;
		return;
	}

	m_replotTimes->append(timestamp - m_lastTimeStamp);
	if(m_replotTimes->size() > m_avgSize) {
		m_replotTimes->removeAt(0);
	}

	qint64 avg = 0;
	for(qint64 time : *m_replotTimes) {
		avg += time;
	}
	avg /= m_replotTimes->size();
	m_lastTimeStamp = timestamp;

	setText(QString(QString::number(1000. / avg, 'g', 3) + " FPS"));
}

TimestampInfo::TimestampInfo(PlotWidget *plot, QWidget *parent)
{
	Style::setStyle(this, style::properties::label::plotInfo, true, true);
	connect(plot, &PlotWidget::newData, this,
		[=]() { setText(QDateTime::currentDateTime().time().toString("hh:mm:ss")); });
}

TimestampInfo::~TimestampInfo() {}

#include "moc_plotinfowidgets.cpp"
