#include "time_yautoscale.h"

#include <QHBoxLayout>
#include <QLoggingCategory>
#include <QTimer>

Q_LOGGING_CATEGORY(CAT_TIMEYAUTOSCALE, "Time Y-Autoscale");

using namespace scopy::grutil;

TimeYAutoscale::TimeYAutoscale(QObject *parent)
	: QObject(parent)
{
	// AUTOSCALE
	m_autoScaleTimer = new QTimer(this);
	m_autoScaleTimer->setInterval(1000);
	connect(m_autoScaleTimer, &QTimer::timeout, this, &TimeYAutoscale::autoscale);
}

TimeYAutoscale::~TimeYAutoscale() {}

void TimeYAutoscale::start() { m_autoScaleTimer->start(); }

void TimeYAutoscale::stop() { m_autoScaleTimer->stop(); }

void TimeYAutoscale::autoscale()
{
	double max = -1000000.0;
	double min = 1000000.0;

	for(PlotChannel *plotCh : m_channels) {
		auto data = plotCh->curve()->data();
		for(int i = 0; i < data->size(); i++) {
			auto sample = data->sample(i).y();
			if(max < sample)
				max = sample;
			if(min > sample)
				min = sample;
		}
		qInfo(CAT_TIMEYAUTOSCALE)
			<< "Autoscaling channel " << plotCh->name() << "to (" << min << ", " << max << ")";
	}
	Q_EMIT newMin(min);
	Q_EMIT newMax(max);
}

void TimeYAutoscale::addChannels(PlotChannel *c) { m_channels.append(c); }

void TimeYAutoscale::removeChannels(PlotChannel *c) { m_channels.removeAll(c); }
