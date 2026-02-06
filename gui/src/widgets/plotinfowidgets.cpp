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
#include "hoverwidget.h"
#include "plotaxis.h"
#include "plotchannel.h"
#include "plotnavigator.hpp"
#include "plotwidget.h"
#include <QDateTime>
#include <QHBoxLayout>
#include <style.h>
#include <stylehelper.h>
#include <pluginbase/preferences.h>

using namespace scopy;

HDivInfo::HDivInfo(PlotWidget *plot, QWidget *parent)
	: m_mpf(new MetricPrefixFormatter(this))
	, m_plot(plot)
{
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
	, m_showSPS(true)
{
	m_mpf->setTrimZeroes(true);
	m_mpf->setTwoDecimalMode(false);
}

TimeSamplingInfo::~TimeSamplingInfo() {}

void TimeSamplingInfo::setShowSPS(bool show) { m_showSPS = show; }

void TimeSamplingInfo::update(SamplingInfo info)
{
	QString text;
	text = QString("%1 samples").arg(QString::number(info.plotSize));

	if(m_showSPS) {
		text += QString(" at %2").arg(m_mpf->format(info.sampleRate, "sps", 2));
	}

	setText(text);
}

FFTSamplingInfo::FFTSamplingInfo(QWidget *parent)
	: m_mpf(new MetricPrefixFormatter(this))
{
	m_mpf->setTrimZeroes(true);
	m_mpf->setTwoDecimalMode(false);
}

FFTSamplingInfo::~FFTSamplingInfo() {}

void FFTSamplingInfo::setShowSPS(bool show) { m_showSPS = show; }

void FFTSamplingInfo::update(SamplingInfo info)
{
	QString text;
	text = QString("%1").arg(m_mpf->format(info.plotSize, "samples", 3));
	if(m_showSPS) {
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
	connect(plot, &PlotWidget::newData, this,
		[=]() { setText(QDateTime::currentDateTime().time().toString("hh:mm:ss")); });
}

TimestampInfo::~TimestampInfo() {}

PlotLegendWidget::PlotLegendWidget(PlotWidget *plot, QWidget *parent)
	: QPushButton("Legend", parent)
	, m_plot(plot)
{
	setCheckable(true);
	setChecked(false);
	setContentsMargins(0, 0, 0, 0);
	setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

	m_legendPanel = new QWidget(m_plot);
	m_legendLayout = new QVBoxLayout(m_legendPanel);
	m_legendLayout->setContentsMargins(6, 6, 6, 6);
	m_legendLayout->setSpacing(4);

	m_hoverWidget = new HoverWidget(m_legendPanel, this, m_plot);
	m_hoverWidget->setAnchorPos(HoverPosition::HP_TOPRIGHT);
	m_hoverWidget->setContentPos(HoverPosition::HP_TOPLEFT);
	m_hoverWidget->setDraggable(true);
	m_hoverWidget->setRelative(true);
	m_hoverWidget->setVisible(false);

	connect(this, &QPushButton::toggled, this, &PlotLegendWidget::onToggled);
	connect(m_plot, &PlotWidget::addedChannel, this, &PlotLegendWidget::addChannel);
	connect(m_plot, &PlotWidget::removedChannel, this, &PlotLegendWidget::removeChannel);

	for(PlotChannel *ch : m_plot->getChannels()) {
		addChannel(ch);
	}
}

PlotLegendWidget::~PlotLegendWidget() {}

void PlotLegendWidget::addChannel(PlotChannel *ch)
{
	if(m_entries.contains(ch)) {
		return;
	}

	QWidget *entry = new QWidget(m_legendPanel);
	Style::setBackgroundColor(entry, QString("transparent"), true);
	QHBoxLayout *entryLayout = new QHBoxLayout(entry);
	entryLayout->setContentsMargins(0, 0, 0, 0);
	entryLayout->setSpacing(6);

	QWidget *colorLine = new QWidget(entry);
	colorLine->setFixedSize(20, 3);

	QLabel *nameLabel = new QLabel(ch->name(), entry);

	entryLayout->addWidget(colorLine);
	entryLayout->addWidget(nameLabel);
	entryLayout->addStretch();

	m_legendLayout->addWidget(entry);
	m_entries.insert(ch, entry);
	m_labels.insert(ch, nameLabel);
	m_colorLines.insert(ch, colorLine);

	updateLineStyle(ch);

	connect(ch, &PlotChannel::enabledChanged, this, &PlotLegendWidget::updateStyles);
	connect(ch, &PlotChannel::thicknessChanged, this, [this, ch]() { updateLineStyle(ch); });
	connect(ch, &PlotChannel::styleChanged, this, [this, ch]() { updateLineStyle(ch); });
}

void PlotLegendWidget::removeChannel(PlotChannel *ch)
{
	if(!m_entries.contains(ch)) {
		return;
	}

	disconnect(ch, &PlotChannel::enabledChanged, this, &PlotLegendWidget::updateStyles);
	disconnect(ch, &PlotChannel::thicknessChanged, this, nullptr);
	disconnect(ch, &PlotChannel::styleChanged, this, nullptr);

	QWidget *entry = m_entries.take(ch);
	m_labels.remove(ch);
	m_colorLines.remove(ch);
	m_legendLayout->removeWidget(entry);
	entry->deleteLater();
}

void PlotLegendWidget::updateStyles()
{
	for(auto it = m_entries.begin(); it != m_entries.end(); ++it) {
		it.value()->setVisible(it.key()->isEnabled());
	}
}

void PlotLegendWidget::updateLineStyle(PlotChannel *ch)
{
	if(!m_colorLines.contains(ch)) {
		return;
	}

	QWidget *colorLine = m_colorLines.value(ch);
	QColor penColor = ch->curve()->pen().color();
	QString color = QString("rgba(%1, %2, %3, %4)")
				.arg(penColor.red())
				.arg(penColor.green())
				.arg(penColor.blue())
				.arg(penColor.alpha());
	int thickness = ch->thickness();
	int style = ch->style();

	QString borderStyle;
	switch(style) {
	case PlotChannel::PCS_DOTS:
		borderStyle = QString("background-color: transparent; border: %1px dotted %2;").arg(thickness).arg(color);
		break;
	case PlotChannel::PCS_STICKS:
	case PlotChannel::PCS_STEPS:
		borderStyle = QString("background-color: transparent; border: %1px dashed %2;").arg(thickness).arg(color);
		break;
	case PlotChannel::PCS_LINES:
	case PlotChannel::PCS_SMOOTH:
	default:
		borderStyle = QString("background-color: %1;").arg(color);
		break;
	}

	colorLine->setFixedHeight(thickness);
	colorLine->setStyleSheet(borderStyle);
}

void PlotLegendWidget::onToggled(bool checked)
{
	if(checked) {
		updateStyles();
	}
	m_hoverWidget->setVisible(checked);
}

#include "moc_plotinfowidgets.cpp"
