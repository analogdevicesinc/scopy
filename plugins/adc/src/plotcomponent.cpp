#include "plotcomponent.h"
#include "plotaxis.h"

using namespace scopy;
using namespace adc;

PlotComponent::PlotComponent(QString name, QWidget *parent)
	: QWidget(parent)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_lay = new QVBoxLayout(this);
	m_lay->setMargin(0);
	m_lay->setSpacing(0);
	m_plot = new PlotWidget(this);
	m_plot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	setLayout(m_lay);
	m_name = name;

	m_plot->xAxis()->setInterval(0, 1);
	m_plot->leftHandlesArea()->setVisible(true);
	m_plot->rightHandlesArea()->setVisible(true);
	m_plot->bottomHandlesArea()->setVisible(true);
	m_plot->xAxis()->setVisible(true);
	//	m_plotWidget->topHandlesArea()->setVisible(true);

	m_info = new TimePlotInfo(m_plot, this);
	m_plot->addPlotInfoSlot(m_info);
/*	connect(m_plot->navigator(), &PlotNavigator::rectChanged, this,
		[=]() { m_info->update(m_currentSamplingInfo); });
	*/

	m_measurePanel = new MeasurementsPanel(this);
	m_measurePanel->setFixedHeight(100);
	//tool->topStack()->add(measureMenuId, m_measurePanel);

	m_statsPanel = new StatsPanel(this);
	m_statsPanel->setFixedHeight(80);
	//tool->bottomStack()->add(statsMenuId, m_statsPanel);
	m_lay->addWidget(m_measurePanel);
	m_lay->addWidget(m_plot);
	m_lay->addWidget(m_statsPanel);
	m_measurePanel->setVisible(false);
	m_statsPanel->setVisible(false);


}

PlotComponent::~PlotComponent() {}

PlotWidget *PlotComponent::plot() { return m_plot; }

void PlotComponent::replot() { m_plot->replot(); }

void PlotComponent::enableMeasurementPanel(bool b)
{
	m_measurePanel->setVisible(b);
}

void PlotComponent::enableStatsPanel(bool b)
{
	m_statsPanel->setVisible(b);
}

void PlotComponent::onStart() {}

void PlotComponent::onStop() {}

void PlotComponent::onInit() {}

void PlotComponent::onDeinit() {}

MeasurementsPanel *PlotComponent::measurePanel() const
{
	return m_measurePanel;
}

StatsPanel *PlotComponent::statsPanel() const
{
	return m_statsPanel;
}
