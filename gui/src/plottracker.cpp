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

void PlotTracker::setYAxisUnit(QString unit)
{
	m_yAxisUnit = unit;

	for(ChannelTracker *chTracker : *m_trackers) {
		chTracker->tracker->setYAxisUnit(m_yAxisUnit);
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
	tracker->setEnabled(m_plot->selectedChannel() == ch);

	// PLOT AXIS SHOULD CONTAIN FORMATTER AND UNITS
	// PLOTAXIS USE OSCSCALEDRAW WHICH IS IN DISPLAYPLOT
	MetricPrefixFormatter *formatter = new MetricPrefixFormatter();
	formatter->setTrimZeroes(true);
	formatter->setTwoDecimalMode(true);
	tracker->setXFormatter(formatter);
	tracker->setXAxisUnit("s");
	tracker->setYAxisUnit(m_yAxisUnit);

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
