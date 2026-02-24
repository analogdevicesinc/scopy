/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#include "plotwidget_api.h"

#include <plotwidget.h>
#include <plotaxis.h>
#include <plotchannel.h>
#include <plotscales.h>
#include <plotnavigator.hpp>

using namespace scopy;

PlotWidget_API::PlotWidget_API(PlotWidget *plot, QObject *parent)
	: ApiObject(parent)
	, m_plot(plot)
{}

PlotWidget_API::~PlotWidget_API() {}

// X-Axis

double PlotWidget_API::getXMin() { return m_plot->xAxis()->min(); }

double PlotWidget_API::getXMax() { return m_plot->xAxis()->max(); }

void PlotWidget_API::setXMin(double val) { m_plot->xAxis()->setMin(val); }

void PlotWidget_API::setXMax(double val) { m_plot->xAxis()->setMax(val); }

void PlotWidget_API::setXInterval(double min, double max) { m_plot->xAxis()->setInterval(min, max); }

QString PlotWidget_API::getXUnits() { return m_plot->xAxis()->getUnits(); }

void PlotWidget_API::setXUnits(const QString &units) { m_plot->xAxis()->setUnits(units); }

double PlotWidget_API::getXDivs() { return m_plot->xAxis()->divs(); }

void PlotWidget_API::setXDivs(double divs) { m_plot->xAxis()->setDivs(divs); }

// Y-Axis

double PlotWidget_API::getYMin() { return m_plot->yAxis()->min(); }

double PlotWidget_API::getYMax() { return m_plot->yAxis()->max(); }

void PlotWidget_API::setYMin(double val) { m_plot->yAxis()->setMin(val); }

void PlotWidget_API::setYMax(double val) { m_plot->yAxis()->setMax(val); }

void PlotWidget_API::setYInterval(double min, double max) { m_plot->yAxis()->setInterval(min, max); }

QString PlotWidget_API::getYUnits() { return m_plot->yAxis()->getUnits(); }

void PlotWidget_API::setYUnits(const QString &units) { m_plot->yAxis()->setUnits(units); }

double PlotWidget_API::getYDivs() { return m_plot->yAxis()->divs(); }

void PlotWidget_API::setYDivs(double divs) { m_plot->yAxis()->setDivs(divs); }

// Channels

QStringList PlotWidget_API::getChannelNames()
{
	QStringList names;
	for(auto *ch : m_plot->getChannels()) {
		names.append(ch->name());
	}
	return names;
}

void PlotWidget_API::setChannelEnabled(const QString &name, bool enabled)
{
	PlotChannel *ch = findChannel(name);
	if(ch) {
		ch->setEnabled(enabled);
	}
}

bool PlotWidget_API::isChannelEnabled(const QString &name)
{
	PlotChannel *ch = findChannel(name);
	return ch ? ch->isEnabled() : false;
}

void PlotWidget_API::setChannelThickness(const QString &name, int thickness)
{
	PlotChannel *ch = findChannel(name);
	if(ch) {
		ch->setThickness(thickness);
	}
}

int PlotWidget_API::getChannelThickness(const QString &name)
{
	PlotChannel *ch = findChannel(name);
	return ch ? ch->thickness() : 0;
}

void PlotWidget_API::setChannelStyle(const QString &name, int style)
{
	PlotChannel *ch = findChannel(name);
	if(ch) {
		ch->setStyle(style);
	}
}

int PlotWidget_API::getChannelStyle(const QString &name)
{
	PlotChannel *ch = findChannel(name);
	return ch ? ch->style() : 0;
}

void PlotWidget_API::selectChannel(const QString &name)
{
	PlotChannel *ch = findChannel(name);
	if(ch) {
		m_plot->selectChannel(ch);
	}
}

// Grid/Scales

void PlotWidget_API::setGridEnabled(bool enabled) { m_plot->scales()->setGridEn(enabled); }

bool PlotWidget_API::isGridEnabled() { return m_plot->scales()->getGridEn(); }

void PlotWidget_API::setGraticuleEnabled(bool enabled) { m_plot->scales()->setGraticuleEn(enabled); }

bool PlotWidget_API::isGraticuleEnabled() { return m_plot->scales()->getGraticuleEn(); }

// Labels

void PlotWidget_API::setShowXAxisLabels(bool show) { m_plot->setShowXAxisLabels(show); }

void PlotWidget_API::setShowYAxisLabels(bool show) { m_plot->setShowYAxisLabels(show); }

// Navigator

void PlotWidget_API::setNavigatorEnabled(bool enabled) { m_plot->navigator()->setEnabled(enabled); }

bool PlotWidget_API::isNavigatorEnabled() { return m_plot->navigator()->isEnabled(); }

// General

void PlotWidget_API::replot() { m_plot->replot(); }

QString PlotWidget_API::exportCsv() { return m_plot->generateCsvData(); }

// Private helper

PlotChannel *PlotWidget_API::findChannel(const QString &name)
{
	for(auto *ch : m_plot->getChannels()) {
		if(ch->name() == name) {
			return ch;
		}
	}
	return nullptr;
}

#include "moc_plotwidget_api.cpp"
