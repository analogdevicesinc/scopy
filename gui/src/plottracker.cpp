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

#include "plottracker.hpp"
#include "plotaxis.h"
#include <QSet>

using namespace scopy;

ChannelTracker::ChannelTracker() {}

ChannelTracker::~ChannelTracker() { tracker->deleteLater(); }

PlotTracker::PlotTracker(PlotWidget *plot, QList<PlotChannel *> *channels)
	: QObject(plot)
	, m_plot(plot)
	, m_trackers(new QSet<ChannelTracker *>())
	, m_en(true)
{
	init(channels);
}

PlotTracker::~PlotTracker() { delete m_trackers; }

void PlotTracker::init(QList<PlotChannel *> *channels)
{
	for(PlotChannel *ch : *channels) {
		m_trackers->insert(createTracker(ch));
	}

	connect(m_plot, &PlotWidget::channelSelected, this, &PlotTracker::onChannelSelected);
}

void PlotTracker::setEnabled(bool en)
{
	m_en = en;
	for(ChannelTracker *chTracker : *m_trackers) {
		chTracker->tracker->setEnabled(m_en);
	}
}

bool PlotTracker::isEnabled() { return m_en; }

void PlotTracker::addChannel(PlotChannel *ch) { m_trackers->insert(createTracker(ch)); }

void PlotTracker::removeChannel(PlotChannel *ch)
{
	ChannelTracker *toRemove;
	for(ChannelTracker *chTracker : qAsConst(*m_trackers)) {
		if(chTracker->channel == ch) {
			toRemove = chTracker;
			break;
		}
	}
	m_trackers->remove(toRemove);
	delete toRemove;
}

ChannelTracker *PlotTracker::createTracker(PlotChannel *ch)
{
	ChannelTracker *chTracker = new ChannelTracker();
	m_trackers->insert(chTracker);
	chTracker->channel = ch;

	BasicTracker *tracker = new BasicTracker(m_plot->plot());
	chTracker->tracker = tracker;
	tracker->setXAxis(ch->xAxis()->axisId());
	tracker->setYAxis(ch->yAxis()->axisId());
	tracker->setColor(ch->curve()->pen().color());
	tracker->setEnabled(m_en && m_plot->selectedChannel() == ch);
	tracker->setXAxisUnit(ch->xAxis()->getUnits());
	tracker->setYAxisUnit(ch->yAxis()->getUnits());
	tracker->setXFormatter(ch->xAxis()->getFormatter());
	tracker->setYFormatter(ch->yAxis()->getFormatter());

	connect(ch->xAxis(), &PlotAxis::formatterChanged, tracker, &BasicTracker::setXFormatter);
	connect(ch->yAxis(), &PlotAxis::formatterChanged, tracker, &BasicTracker::setYFormatter);
	connect(ch->xAxis(), &PlotAxis::unitsChanged, tracker, &BasicTracker::setXAxisUnit);
	connect(ch->yAxis(), &PlotAxis::unitsChanged, tracker, &BasicTracker::setYAxisUnit);

	return chTracker;
}

void PlotTracker::onChannelSelected(PlotChannel *ch)
{
	if(m_en) {
		for(ChannelTracker *chTracker : qAsConst(*m_trackers)) {
			chTracker->tracker->setEnabled(chTracker->channel == ch);
		}
	}
}

#include "moc_plottracker.cpp"
