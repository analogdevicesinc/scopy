#include "plotautoscaler.h"

#include <QHBoxLayout>
#include <QLoggingCategory>
#include <QTimer>

Q_LOGGING_CATEGORY(CAT_TIMEYAUTOSCALE, "Time Y-Autoscale");

using namespace scopy::gui;

PlotAutoscaler::PlotAutoscaler(bool xAxis, QObject *parent)
	: QObject(parent)
	, m_xAxis(xAxis)
{
	// AUTOSCALE
	m_autoScaleTimer = new QTimer(this);
	m_autoScaleTimer->setInterval(1000);
	connect(m_autoScaleTimer, &QTimer::timeout, this, &PlotAutoscaler::autoscale);
}

PlotAutoscaler::~PlotAutoscaler() {}

void PlotAutoscaler::start() { m_autoScaleTimer->start(); }

void PlotAutoscaler::stop() { m_autoScaleTimer->stop(); }

void PlotAutoscaler::autoscale()
{
	double max = -1000000.0;
	double min = 1000000.0;

	for(PlotChannel *plotCh : m_channels) {
		auto data = plotCh->curve()->data();
		for(int i = 0; i < data->size(); i++) {

			qreal sample;
			if(m_xAxis) {
				sample = data->sample(i).x();
			} else {
				sample = data->sample(i).y();
			}
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

void PlotAutoscaler::addChannels(PlotChannel *c) { m_channels.append(c); }

void PlotAutoscaler::removeChannels(PlotChannel *c) { m_channels.removeAll(c); }

#include "moc_plotautoscaler.cpp"
