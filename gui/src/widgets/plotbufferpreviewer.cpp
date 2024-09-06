#include "plotbufferpreviewer.h"
#include "plotaxis.h"
#include <cmath>
#include <plotmagnifier.hpp>
#include <plotnavigator.hpp>

using namespace scopy;

PlotBufferPreviewer::PlotBufferPreviewer(PlotWidget *p, BufferPreviewer *b, QWidget *parent)
	: QWidget{parent}
	, m_manualDataLimits(false)
	, m_lastMin(p->xAxis()->visibleMin())
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

void PlotBufferPreviewer::setManualDataLimits(bool enabled) { m_manualDataLimits = enabled; }

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

	connect(m_bufferPreviewer, &BufferPreviewer::bufferStartDrag, this, [=]() {
		// reset the buffer preview position to current visible section
		m_lastMin = m_plot->xAxis()->visibleMin();
	});

	connect(m_bufferPreviewer, &BufferPreviewer::bufferMovedBy, this, [=](int bufferPos) {
		double bufferWidth = m_bufferPreviewer->width();
		double axisWidth = m_bufferDataLimitMax - m_bufferDataLimitMin;
		double newAxisPos = bufferPos * axisWidth / bufferWidth;
		double axisOffset = m_lastMin - m_plot->xAxis()->visibleMin();
		if(axisWidth < 0)
			axisOffset *= -1;

		double panAmount = PlotMagnifier::scaleToFactor(newAxisPos + axisOffset, m_plot->xAxis()->axisId(),
								m_plot->plot());

		bool bounded = m_plot->navigator()->isBounded();
		m_plot->navigator()->setBounded(false);
		m_plot->navigator()->forcePan(m_plot->xAxis()->axisId(), panAmount);
		m_plot->navigator()->setBounded(bounded);
		updateBufferPreviewer();
	});

	connect(m_bufferPreviewer, &BufferPreviewer::bufferResetPosition, this, [=]() {
		Q_EMIT m_plot->navigator()->reset();
		updateBufferPreviewer();
	});

	connect(m_plot->navigator(), &PlotNavigator::rectChanged, this, [=]() { updateBufferPreviewer(); });
}

void PlotBufferPreviewer::updateDataLimits(double min, double max)
{
	m_bufferDataLimitMin = min;
	m_bufferDataLimitMax = max;
	updateBufferPreviewer();
}

void PlotBufferPreviewer::updateBufferPreviewer()
{
	PlotAxis *xAxis = (m_plot->selectedChannel()) ? m_plot->selectedChannel()->xAxis() : m_plot->xAxis();

	// Time interval within the plot canvas
	double left = xAxis->visibleMin();
	double right = xAxis->visibleMax();
	QwtInterval plotInterval(std::min(left, right), std::max(left, right));

	// Time interval that represents the captured data
	if(!m_manualDataLimits) {
		m_bufferDataLimitMin = xAxis->min();
		m_bufferDataLimitMax = xAxis->max();
	}
	QwtInterval dataInterval(std::min(m_bufferDataLimitMin, m_bufferDataLimitMax),
				 std::fmax(m_bufferDataLimitMin, m_bufferDataLimitMax));

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

#include "moc_plotbufferpreviewer.cpp"
