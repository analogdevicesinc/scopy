#include "datamonitorview.hpp"

#include <monitorplot.hpp>

using namespace scopy;
using namespace datamonitor;

DataMonitorView::DataMonitorView(QWidget *parent)
	: QWidget{parent}
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	this->setLayout(layout);

	m_monitorPlot = new MonitorPlot(this);
	layout->addWidget(m_monitorPlot);
}

MonitorPlot *DataMonitorView::monitorPlot() const
{
	return m_monitorPlot;
}
