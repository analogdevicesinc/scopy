#include "plotinfo.h"

#include "hoverwidget.h"
#include "plotaxis.h"
#include "qdatetime.h"
#include <pluginbase/preferences.h>

using namespace scopy;

TimePlotHDivInfo::TimePlotHDivInfo(QWidget *parent)
{
	StyleHelper::TimePlotHDivInfo(this);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	m_mpf = new MetricPrefixFormatter(this);
	m_mpf->setTrimZeroes(true);
}
TimePlotHDivInfo::~TimePlotHDivInfo() {}

void TimePlotHDivInfo::update(double val, bool zoomed)
{
	setText(m_mpf->format(val, "s", 2) + "/div" + (zoomed ? " (zoomed)" : ""));
}

TimePlotSamplingInfo::TimePlotSamplingInfo(QWidget *parent)
{
	StyleHelper::TimePlotSamplingInfo(this);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	m_mpf = new MetricPrefixFormatter(this);
	m_mpf->setTrimZeroes(true);
}

TimePlotSamplingInfo::~TimePlotSamplingInfo() {}

void TimePlotSamplingInfo::update(int ps, int bs, double sr)
{
	QString text;
	text = QString("%1").arg(m_mpf->format(ps, "samples", 2)); //.arg(m_mpf->format(bs, "samples", 2));
								   //	if(sr != 1.0)
	text += QString(" at %2").arg(m_mpf->format(sr, "sps", 2));

	setText(text);
}

TimePlotFPS::TimePlotFPS(QWidget *parent)
{
	StyleHelper::TimePlotSamplingInfo(this);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	m_replotTimes = new QList<qint64>();
	m_lastTimeStamp = 0;
	m_avgSize = 10;
}

TimePlotFPS::~TimePlotFPS() {}

void TimePlotFPS::update(qint64 timestamp)
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
	for(qint64 time: *m_replotTimes) {
		avg += time;
	}
	avg /= m_replotTimes->size();
	m_lastTimeStamp = timestamp;

	setText(QString(QString::number(1000. / avg, 'g', 3) + " FPS"));
}
TimePlotStatusInfo::TimePlotStatusInfo(QWidget *parent)
{
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	StyleHelper::TimePlotSamplingInfo(this);
}

TimePlotStatusInfo::~TimePlotStatusInfo() {}

TimePlotInfo::TimePlotInfo(PlotWidget *plot, QWidget *parent)
{
	Preferences *p = Preferences::GetInstance();

	m_plot = plot;
	QVBoxLayout *vlay = new QVBoxLayout(this);
	vlay->setMargin(0);
	vlay->setSpacing(2);
	setLayout(vlay);
	QHBoxLayout *lay = new QHBoxLayout();
	lay->setMargin(0);
	lay->setSpacing(0);

	m_hdiv = new TimePlotHDivInfo(this);
	m_sampling = new TimePlotSamplingInfo(this);
	m_status = new TimePlotStatusInfo(this);

	m_fps = new TimePlotFPS(this);
	connect(plot, &PlotWidget::reploted, this, [=](){
		m_fps->update(QDateTime::currentMSecsSinceEpoch());
	});

	vlay->addLayout(lay);

#ifdef HOVER_INFO
	lay->addWidget(m_hdiv);
	lay->addWidget(m_sampling);
	lay->setAlignment(m_sampling, Qt::AlignRight);
#else

	HoverWidget *hdivhover = new HoverWidget(nullptr, plot->plot()->canvas(), plot->plot());
	hdivhover->setContent(m_hdiv);
	hdivhover->setAnchorPos(HoverPosition::HP_TOPLEFT);
	hdivhover->setContentPos(HoverPosition::HP_BOTTOMRIGHT);
	hdivhover->setAnchorOffset(QPoint(8, 6));
	hdivhover->show();
	hdivhover->setAttribute(Qt::WA_TransparentForMouseEvents);

	HoverWidget *samplinginfohover = new HoverWidget(nullptr, plot->plot()->canvas(), plot->plot());
	samplinginfohover->setContent(m_sampling);
	samplinginfohover->setAnchorPos(HoverPosition::HP_TOPRIGHT);
	samplinginfohover->setContentPos(HoverPosition::HP_BOTTOMLEFT);
	samplinginfohover->setAnchorOffset(QPoint(-8, 6));
	samplinginfohover->show();
	samplinginfohover->setAttribute(Qt::WA_TransparentForMouseEvents);

	HoverWidget *fpsHover = new HoverWidget(nullptr, plot->plot()->canvas(), plot->plot());
	fpsHover->setContent(m_fps);
	fpsHover->setAnchorPos(HoverPosition::HP_TOPLEFT);
	fpsHover->setContentPos(HoverPosition::HP_BOTTOMRIGHT);
	fpsHover->setAnchorOffset(QPoint(8, 26));
	fpsHover->setAttribute(Qt::WA_TransparentForMouseEvents);
	bool showFps = p->get("general_show_plot_fps").toBool();
	fpsHover->setVisible(showFps);
	connect(p, &Preferences::preferenceChanged, this, [=](QString name, QVariant type){
		if(name == "general_show_plot_fps") {
			fpsHover->setVisible(p->get("general_show_plot_fps").toBool());
		}
	});
#endif
}

TimePlotInfo::~TimePlotInfo() {}

void TimePlotInfo::update(PlotSamplingInfo info)
{
	PlotAxis *xAxis = m_plot->xAxis();
	double currMin, currMax, axisMax, axisMin, divs;
	bool zoomed;

	axisMax = xAxis->max();
	axisMin = xAxis->min();
	currMax = xAxis->visibleMax();
	currMin = xAxis->visibleMin();
	zoomed = axisMax != currMax || axisMin != currMin;
	divs = xAxis->divs();

	m_hdiv->update(abs(currMax - currMin) / divs, zoomed);
	m_sampling->update(info.plotSize, info.bufferSize, info.sampleRate);
}

#include "moc_plotinfo.cpp"
