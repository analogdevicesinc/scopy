#include "xyplot.h"
#include "plotaxis.h"
#include "stylehelper.h"

using namespace scopy::qiqplugin;

XYPlot::XYPlot() { m_plot = new PlotWidget(); }

XYPlot::~XYPlot()
{
	if(m_plot) {
		delete m_plot;
	}
}

int XYPlot::id() { return m_plotInfo.id; }

void XYPlot::init(QIQPlotInfo info, int samplingFreq)
{
	m_plotInfo = info;
	m_plot->plot()->setTitle(info.title);
	initAxis();
	int chCount = info.channels.size();
	for(int i = 0; i < chCount; i++) {
		QString chId = "ch" + QString::number(i);
		QPen pen(StyleHelper::getChannelColor(i));
		PlotChannel *ch = new PlotChannel(chId, pen, m_plot->xAxis(), m_plot->yAxis(), m_plot);
		m_plot->addPlotChannel(ch);
	}
}

void XYPlot::updateData(QList<CurveData> curveData)
{
	const QList<PlotChannel *> channels = m_plot->getChannels();
	if(curveData.size() != channels.size()) {
		qWarning() << "Dataset size must be the same as the channels size!";
		return;
	}
	for(PlotChannel *ch : channels) {
		CurveData curve = curveData.takeFirst();
		ch->curve()->setSamples(curve.x, curve.y);
	}
	m_plot->replot();
}

QWidget *XYPlot::widget() { return m_plot; }

void XYPlot::setSamplingFreq(int samplingFreq) {}

void XYPlot::initAxis()
{
	m_plot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_plot->xAxis()->setUnits(m_plotInfo.xLabel);
	m_plot->xAxis()->setInterval(-1.5, 1.5);
	m_plot->xAxis()->setVisible(true);

	m_plot->yAxis()->setUnits(m_plotInfo.yLabel);
	m_plot->yAxis()->setInterval(-1.5, 1.5);
	m_plot->yAxis()->setVisible(true);
}
