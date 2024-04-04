#include "measurecomponent.h"
#include "menucontrolbutton.h"
#include "measurementsettings.h"

using namespace scopy;
using namespace scopy::adc;

MeasureComponent::MeasureComponent(ToolTemplate *tool, QObject *parent) {

	MenuControlButton *measure = new MenuControlButton();
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


	m_measurePanel = new MeasurementsPanel(tool);
	tool->topStack()->add(measureMenuId, m_measurePanel);

	m_statsPanel = new StatsPanel(tool);
	tool->bottomStack()->add(statsMenuId, m_statsPanel);
	connect(m_measureSettings, &MeasurementSettings::enableMeasurementPanel, tool->topCentral(),
		&QWidget::setVisible);
	connect(m_measureSettings, &MeasurementSettings::enableStatsPanel, tool->bottomCentral(), &QWidget::setVisible);
	connect(m_measureSettings, &MeasurementSettings::sortMeasurements, m_measurePanel, &MeasurementsPanel::sort);
	connect(m_measureSettings, &MeasurementSettings::sortStats, m_statsPanel, &StatsPanel::sort);
	connect(measure, &MenuControlButton::toggled, this, [=](bool b){
	if(b) {
		tool->requestMenu(measureMenuId);
		tool->requestMenu(statsMenuId);
	}
	tool->openTopContainerHelper(b);
	tool->openBottomContainerHelper(b);

	});
	tool->addWidgetToBottomContainerHelper(measure, TTA_RIGHT);


}

void MeasureComponent::setupMeasureButtonHelper(MenuControlButton *btn)
{
	btn->setName("Measure");
	btn->setOpenMenuChecksThis(true);
	btn->setDoubleClickToOpenMenu(true);
	btn->checkBox()->setVisible(false);
}

MeasurementsPanel *MeasureComponent::measurePanel()
{
	return m_measurePanel;
}

StatsPanel *MeasureComponent::statsPanel()
{
	return m_statsPanel;
}

MeasurementSettings *MeasureComponent::measureSettings()
{
	return m_measureSettings;
}
