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

#include "fftplotcomponent.h"
#include "plotaxis.h"
#include "plotnavigator.hpp"

#include <gui/style.h>
#include <gui/style_attributes.h>
#include <gui/docking/dockablearea.h>
#include <gui/docking/dockwrapper.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menuplotaxisrangecontrol.h>
#include <gui/widgets/plotlegend.h>

#include <pluginbase/preferences.h>
#include <qwt_point_data.h>
#include <QLineEdit>
#include "channelcomponent.h"

using namespace scopy;
using namespace adc;

FFTPlotComponent::FFTPlotComponent(QString name, uint32_t uuid, QWidget *parent)
	: PlotComponent(name, uuid, parent)
	, m_activeChannel(nullptr)
{
	m_dockableArea = createDockableArea(this);
	QWidget *dockableAreaWidget = m_dockableArea->asWidget();
	Style::setBackgroundColor(dockableAreaWidget, json::theme::background_subtle, true);
	m_plotLayout->addWidget(dockableAreaWidget);

	m_fftDockWrapper = createDockWrapper("FFT Plot");
	m_fftPlot = new PlotWidget(this);
	m_fftPlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_fftPlot->xAxis()->setInterval(0, 1);
	m_fftPlot->xAxis()->setVisible(true);
	m_fftPlot->yAxis()->setUnits("dBFS");
	m_fftDockWrapper->setInnerWidget(m_fftPlot);

	// Waterfall plot in its own dock, named after the active channel
	m_waterfallPlot = new WaterfallPlotWidget(this);
	m_waterfallPlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_waterfallDockWrapper = createDockWrapper("Waterfall Plot");
	m_waterfallDockWrapper->setInnerWidget(m_waterfallPlot);

	m_plots.append(m_fftPlot);
	m_plots.append(m_waterfallPlot);

	auto nameLbl = m_fftPlot->getPlotInfo()->addLabelInfo(IP_LEFT, IP_TOP);
	nameLbl->setText(m_name);
	connect(this, &PlotComponent::nameChanged, nameLbl, &QLabel::setText);

	m_fftInfo = new FFTSamplingInfo(m_fftPlot);
	m_fftPlot->getPlotInfo()->addCustomInfo(m_fftInfo, IP_RIGHT);

	auto m_timeStampInfo = new TimestampInfo(m_fftPlot, m_fftPlot);
	m_fftPlot->getPlotInfo()->addCustomInfo(m_timeStampInfo, IP_RIGHT);

	m_legend = new PlotLegend(m_fftPlot, this);
	m_legend->hide();

	m_plotMenu = new FFTPlotComponentSettings(this, parent);
	addComponent(m_plotMenu);
	m_dockableArea->addDockWrapper(m_fftDockWrapper, DockableAreaInterface::Direction_TOP);
	m_dockableArea->addDockWrapper(m_waterfallDockWrapper, DockableAreaInterface::Direction_BOTTOM);

	connect(m_plotMenu, &FFTPlotComponentSettings::requestDeletePlot, this, [=]() { Q_EMIT requestDeletePlot(); });
	m_cursor = new CursorController(m_fftPlot, this);
	int xCursorPos = Preferences::get("adc_plot_xcursor_position").toInt();
	int yCursorPos = Preferences::get("adc_plot_ycursor_position").toInt();
	m_cursor->getPlotCursors()->setXHandlePos((HandlePos)xCursorPos);
	m_cursor->getPlotCursors()->setYHandlePos((HandlePos)yCursorPos);

	m_waterfallCursor = new CursorController(m_waterfallPlot, this);
	m_waterfallCursor->getPlotCursors()->setXHandlePos((HandlePos)xCursorPos);
	m_waterfallCursor->getPlotCursors()->setYHandlePos((HandlePos)yCursorPos);
	m_waterfallCursor->setAxes(m_waterfallPlot->xAxis(), m_waterfallPlot->yAxis());

	CursorController::syncXCursorControllers(m_cursor, m_waterfallCursor);
	PlotNavigator::syncXNavigators(m_fftPlot->navigator(), m_waterfallPlot->navigator());
}

FFTPlotComponent::~FFTPlotComponent() {}

PlotWidget *FFTPlotComponent::fftPlot() { return m_plots[0]; }

WaterfallPlotWidget *FFTPlotComponent::waterfallPlot() { return m_waterfallPlot; }

DockWrapperInterface *FFTPlotComponent::waterfallDockWrapper() { return m_waterfallDockWrapper; }

ChannelComponent *FFTPlotComponent::activeChannel() const { return m_activeChannel; }

void FFTPlotComponent::selectChannel(ChannelComponent *ch)
{
	// Only forward channel selection to the FFT plot — the waterfall has no PlotChannel objects.
	// Calling PlotComponent::selectChannel would set m_selectedChannel on the waterfall to an
	// FFT plot channel, breaking showAxisLabels() which branches on m_selectedChannel != nullptr.
	if(ch && ch->plotChannelCmpt())
		m_fftPlot->selectChannel(ch->plotChannelCmpt()->plotChannel());

	if(m_activeChannel)
		disconnect(m_activeChannel, &ChannelComponent::nameChanged, nullptr, nullptr);

	m_activeChannel = ch;
	m_waterfallPlot->setChannel(ch ? ch->chData() : nullptr);

	if(ch) {
		m_waterfallDockWrapper->setTitle("Waterfall Plot - " + ch->name());
		connect(ch, &ChannelComponent::nameChanged, this,
			[=](const QString &name) { m_waterfallDockWrapper->setTitle("Waterfall Plot - " + name); });
	}
}

void FFTPlotComponent::addChannel(ChannelComponent *c)
{
	PlotComponent::addChannel(c);
	m_plotMenu->addChannel(c);
}

void FFTPlotComponent::removeChannel(ChannelComponent *c)
{
	if(c == m_activeChannel)
		selectChannel(nullptr);
	PlotComponent::removeChannel(c);
	m_plotMenu->removeChannel(c);
}

FFTPlotComponentSettings *FFTPlotComponent::createPlotMenu(QWidget *parent) { return m_plotMenu; }

FFTPlotComponentSettings *FFTPlotComponent::plotMenu() { return m_plotMenu; }

CursorController *FFTPlotComponent::waterfallCursor() const { return m_waterfallCursor; }

FFTSamplingInfo *FFTPlotComponent::fftPlotInfo() const { return m_fftInfo; }

PlotLegend *FFTPlotComponent::legend() const { return m_legend; }

#include "moc_fftplotcomponent.cpp"
