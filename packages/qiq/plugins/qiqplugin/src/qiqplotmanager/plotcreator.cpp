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

#include "qiqplotmanager/plotcreator.h"
#include <plotaxis.h>
#include <stylehelper.h>
#include <pluginbase/preferences.h>

using namespace scopy::qiqplugin;

StandardPlotCreator::StandardPlotCreator(QObject *parent)
	: PlotCreatorBase(parent)
{}

QWidget *StandardPlotCreator::createPlot(const QIQPlotInfo &plotInfo)
{
	auto plot = new PlotWidget();
	bool useDock = Preferences::get("general_use_docking_if_available").toBool();
	if(!useDock) {
		plot->plot()->setTitle(plotInfo.title);
	}
	setupPlotChannels(plot, plotInfo);
	configurePlotAxis(plot, plotInfo);
	applyPlotFlags(plot, plotInfo);
	plot->replot();

	return plot;
}

void StandardPlotCreator::setupPlotChannels(PlotWidget *plot, const QIQPlotInfo &plotInfo)
{
	int i = 0;
	for(const auto &plotCh : plotInfo.channels) {
		QString chId = "ch" + QString::number(i);
		QPen pen(StyleHelper::getChannelColor(i));
		auto ch = new PlotChannel(chId, pen, plot->xAxis(), plot->yAxis(), plot);
		plot->addPlotChannel(ch);
		ch->setEnabled(true);

		// Apply channel-specific flags
		if(plotInfo.flags.contains("points")) {
			ch->setThickness(3);
			ch->setStyle(PlotChannel::PlotCurveStyle::PCS_DOTS);
		}
		if(plotInfo.flags.contains("sticks")) {
			ch->setStyle(PlotChannel::PlotCurveStyle::PCS_STICKS);
		}
		i++;
	}
}

void StandardPlotCreator::configurePlotAxis(PlotWidget *plot, const QIQPlotInfo &plotInfo)
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

void StandardPlotCreator::applyPlotFlags(PlotWidget *plot, const QIQPlotInfo &plotInfo)
{
	// Handle labels flag
	bool showLabels = plotInfo.flags.contains("labels");
	plot->setShowXAxisLabels(showLabels);
	plot->setShowYAxisLabels(showLabels);
	plot->showAxisLabels();
}

void StandardPlotCreator::updatePlot(QWidget *plotWidget, const QIQPlotInfo &plotInfo,
				     const QMap<QString, QVector<double>> &data)
{
	auto plot = qobject_cast<PlotWidget *>(plotWidget);
	if(!plot) {
		return;
	}

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

PlotCreatorBase *PlotCreatorFactory::createPlotCreator(const QIQPlotInfo &plotInfo)
{
	// This is where the logic needs to be put when multiple plots will be used.
	// Default to standard plot.
	return new StandardPlotCreator();
}

QString PlotCreatorFactory::determinePlotType(const QIQPlotInfo &plotInfo)
{
	// Logic to determine plot type based on plotInfo
	if(plotInfo.flags.contains("spectrogram")) {
		return "spectrogram";
	}

	return "plotWidget";
}
