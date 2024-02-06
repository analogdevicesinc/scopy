#include "monitorplot.hpp"

#include <cursorcontroller.h>
#include <plotinfo.h>

#include <pluginbase/preferences.h>

using namespace scopy;
using namespace datamonitor;

MonitorPlot::MonitorPlot(QWidget *parent)
	: QWidget{parent}
{
	installEventFilter(this);

	QVBoxLayout *layout = new QVBoxLayout(this);
	setLayout(layout);

	m_plot = new PlotWidget(this);

	// TODO set this and set autoscale
	m_plot->xAxis()->setInterval(0, 100);
	m_plot->xAxis()->setVisible(true);

	m_plotInfo = new TimePlotInfo(m_plot, this);

	layout->addWidget(m_plotInfo);
	layout->addWidget(m_plot);

	TimePlotStatusInfo *status = new TimePlotStatusInfo(this);
	layout->addWidget(status);

	m_monitorCurves = new QMap<QString, PlotData *>();
}

PlotWidget *MonitorPlot::plot() const { return m_plot; }

void MonitorPlot::addMonitor(DataMonitorModel *dataMonitorModel)
{
	QPen chpen = QPen(dataMonitorModel->getColor(), 1);
	PlotAxis *chPlotAxis = new PlotAxis(QwtAxis::YLeft, m_plot, chpen);

	m_plot->plot()->setAxisScale(chPlotAxis->axisId(), 0, 10);
	chPlotAxis->setVisible(true);


	PlotChannel *plotch =
		new PlotChannel(dataMonitorModel->getName(), chpen, m_plot, m_plot->xAxis(), chPlotAxis, this);
	plotch->setEnabled(true);

	PlotData *pd = new PlotData();
	pd->plotch = plotch;
	pd->xdata = dataMonitorModel->getXdata();
	pd->ydata = dataMonitorModel->getYdata();

	// size from pref
	Preferences *p = Preferences::GetInstance();
	auto dataSizePref = p->get("datamonitor_data_storage_size").toString().split(" ");
	int dataSize = dataSizePref[0].toInt();
	if (dataSizePref[1] == "Kb") {
		dataSize *= 1000;
	} else if (dataSizePref[1] == "Mb") {
		dataSize *= 1000000;
	}


	// TODO CLEAR PLOT ON CLEAR DATA
	connect(dataMonitorModel, &DataMonitorModel::dataCleared, this, [=]() {
		pd->xdata = dataMonitorModel->getXdata();
		pd->ydata = dataMonitorModel->getYdata();
		plotch->curve()->setRawSamples(pd->xdata->data(), pd->ydata->data(), dataSize);
		m_plot->replot();
	});

	plotch->curve()->setRawSamples(pd->xdata->data(), pd->ydata->data(), dataSize);

	m_monitorCurves->insert(dataMonitorModel->getName(), pd);

	connect(dataMonitorModel, &DataMonitorModel::valueUpdated, m_plot, [=]() { m_plot->replot(); });

	m_plot->replot();
}

void MonitorPlot::removeMonitor(QString monitorTitle)
{
	m_plot->removePlotChannel(m_monitorCurves->value(monitorTitle)->plotch);
	m_monitorCurves->remove(monitorTitle);
	delete m_monitorCurves->value(monitorTitle);
}

void MonitorPlot::toggleMonitor(bool toggled, QString monitorTitle)
{
	if(m_monitorCurves->contains(monitorTitle)) {
		m_monitorCurves->value(monitorTitle)->plotch->setEnabled(toggled);
		m_plot->replot();
	}
}

bool MonitorPlot::hasMonitor(QString title) { return m_monitorCurves->contains(title); }

bool MonitorPlot::firstMonitor() const { return m_firstMonitor; }

void MonitorPlot::setFirstMonitor(bool newFirstMonitor) { m_firstMonitor = newFirstMonitor; }

void MonitorPlot::changeCurveStyle(QString plotCurve, int style)
{
	if(m_monitorCurves->contains(plotCurve)) {

		auto curve = m_monitorCurves->value(plotCurve)->plotch->curve();

		curve->setPaintAttribute(QwtPlotCurve::ClipPolygons, true);
		curve->setCurveAttribute(QwtPlotCurve::Fitted, false);

		switch(style) {
		case 0:
			curve->setStyle(QwtPlotCurve::CurveStyle::Lines);
			m_plot->replot();
			break;
		case 1:
			curve->setStyle(QwtPlotCurve::CurveStyle::Dots);
			m_plot->replot();
			break;
		case 2:
			curve->setStyle(QwtPlotCurve::CurveStyle::Steps);
			m_plot->replot();
			break;
		case 3:
			curve->setStyle(QwtPlotCurve::CurveStyle::Sticks);
			m_plot->replot();
			break;
		case 4:
			curve->setPaintAttribute(QwtPlotCurve::ClipPolygons, false);
			curve->setCurveAttribute(QwtPlotCurve::Fitted, true);
			curve->setStyle(QwtPlotCurve::CurveStyle::Lines);
			m_plot->replot();
			break;
		}
	}
}

void MonitorPlot::setMainMonitor(QString newMainMonitor)
{
	if(m_monitorCurves->contains(newMainMonitor)) {
		m_monitorCurves->value(newMainMonitor)->plotch->detach();
		m_monitorCurves->value(newMainMonitor)->plotch->attach();
	}
	m_plot->replot();
}

void MonitorPlot::clearMonitor()
{
	//	m_monitorCurves->values()

	// int size = 10000;
	// m_monitorCurves->begin().value()->plotch->curve()->setRawSamples(m_monitorCurves->begin().value()->ydata->data(),
	// m_monitorCurves->begin().value()->xdata->data(), size);

	//	for (auto monitor: m_monitorCurves->keys()){
	//		m_monitorCurves->value(monitor)->xdata->erase(m_monitorCurves->value(monitor)->xdata->begin(),
	// m_monitorCurves->value(monitor)->xdata->end());
	//		m_monitorCurves->value(monitor)->ydata->erase(m_monitorCurves->value(monitor)->ydata->begin(),
	// m_monitorCurves->value(monitor)->ydata->end());
	//	}
}
