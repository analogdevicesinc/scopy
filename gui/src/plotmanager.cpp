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

#include "plotmanager.h"
#include "plotmanagercombobox.h"
#include "plotaxis.h"

using namespace scopy;

PlotManager::PlotManager(QString name, QWidget *parent)
	: QWidget(parent)
	, MetaComponent()
	, m_plotIdx(0)
{
	m_lay = new QVBoxLayout(this);
	m_lay->setMargin(0);
	m_lay->setSpacing(0);

	m_measurePanel = new MeasurementsPanel(this);
	m_measurePanel->setFixedHeight(110);
	m_measurePanel->setVisible(false);

	m_statsPanel = new StatsPanel(this);
	m_statsPanel->setFixedHeight(100);
	m_statsPanel->setVisible(false);

	m_markerPanel = new MarkerPanel(this);
	m_markerPanel->setVisible(false);

	m_lay->addWidget(m_measurePanel);
	m_lay->addWidget(m_statsPanel);
	m_lay->addWidget(m_markerPanel);
}

PlotManager::~PlotManager() {}

void PlotManager::enableMeasurementPanel(bool b) { m_measurePanel->setVisible(b); }

void PlotManager::enableStatsPanel(bool b) { m_statsPanel->setVisible(b); }

void PlotManager::enableMarkerPanel(bool b) { m_markerPanel->setVisible(b); }

void PlotManager::setXInterval(double xMin, double xMax)
{
	m_xInterval.first = xMin;
	m_xInterval.second = xMax;
	for(auto plt : qAsConst(m_plots)) {
		plt->setXInterval(xMin, xMax);
	}
}

void PlotManager::setXUnit(QString s)
{
	for(PlotComponent *p : m_plots) {
		p->setXUnit(s);
	}
}

void PlotManager::selectChannel(ChannelComponent *c)
{
	c->ctrl()->setChecked(true);
	for(PlotComponentChannel *pcc : qAsConst(m_channels)) {
		if(pcc->channelComponent() == c) {
			pcc->plotComponent()->selectChannel(c);
		}
	}
}

MeasurementsPanel *PlotManager::measurePanel() const { return m_measurePanel; }

StatsPanel *PlotManager::statsPanel() const { return m_statsPanel; }

MarkerPanel *PlotManager::markerPanel() const { return m_markerPanel; }

QWidget *PlotManager::plotCombo(ChannelComponent *c) { return m_channelPlotcomboMap[c]; }

void PlotManager::updateAxisScales()
{
	for(PlotComponent *plt : plots()) {
		for(PlotWidget *pw : plt->plots()) {
			pw->yAxis()->scaleDraw()->invalidateCache();
			pw->xAxis()->scaleDraw()->invalidateCache();
			if(pw->selectedChannel()) {
				pw->selectedChannel()->yAxis()->scaleDraw()->invalidateCache();
				pw->selectedChannel()->xAxis()->scaleDraw()->invalidateCache();
			}
			pw->replot();
		}
	}
}

void PlotManager::addChannel(ChannelComponent *c)
{
	m_channels.append(c->plotChannelCmpt());
	PlotComponent *plt = c->plotChannelCmpt()->plotComponent();
	plt->addChannel(c);
	m_channelPlotcomboMap.insert(c, new PlotManagerCombobox(this, c));
	c->addChannelToPlot();
}

void PlotManager::removeChannel(ChannelComponent *c)
{
	c->removeChannelFromPlot();
	c->plotChannelCmpt()->plotComponent()->removeChannel(c);
	m_channels.removeAll(c->plotChannelCmpt());
	m_channelPlotcomboMap.remove(c);
}

void PlotManager::moveChannel(ChannelComponent *c, uint32_t uuid)
{
	c->removeChannelFromPlot();
	c->plotChannelCmpt()->plotComponent()->removeChannel(c);
	PlotComponent *plt = plot(uuid);
	c->plotChannelCmpt()->initPlotComponent(plt);
	c->addChannelToPlot();
	plt->addChannel(c);
	plt->replot();
}

PlotComponent *PlotManager::plot(uint32_t uuid)
{
	PlotComponent *plt = nullptr;
	for(PlotComponent *p : qAsConst(m_plots)) {
		if(p->uuid() == uuid) {
			plt = p;
		}
	}
	return plt;
}

QList<PlotComponent *> PlotManager::plots() const { return m_plots; }

void PlotManager::replot()
{
	for(PlotComponent *p : m_plots) {
		p->replot();
	}
	Q_EMIT newData();
}

#include "moc_plotmanager.cpp"
