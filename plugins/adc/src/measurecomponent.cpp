/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "measurecomponent.h"
#include "menucontrolbutton.h"
#include "measurementsettings.h"
#include <QButtonGroup>
using namespace scopy;
using namespace scopy::adc;

MeasureComponent::MeasureComponent(ToolTemplate *tool, QButtonGroup *btngroup, MeasurementPanelInterface *p,
				   QObject *parent)
	: QObject(parent)
	, ToolComponent()
	, hoverBtnGroup(btngroup)
{

	m_measurementPanelInterface = p;
	measure = new MenuControlButton();
	setupMeasureButtonHelper(measure);

	m_measureSettings = new MeasurementSettings(tool);
	HoverWidget *measurePanelManagerHover = new HoverWidget(nullptr, measure, tool);
	measurePanelManagerHover->setContent(m_measureSettings);
	measurePanelManagerHover->setAnchorPos(HoverPosition::HP_TOPRIGHT);
	measurePanelManagerHover->setContentPos(HoverPosition::HP_TOPLEFT);
	measurePanelManagerHover->setAnchorOffset(QPoint(0, -10));
	connect(measure->button(), &QPushButton::toggled, this, [=](bool b) {
		measurePanelManagerHover->setVisible(b);
		measurePanelManagerHover->raise();
	});

	connect(measure, &MenuControlButton::toggled, this, [=](bool b) {
		if(b) {
			tool->requestMenu(measureMenuId);
			tool->requestMenu(statsMenuId);
		}
		// tool->openTopContainerHelper(b);
		// tool->openBottomContainerHelper(b);
		m_measurementPanelInterface->enableMeasurementPanel(m_measureSettings->measurementEnabled() && b);
		m_measurementPanelInterface->enableStatsPanel(m_measureSettings->statsEnabled() && b);
		m_measurementPanelInterface->enableMarkerPanel(m_measureSettings->markerEnabled() && b);
	});
	tool->addWidgetToBottomContainerHelper(measure, TTA_RIGHT);

	MeasurementsPanel *m_measurePanel = p->measurePanel();
	StatsPanel *m_statsPanel = p->statsPanel();

	connect(m_measureSettings, SIGNAL(enableMeasurementPanel(bool)), dynamic_cast<QObject *>(p),
		SLOT(enableMeasurementPanel(bool)));
	connect(m_measureSettings, SIGNAL(enableStatsPanel(bool)), dynamic_cast<QObject *>(p),
		SLOT(enableStatsPanel(bool)));
	connect(m_measureSettings, SIGNAL(enableMarkerPanel(bool)), dynamic_cast<QObject *>(p),
		SLOT(enableMarkerPanel(bool)));
	connect(m_measureSettings, &MeasurementSettings::sortMeasurements, m_measurePanel, &MeasurementsPanel::sort);
	connect(m_measureSettings, &MeasurementSettings::sortStats, m_statsPanel, &StatsPanel::sort);
}
/*
void MeasureComponent::removePlotComponent(PlotComponent*p) {
	MeasurementsPanel *m_measurePanel = p->measurePanel();
	StatsPanel *m_statsPanel = p->statsPanel();

	disconnect(m_measureSettings, &MeasurementSettings::enableMeasurementPanel, p,
&PlotComponent::enableMeasurementPanel); disconnect(m_measureSettings, &MeasurementSettings::enableStatsPanel, p,
&PlotComponent::enableStatsPanel); disconnect(m_measureSettings, &MeasurementSettings::sortMeasurements, m_measurePanel,
&MeasurementsPanel::sort); disconnect(m_measureSettings, &MeasurementSettings::sortStats, m_statsPanel,
&StatsPanel::sort);

	m_plotComponents.removeAll(p);

}*/

void MeasureComponent::setupMeasureButtonHelper(MenuControlButton *btn)
{
	btn->setName("Measure");
	btn->setOpenMenuChecksThis(true);
	btn->setDoubleClickToOpenMenu(true);
	btn->checkBox()->setVisible(false);
	hoverBtnGroup->addButton(btn->button());
}

MeasurementSettings *MeasureComponent::measureSettings() { return m_measureSettings; }
