#include "plottracker.hpp"
#include "plotaxis.h"

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
	for(ChannelTracker *chTracker : *m_trackers) {
		if(chTracker->channel == ch) {
			m_trackers->remove(chTracker);
			delete chTracker;
		}
	}
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
	tracker->setXFormatter(ch->xAxis()->getFromatter());
	tracker->setYFormatter(ch->yAxis()->getFromatter());

	connect(ch->xAxis(), &PlotAxis::formatterChanged, tracker, &BasicTracker::setXFormatter);
	connect(ch->yAxis(), &PlotAxis::formatterChanged, tracker, &BasicTracker::setYFormatter);
	connect(ch->xAxis(), &PlotAxis::unitsChanged, tracker, &BasicTracker::setXAxisUnit);
	connect(ch->yAxis(), &PlotAxis::unitsChanged, tracker, &BasicTracker::setYAxisUnit);

	return chTracker;
}

void PlotTracker::onChannelSelected(PlotChannel *ch)
{
	if(m_en) {
		for(ChannelTracker *chTracker : *m_trackers) {
			chTracker->tracker->setEnabled(chTracker->channel == ch);
		}
	}
}

#include "moc_plottracker.cpp"
