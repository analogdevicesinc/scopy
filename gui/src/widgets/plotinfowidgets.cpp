#include "plotinfowidgets.h"
#include "plotaxis.h"
#include "plotnavigator.hpp"
#include "plotwidget.h"
#include <QDateTime>
#include <stylehelper.h>
#include <pluginbase/preferences.h>

using namespace scopy;

HDivInfo::HDivInfo(PlotWidget *plot, QWidget *parent)
	: m_mpf(new MetricPrefixFormatter(this))
	, m_plot(plot)
{
	StyleHelper::PlotInfoLabel(this);
	m_mpf->setTrimZeroes(true);
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
	StyleHelper::PlotInfoLabel(this);
	m_mpf->setTrimZeroes(true);
}

TimeSamplingInfo::~TimeSamplingInfo() {}

void TimeSamplingInfo::update(SamplingInfo info)
{
	QString text;
	text = QString("%1").arg(m_mpf->format(info.plotSize, "samples", 2));
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
	StyleHelper::PlotInfoLabel(this);
	m_mpf->setTrimZeroes(true);
}

FFTSamplingInfo::~FFTSamplingInfo() {}

void FFTSamplingInfo::update(SamplingInfo info)
{
	QString text;
	text = QString("%1").arg(m_mpf->format(info.plotSize, "samples", 2));
	//.arg(m_mpf->format(binfo.bufferSizes, "samples", 2));
	//	if(info.sampleRate != 1.0)
	if(info.sampleRate != 1) {
		text += QString(" at %2").arg(m_mpf->format(info.sampleRate, "sps", 2));
	}
	if(info.freqOffset != 0) {
		text += QString("\nCenter Frequency: %1").arg(m_mpf->format(info.freqOffset,"Hz",3));
	}


	setText(text);
}

FPSInfo::FPSInfo(PlotWidget *plot, QWidget *parent)
	: m_plot(plot)
	, m_replotTimes(new QList<qint64>())
	, m_lastTimeStamp(0)
	, m_avgSize(10)
{
	StyleHelper::PlotInfoLabel(this);
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
	StyleHelper::PlotInfoLabel(this);
	connect(plot, &PlotWidget::newData, this,
		[=]() { setText(QDateTime::currentDateTime().time().toString("hh:mm:ss.zzz")); });
}

TimestampInfo::~TimestampInfo() {}

#include "moc_plotinfowidgets.cpp"
