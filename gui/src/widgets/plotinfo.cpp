#include "plotinfo.h"
#include "hoverwidget.h"
#include "plotaxis.h"

using namespace scopy;

TimePlotHDivInfo::TimePlotHDivInfo(QWidget *parent) {
	StyleHelper::TimePlotHDivInfo(this);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	m_mpf = new MetricPrefixFormatter(this);

}

TimePlotHDivInfo::~TimePlotHDivInfo()
{

}

void TimePlotHDivInfo::update(double val) {
	setText(m_mpf->format(val, "s", 2) + "/div");
}

TimePlotSamplingInfo::TimePlotSamplingInfo(QWidget *parent) {
	StyleHelper::TimePlotSamplingInfo(this);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	m_mpf = new MetricPrefixFormatter(this);
}

TimePlotSamplingInfo::~TimePlotSamplingInfo()
{

}

void TimePlotSamplingInfo::update(int ps, int bs, double sr) {
	QString text;
	text = QString("%1").arg(m_mpf->format(ps, "samples", 2));//.arg(m_mpf->format(bs, "samples", 2));
//	if(sr != 1.0)
		text += QString(" at %2").arg(m_mpf->format(sr, "sps", 2));

	setText(text);
}

TimePlotStatusInfo::TimePlotStatusInfo(QWidget *parent)
{
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	StyleHelper::TimePlotSamplingInfo(this);
}

TimePlotStatusInfo::~TimePlotStatusInfo()
{

}

void PlotBufferPreviewerController::setupBufferPreviewer() {
	m_bufferPreviewer->setMinimumHeight(20);
	m_bufferPreviewer->setCursorPos(0.5);
	m_bufferPreviewer->setHighlightPos(0.05);
	m_bufferPreviewer->setHighlightWidth(0.2);
	m_bufferPreviewer->setCursorVisible(false);
	m_bufferPreviewer->setWaveformPos(0.1);
	m_bufferPreviewer->setWaveformWidth(0.5);

	updateDataLimits(m_plot->xAxis()->min(), m_plot->xAxis()->max());

	connect(m_bufferPreviewer, &BufferPreviewer::bufferStopDrag, this, [=]() {
	});

	connect(m_bufferPreviewer, &BufferPreviewer::bufferStartDrag, this, [=]() {
		m_bufferPrevInitMin = m_plot->xAxis()->min();
		m_bufferPrevInitMax = m_plot->xAxis()->max();
	});

	connect(m_bufferPreviewer, &BufferPreviewer::bufferMovedBy, this, [=](int value) {
		double moveTo = 0.0;

		int width = m_bufferPreviewer->width();
		double xAxisWidth = m_bufferPrevData;

		moveTo = value * xAxisWidth / width;
		m_plot->xAxis()->setInterval(m_bufferPrevInitMin + moveTo,  m_bufferPrevInitMax + moveTo);
		m_plot->replot();

		updateBufferPreviewer();
	} );

	connect(m_bufferPreviewer, &BufferPreviewer::bufferResetPosition, this, [=]() {
		m_plot->xAxis()->setInterval(m_bufferPrevInitMin,  m_bufferPrevInitMax);
		m_plot->replot();

		updateBufferPreviewer();
	} );
}

void PlotBufferPreviewerController::updateDataLimits(double min, double max) {
	m_bufferPrevData = max - min;
	updateBufferPreviewer();
}

void PlotBufferPreviewerController::updateBufferPreviewer() {
	// Time interval within the plot canvas
	QwtInterval plotInterval(m_plot->xAxis()->min(),  m_plot->xAxis()->max());

	// Time interval that represents the captured data
	QwtInterval dataInterval(0.0, m_bufferPrevData);

	// Use the two intervals to determine the width and position of the
	// waveform and of the highlighted area
	QwtInterval fullInterval = plotInterval | dataInterval;
	double wPos = 1 - (fullInterval.maxValue() - dataInterval.minValue()) /
						  fullInterval.width();
	double wWidth = dataInterval.width() / fullInterval.width();

	double hPos = 1 - (fullInterval.maxValue() - plotInterval.minValue()) /
						  fullInterval.width();
	double hWidth = plotInterval.width() / fullInterval.width();


	m_bufferPreviewer->setWaveformWidth(wWidth);
	m_bufferPreviewer->setWaveformPos(wPos);
	m_bufferPreviewer->setHighlightWidth(hWidth);
	m_bufferPreviewer->setHighlightPos(hPos);
}

PlotBufferPreviewerController::PlotBufferPreviewerController(PlotWidget *p, BufferPreviewer *b, QWidget *parent)
{
	m_plot = p;
	m_bufferPreviewer = b;
	setupBufferPreviewer();
}

PlotBufferPreviewerController::~PlotBufferPreviewerController()
{

}

TimePlotInfo::TimePlotInfo(PlotWidget *plot, QWidget *parent)
{
	m_plot = plot;
	QVBoxLayout *vlay = new QVBoxLayout(this);
	vlay->setMargin(4);
	vlay->setSpacing(2);
	setLayout(vlay);
	QHBoxLayout *lay = new QHBoxLayout();
	lay->setMargin(0);
	lay->setSpacing(0);

	m_hdiv = new TimePlotHDivInfo(this);
	m_sampling = new TimePlotSamplingInfo(this);
	m_status = new TimePlotStatusInfo(this);

	m_bufferPreviewer = new AnalogBufferPreviewer(this);
	m_bufferController = new PlotBufferPreviewerController(plot, m_bufferPreviewer, this);

	vlay->addWidget(m_bufferPreviewer);
	vlay->addLayout(lay);

#ifdef HOVER_INFO
	lay->addWidget(m_hdiv);
	lay->addWidget(m_sampling);
	lay->setAlignment(m_sampling,Qt::AlignRight);
#else

	HoverWidget *hdivhover = new HoverWidget(nullptr, plot->plot()->canvas(), plot->plot());
	hdivhover->setContent(m_hdiv);
	hdivhover->setAnchorPos(HoverPosition::HP_TOPLEFT);
	hdivhover->setContentPos(HoverPosition::HP_BOTTOMRIGHT);
	hdivhover->setAnchorOffset(QPoint(8,6));
	hdivhover->show();
	m_hdiv->setAttribute(Qt::WA_TransparentForMouseEvents);

	HoverWidget *samplinginfohover = new HoverWidget(nullptr,  plot->plot()->canvas(), plot->plot());
	samplinginfohover->setContent(m_sampling);
	samplinginfohover->setAnchorPos(HoverPosition::HP_TOPRIGHT);
	samplinginfohover->setContentPos(HoverPosition::HP_BOTTOMLEFT);
	samplinginfohover->setAnchorOffset(QPoint(-8,6));
	samplinginfohover->show();
	samplinginfohover->setAttribute(Qt::WA_TransparentForMouseEvents);
#endif
}

TimePlotInfo::~TimePlotInfo()
{

}

void TimePlotInfo::update(PlotSamplingInfo info) {
	auto x = m_plot->xAxis();
	auto max = x->max();
	auto min = x->min();
	auto divs = x->divs();
	double hdiv = abs(max - min) / divs;
	m_hdiv->update(hdiv);
	m_sampling->update(info.plotSize, info.bufferSize, info.sampleRate);
	m_bufferController->updateDataLimits(min, max);
}

void TimePlotInfo::updateBufferPreviewer()
{
	m_bufferController->updateBufferPreviewer();
}
