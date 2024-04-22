#include "measurecomponent.h"
#include "menucontrolbutton.h"
#include "measurementsettings.h"

using namespace scopy;
using namespace scopy::adc;

MeasureComponent::MeasureComponent(ToolTemplate *tool, QObject *parent) {

	measure = new MenuControlButton();
	setupMeasureButtonHelper(measure);

	m_measureSettings = new MeasurementSettings(tool);
	HoverWidget *measurePanelManagerHover = new HoverWidget(nullptr, measure, tool);
	measurePanelManagerHover->setContent(m_measureSettings);
	measurePanelManagerHover->setAnchorPos(HoverPosition::HP_TOPRIGHT);
	measurePanelManagerHover->setContentPos(HoverPosition::HP_TOPLEFT);
	connect(measure->button(), &QPushButton::toggled, this, [=](bool b) {
		measurePanelManagerHover->setVisible(b);
		measurePanelManagerHover->raise();
	});

	connect(measure, &MenuControlButton::toggled, this, [=](bool b){
		if(b) {
			tool->requestMenu(measureMenuId);
			tool->requestMenu(statsMenuId);
		}
		// tool->openTopContainerHelper(b);
		// tool->openBottomContainerHelper(b);
		for( PlotComponent *p : m_plotComponents) {
			p->enableMeasurementPanel(m_measureSettings->measurementEnabled() && b);
			p->enableStatsPanel(m_measureSettings->statsEnabled() && b);
		}

	});
	tool->addWidgetToBottomContainerHelper(measure, TTA_RIGHT);
}

void MeasureComponent::addPlotComponent(PlotComponent *p) {
	MeasurementsPanel *m_measurePanel = p->measurePanel();
	StatsPanel *m_statsPanel = p->statsPanel();

	connect(m_measureSettings, &MeasurementSettings::enableMeasurementPanel, p, &PlotComponent::enableMeasurementPanel);
	connect(m_measureSettings, &MeasurementSettings::enableStatsPanel, p, &PlotComponent::enableStatsPanel);
	connect(m_measureSettings, &MeasurementSettings::sortMeasurements, m_measurePanel, &MeasurementsPanel::sort);
	connect(m_measureSettings, &MeasurementSettings::sortStats, m_statsPanel, &StatsPanel::sort);

	m_plotComponents.append(p);


}

void MeasureComponent::removePlotComponent(PlotComponent*p) {
	MeasurementsPanel *m_measurePanel = p->measurePanel();
	StatsPanel *m_statsPanel = p->statsPanel();

	disconnect(m_measureSettings, &MeasurementSettings::enableMeasurementPanel, p, &PlotComponent::enableMeasurementPanel);
	disconnect(m_measureSettings, &MeasurementSettings::enableStatsPanel, p, &PlotComponent::enableStatsPanel);
	disconnect(m_measureSettings, &MeasurementSettings::sortMeasurements, m_measurePanel, &MeasurementsPanel::sort);
	disconnect(m_measureSettings, &MeasurementSettings::sortStats, m_statsPanel, &StatsPanel::sort);

	m_plotComponents.removeAll(p);

}

void MeasureComponent::setupMeasureButtonHelper(MenuControlButton *btn)
{
	btn->setName("Measure");
	btn->setOpenMenuChecksThis(true);
	btn->setDoubleClickToOpenMenu(true);
	btn->checkBox()->setVisible(false);
}

MeasurementSettings *MeasureComponent::measureSettings()
{
	return m_measureSettings;
}
