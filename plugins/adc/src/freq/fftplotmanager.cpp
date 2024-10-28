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

#include "fftplotmanager.h"
#include <plotaxis.h>
#include "plotnavigator.hpp"
#include <gui/plotmanagercombobox.h>
#include <freq/fftplotcomponentchannel.h>
#include <freq/fftplotcomponentsettings.h>

using namespace scopy;
using namespace scopy::adc;

FFTPlotManager::FFTPlotManager(QString name, QWidget *parent)
	: PlotManager(name, parent)
{
	m_primary = nullptr;
}

FFTPlotManager::~FFTPlotManager() {}

uint32_t FFTPlotManager::addPlot(QString name)
{
	FFTPlotComponent *plt = new FFTPlotComponent(name, m_plotIdx, this);
	m_plotIdx++;
	m_plots.append(plt);
	if(m_primary == nullptr) {
		m_primary = plt;
	}

	connect(this, &PlotManager::newData, plt->plot(0), &PlotWidget::newData);

	plt->setXInterval(m_xInterval);

	connect(plt, &FFTPlotComponent::requestDeletePlot, this, [=]() {
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

void FFTPlotManager::removePlot(uint32_t uuid)
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

FFTPlotComponent *FFTPlotManager::plot(uint32_t uuid)
{
	return dynamic_cast<FFTPlotComponent *>(PlotManager::plot(uuid));
}

void FFTPlotManager::enableMeasurementPanel(bool)
{
	// TODO: Remove this whole function once the measurements for fft
	// are implemented. This is just a placeholder.
}

void FFTPlotManager::multiPlotUpdate()
{
	bool b = m_plots.count() > 1;
	for(PlotComponent *p : qAsConst(m_plots)) {
		auto plt = dynamic_cast<FFTPlotComponent *>(p);
		plt->plotMenu()->showDeleteButtons(b);
	}

	for(PlotManagerCombobox *cb : m_channelPlotcomboMap) {
		cb->setVisible(b);
	}
}

void FFTPlotManager::syncNavigatorAndCursors(PlotComponent *p)
{
	if(p == m_primary)
		return;

	PlotNavigator::syncPlotNavigators(m_primary->plot(0)->navigator(), p->plot(0)->navigator());
	CursorController::syncXCursorControllers(m_primary->cursor(), p->cursor());
}

void FFTPlotManager::syncAllPlotNavigatorsAndCursors()
{
	if(m_primary != m_plots[0]) {
		m_primary = m_plots[0];
		for(PlotComponent *p : qAsConst(m_plots)) {
			syncNavigatorAndCursors(p);
		}
	}
}
