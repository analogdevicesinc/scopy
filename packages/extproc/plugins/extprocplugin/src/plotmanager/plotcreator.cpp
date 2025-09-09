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

#include "plotmanager/plotcreator.h"
#include "menuplotchannelcurvestylecontrol.h"
#include <menuonoffswitch.h>
#include <menuplotaxisrangecontrol.h>
#include <menusectionwidget.h>
#include <plotaxis.h>
#include <style.h>
#include <stylehelper.h>
#include <pluginbase/preferences.h>

using namespace scopy::gui;
using namespace scopy::extprocplugin;

StandardPlotCreator::StandardPlotCreator(QObject *parent)
	: PlotCreatorBase(parent)
{}

QWidget *StandardPlotCreator::createPlot(const ExtProcPlotInfo &plotInfo)
{
	auto plot = new PlotWidget();
	bool useDock = Preferences::get("general_use_docking_if_available").toBool();
	if(!useDock) {
		plot->plot()->setTitle(plotInfo.title);
	}

	setupPlotChannels(plot, plotInfo);
	configurePlotAxis(plot, plotInfo);
	applyPlotFlags(plot, plotInfo);
	createPlotSettings(plot, plotInfo);

	QPushButton *settingsPlotHover = new QPushButton("");
	settingsPlotHover->setMaximumSize(16, 16);
	settingsPlotHover->setIcon(
		QIcon(":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) + "/icons/preferences.svg"));

	connect(settingsPlotHover, &QAbstractButton::clicked, this,
		[this, plotInfo]() { Q_EMIT requestSettings(plotInfo.title); });

	QWidget *paddingW = new QWidget();
	paddingW->setVisible(false);

	plot->plotButtonManager()->add(settingsPlotHover);
	plot->plotButtonManager()->add(paddingW);
	plot->plotButtonManager()->setVisible(true);

	plot->replot();

	return plot;
}

QWidget *StandardPlotCreator::settingsMenu() { return m_plotSettings; }

void StandardPlotCreator::setupPlotChannels(PlotWidget *plot, const ExtProcPlotInfo &plotInfo)
{
	int i = 0;
	for(const auto &plotCh : plotInfo.channels) {
		QString chId = "ch" + QString::number(i);
		QPen pen(StyleHelper::getChannelColor(i));
		auto ch = new PlotChannel(chId, pen, plot->xAxis(), plot->yAxis(), plot);
		plot->addPlotChannel(ch);
		ch->setEnabled(true);
		i++;
	}
}

void StandardPlotCreator::configurePlotAxis(PlotWidget *plot, const ExtProcPlotInfo &plotInfo)
{
	// X Axis configuration
	plot->xAxis()->scaleDraw()->setFormatter(new MetricPrefixFormatter());
	plot->xAxis()->scaleDraw()->setFloatPrecision(2);
	if(plotInfo.flags.contains("points")) {
		plot->xAxis()->setInterval(-200, 200);
	}

	// Y Axis configuration
	plot->yAxis()->scaleDraw()->setFormatter(new MetricPrefixFormatter());
	plot->yAxis()->scaleDraw()->setFloatPrecision(2);
	plot->yAxis()->setInterval(-200, 200);
}

void StandardPlotCreator::applyPlotFlags(PlotWidget *plot, const ExtProcPlotInfo &plotInfo)
{
	// Handle labels flag
	bool showLabels = plotInfo.flags.contains("labels");
	plot->setShowXAxisLabels(showLabels);
	plot->setShowYAxisLabels(showLabels);
	plot->showAxisLabels();
}

void StandardPlotCreator::createPlotSettings(PlotWidget *plot, const ExtProcPlotInfo &plotInfo)
{
	m_plotSettings = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(m_plotSettings);
	lay->setMargin(0);

	MenuSectionCollapseWidget *yAxis = new MenuSectionCollapseWidget(
		"Y-AXIS", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, m_plotSettings);
	MenuPlotAxisRangeControl *yCtrl = new MenuPlotAxisRangeControl(plot->yAxis(), yAxis);
	yCtrl->setMin(plot->yAxis()->min());
	yCtrl->setMax(plot->yAxis()->max());
	yAxis->add(yCtrl);

	MenuSectionCollapseWidget *plotMenu = new MenuSectionCollapseWidget(
		"SETTINGS", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, m_plotSettings);

	MenuOnOffSwitch *labelsSwitch = new MenuOnOffSwitch("Show plot labels", plotMenu, false);
	labelsSwitch->onOffswitch()->setChecked(plot->showXAxisLabels() && plot->showYAxisLabels());
	connect(labelsSwitch->onOffswitch(), &QAbstractButton::toggled, this, [plot](bool en) {
		if(en) {
			plot->showAxisLabels();
		} else {
			plot->hideAxisLabels();
		}
	});

	MenuPlotChannelCurveStyleControl *curveControl = new MenuPlotChannelCurveStyleControl(plotMenu);
	const QList<PlotChannel *> channels = plot->getChannels();
	for(PlotChannel *ch : channels) {
		curveControl->addChannels(ch);
		if(plotInfo.flags.contains("points")) {
			ch->setThickness(3);
			ch->setStyle(PlotChannel::PlotCurveStyle::PCS_DOTS);
		}
	}

	plotMenu->add(labelsSwitch);
	plotMenu->add(curveControl);

	lay->addWidget(yAxis);
	lay->addWidget(plotMenu);
}

void StandardPlotCreator::updatePlotChannels(PlotWidget *plot, const ExtProcPlotInfo &plotInfo)
{
	const auto &channels = plotInfo.channels;
	auto plotChnls = plot->getChannels();
	if(channels.size() == plotChnls.size()) {
		return;
	}
	clearPlotChannels(plot);
	setupPlotChannels(plot, plotInfo);
}

void StandardPlotCreator::clearPlotChannels(PlotWidget *plot)
{
	const auto channels = plot->getChannels();
	for(auto ch : channels) {
		plot->removePlotChannel(ch);
		ch->deleteLater();
	}
	plot->replot();
}

void StandardPlotCreator::updatePlot(QWidget *plotWidget, const ExtProcPlotInfo &plotInfo,
				     const QMap<QString, QVector<double>> &data)
{
	auto plot = qobject_cast<PlotWidget *>(plotWidget);
	if(!plot) {
		return;
	}
	updatePlotChannels(plot, plotInfo);
	const auto &channels = plotInfo.channels;
	auto plotChnls = plot->getChannels();
	double xFirst = 0, xLast = 0;
	for(int chIdx = 0; chIdx < channels.size() && chIdx < plotChnls.size(); chIdx++) {
		const auto &ch = channels[chIdx];
		auto xData = data.value(ch.x);
		auto yData = data.value(ch.y);

		plotChnls[chIdx]->curve()->setSamples(xData, yData);

		if(xFirst == 0 && xLast == 0 && !xData.isEmpty()) {
			double delta = (xData.size() > 2) ? (xData.at(1) - xData.at(0)) : 0;
			xFirst = xData.first();
			xLast = xData.last() + delta;
		}
	}

	if(!plotInfo.flags.contains("points")) {
		plot->xAxis()->setInterval(xFirst, xLast);
	}
	plot->replot();
}

PlotCreatorBase *PlotCreatorFactory::createPlotCreator(const ExtProcPlotInfo &plotInfo)
{
	// This is where the logic needs to be put when multiple plots will be used.
	// Default to standard plot.
	return new StandardPlotCreator();
}

QString PlotCreatorFactory::determinePlotType(const ExtProcPlotInfo &plotInfo)
{
	// Logic to determine plot type based on plotInfo
	if(plotInfo.flags.contains("spectrogram")) {
		return "spectrogram";
	}

	return "plotWidget";
}
