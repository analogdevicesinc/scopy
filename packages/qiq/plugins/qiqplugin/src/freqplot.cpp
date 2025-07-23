#include "freqplot.h"
#include "plotaxis.h"
#include "stylehelper.h"

using namespace scopy::qiqplugin;

FreqPlot::FreqPlot() { m_plot = new PlotWidget(); }

FreqPlot::~FreqPlot()
{
	if(m_plot) {
		delete m_plot;
	}
}

int FreqPlot::id() { return m_plotInfo.id; }

void FreqPlot::init(QIQPlotInfo info, int samplingFreq)
{
	m_plotInfo = info;
	m_samplingFreq = samplingFreq;
	m_plot->plot()->setTitle(info.title);
	computeXAxis(samplingFreq);
	initAxis();
	int chCount = info.channels.size();
	for(int i = 0; i < chCount; i++) {
		QString chId = "ch" + QString::number(i);
		QPen pen(StyleHelper::getChannelColor(i));
		PlotChannel *ch = new PlotChannel(chId, pen, m_plot->xAxis(), m_plot->yAxis(), m_plot);
		m_plot->addPlotChannel(ch);
	}
}

void FreqPlot::updateData(QList<CurveData> curveData)
{
	const QList<PlotChannel *> channels = m_plot->getChannels();
	if(curveData.size() != channels.size()) {
		qWarning() << "Dataset size must be the same as the channels size!";
		return;
	}
	for(PlotChannel *ch : channels) {
		CurveData curve = curveData.takeFirst();
		if(curve.x.isEmpty()) {
			computeXAxis(curve.y.size());
			ch->curve()->setSamples(m_xFreq, curve.y);
		} else {
			ch->curve()->setSamples(curve.x, curve.y);
		}
	}
	m_plot->replot();
}

void FreqPlot::setSamplingFreq(int samplingFreq) {}

QWidget *FreqPlot::widget() { return m_plot; }

void FreqPlot::computeXAxis(int size)
{
	if(m_xFreq.size() == size) {
		return;
	}
	m_xFreq.clear();
	double deltaF = (double)m_samplingFreq / size;
	for(int i = -size / 2; i <= size / 2; ++i) { // doar partea pozitivă
		m_xFreq.append(i * deltaF);
	}
}

void FreqPlot::initAxis()
{
	m_plot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_plot->xAxis()->setUnits(m_plotInfo.xLabel);
	m_plot->xAxis()->setInterval(-m_samplingFreq / 2, m_samplingFreq / 2);
	m_plot->xAxis()->setVisible(true);

	m_plot->yAxis()->setUnits(m_plotInfo.yLabel);
	m_plot->yAxis()->setVisible(true);
}
