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

#include <timeplotmanager.h>
#include <timeplotcomponentchannel.h>
#include <plotaxis.h>
#include <gui/plotmanagercombobox.h>
#include "plotnavigator.hpp"
#include <timeplotcomponentsettings.h>

using namespace scopy;
using namespace scopy::adc;

TimePlotManager::TimePlotManager(QString name, QWidget *parent)
	: PlotManager(name, parent)
{
	m_primary = nullptr;

	m_bufferpreviewer = new AnalogBufferPreviewer();
	m_plotpreviewer = nullptr;

	int idx = m_lay->indexOf(m_statsPanel);
	m_lay->insertWidget(idx, m_bufferpreviewer);
}

TimePlotManager::~TimePlotManager() {}

uint32_t TimePlotManager::addPlot(QString name)
{
	TimePlotComponent *plt = new TimePlotComponent(name, m_plotIdx, this);
	m_plotIdx++;
	m_plots.append(plt);
	if(m_primary == nullptr) {
		m_primary = plt;
		m_plotpreviewer = new PlotBufferPreviewer(m_primary->plot(0), m_bufferpreviewer, m_primary->plot(0));
		int idx = m_lay->indexOf(m_statsPanel);
		m_lay->insertWidget(idx, m_plotpreviewer);
	}

	connect(this, &PlotManager::newData, plt->plot(0), &PlotWidget::newData);

	plt->setXInterval(m_xInterval);

	connect(plt, &TimePlotComponent::requestDeletePlot, this, [=]() {
		Q_EMIT plotRemoved(plt->uuid());
		removePlot(plt->uuid());

		delete plt->plotMenu();
		delete plt;
	});

	addComponent(plt);

	int idx = m_lay->indexOf(m_statsPanel);
	m_lay->insertWidget(idx, plt);
	for(PlotManagerCombobox *p : m_channelPlotcomboMap.values()) {
		p->addPlot(plt);
	}

	multiPlotUpdate();
	syncNavigatorAndCursors(plt);
	Q_EMIT plotAdded(plt->uuid());
	return plt->uuid();
}

void TimePlotManager::removePlot(uint32_t uuid)
{
	PlotComponent *plt = plot(uuid);
	m_plots.removeAll(plt);
	removeComponent(plt);
	m_lay->removeWidget(plt);

	for(PlotManagerCombobox *p : m_channelPlotcomboMap.values()) {
		p->removePlot(plt);
	}

	multiPlotUpdate();
	// syncAllPlotNavigatorsAndCursors();
}

TimePlotComponent *TimePlotManager::plot(uint32_t uuid)
{
	return dynamic_cast<TimePlotComponent *>(PlotManager::plot(uuid));
}

void TimePlotManager::multiPlotUpdate()
{
	bool b = m_plots.count() > 1;

	for(PlotComponent *p : qAsConst(m_plots)) {
		auto plt = dynamic_cast<TimePlotComponent *>(p);
		plt->plotMenu()->showPlotButtons(b);

		// do not allow users to delete the primary plot
		plt->plotMenu()->showDeleteButtons(b && plt != m_primary);
	}

	for(PlotManagerCombobox *cb : qAsConst(m_channelPlotcomboMap)) {
		cb->setVisible(b);
	}
}

void TimePlotManager::syncNavigatorAndCursors(PlotComponent *p)
{
	if(p == m_primary)
		return;

	PlotNavigator::syncPlotNavigators(m_primary->plot(0)->navigator(), p->plot(0)->navigator());
	CursorController::syncXCursorControllers(m_primary->cursor(), p->cursor());
}

void TimePlotManager::syncAllPlotNavigatorsAndCursors()
{
	if(m_primary != m_plots[0]) {
		m_primary = m_plots[0];
		for(PlotComponent *p : qAsConst(m_plots)) {
			syncNavigatorAndCursors(p);
		}
	}
}
