#include "plotinfo.h"

#include "hoverwidget.h"
#include "plotaxis.h"

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

TimePlotStatusInfo::TimePlotStatusInfo(QWidget *parent)
{
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	StyleHelper::TimePlotSamplingInfo(this);
}

TimePlotStatusInfo::~TimePlotStatusInfo() {}

TimePlotInfo::TimePlotInfo(PlotWidget *plot, QWidget *parent)
{
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
	m_hdiv->setAttribute(Qt::WA_TransparentForMouseEvents);

	HoverWidget *samplinginfohover = new HoverWidget(nullptr, plot->plot()->canvas(), plot->plot());
	samplinginfohover->setContent(m_sampling);
	samplinginfohover->setAnchorPos(HoverPosition::HP_TOPRIGHT);
	samplinginfohover->setContentPos(HoverPosition::HP_BOTTOMLEFT);
	samplinginfohover->setAnchorOffset(QPoint(-8, 6));
	samplinginfohover->show();
	samplinginfohover->setAttribute(Qt::WA_TransparentForMouseEvents);
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
	currMax = m_plot->plot()->axisScaleDiv(xAxis->axisId()).upperBound();
	currMin = m_plot->plot()->axisScaleDiv(xAxis->axisId()).lowerBound();
	zoomed = axisMax != currMax || axisMin != currMin;
	divs = xAxis->divs();

	m_hdiv->update(abs(currMax - currMin) / divs, zoomed);
	m_sampling->update(info.plotSize, info.bufferSize, info.sampleRate);
}

#include "moc_plotinfo.cpp"
