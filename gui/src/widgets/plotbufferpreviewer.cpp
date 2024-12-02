#include "plotbufferpreviewer.h"
#include "plotaxis.h"

using namespace scopy;

PlotBufferPreviewer::PlotBufferPreviewer(PlotWidget *p, BufferPreviewer *b, QWidget *parent)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	QVBoxLayout *layout = new QVBoxLayout(this);
	setLayout(layout);
	layout->setMargin(0);
	layout->setSpacing(0);
	layout->addWidget(b);

	m_plot = p;
	m_bufferPreviewer = b;
	setupBufferPreviewer();
}

PlotBufferPreviewer::~PlotBufferPreviewer() {}

void PlotBufferPreviewer::setupBufferPreviewer()
{
	m_bufferPreviewer->setMinimumHeight(20);
	m_bufferPreviewer->setCursorPos(0.5);
	m_bufferPreviewer->setHighlightPos(0.05);
	m_bufferPreviewer->setHighlightWidth(0.2);
	m_bufferPreviewer->setCursorVisible(false);
	m_bufferPreviewer->setWaveformPos(0.1);
	m_bufferPreviewer->setWaveformWidth(0.5);

	updateDataLimits(m_plot->xAxis()->min(), m_plot->xAxis()->max());

	connect(m_bufferPreviewer, &BufferPreviewer::bufferStopDrag, this, [=]() {});

	connect(m_bufferPreviewer, &BufferPreviewer::bufferStartDrag, this, [=]() {
		m_bufferPrevInitMin = m_plot->xAxis()->min();
		m_bufferPrevInitMax = m_plot->xAxis()->max();
	});

	connect(m_bufferPreviewer, &BufferPreviewer::bufferMovedBy, this, [=](int value) {
		double moveTo = 0.0;

		int width = m_bufferPreviewer->width();
		double xAxisWidth = m_bufferPrevData;
		if(m_plot->xAxis()->min() > m_plot->xAxis()->max()) {
			value *= -1;
		}

		moveTo = value * xAxisWidth / width;
		m_plot->xAxis()->setInterval(m_bufferPrevInitMin + moveTo, m_bufferPrevInitMax + moveTo);
		m_plot->replot();

		updateBufferPreviewer();
	});

	connect(m_bufferPreviewer, &BufferPreviewer::bufferResetPosition, this, [=]() {
		if(m_plot->xAxis()->min() > m_plot->xAxis()->max()) {
			m_plot->xAxis()->setInterval(m_bufferPrevData, 0.);
		} else {
			m_plot->xAxis()->setInterval(0., m_bufferPrevData);
		}
		m_plot->xAxis()->updateAxisScale();
	});
}

void PlotBufferPreviewer::updateDataLimits()
{
	PlotAxis *xAxis = (m_plot->selectedChannel()) ? m_plot->selectedChannel()->xAxis() : m_plot->xAxis();
	m_bufferPrevData = abs(xAxis->max() - xAxis->min());
	updateBufferPreviewer();
}

void PlotBufferPreviewer::updateDataLimits(double min, double max)
{
	m_bufferPrevData = abs(max - min);
	updateBufferPreviewer();
}

void PlotBufferPreviewer::updateBufferPreviewer()
{
	// Time interval within the plot canvas
	double left = m_plot->xAxis()->visibleMin();
	double right = m_plot->xAxis()->visibleMax();
	QwtInterval plotInterval(std::min(left, right), std::max(left, right));

	// Time interval that represents the captured data
	QwtInterval dataInterval(0.0, m_bufferPrevData);

	// Use the two intervals to determine the width and position of the
	// waveform and of the highlighted area
	QwtInterval fullInterval = plotInterval | dataInterval;
	double wPos = 1 - (fullInterval.maxValue() - dataInterval.minValue()) / fullInterval.width();
	double wWidth = dataInterval.width() / fullInterval.width();

	double hPos = 1 - (fullInterval.maxValue() - plotInterval.minValue()) / fullInterval.width();
	double hWidth = plotInterval.width() / fullInterval.width();
	if(left > right) {
		hPos = wWidth - hPos - hWidth;
	}

	m_bufferPreviewer->setWaveformWidth(wWidth);
	m_bufferPreviewer->setWaveformPos(wPos);
	m_bufferPreviewer->setHighlightWidth(hWidth);
	m_bufferPreviewer->setHighlightPos(hPos);
}
