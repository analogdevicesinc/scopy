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

#include <widgets/menucollapsesection.h>
#include <widgets/menusectionwidget.h>
#include <widgets/menuplotaxisrangecontrol.h>
#include <pluginbase/preferences.h>
#include <qwt_point_data.h>
#include <QLineEdit>
#include "channelcomponent.h"

using namespace scopy;
using namespace adc;

FFTPlotComponent::FFTPlotComponent(QString name, uint32_t uuid, QWidget *parent)
	: PlotComponent(name, uuid, parent)
{
	m_fftPlot = new PlotWidget(this);

	m_fftPlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_fftPlot->xAxis()->setInterval(0, 1);
	m_fftPlot->xAxis()->setVisible(true);
	m_fftPlot->yAxis()->setUnits("dB");

	m_plots.append(m_fftPlot);
	m_plotLayout->addWidget(m_fftPlot);

	auto nameLbl = m_fftPlot->getPlotInfo()->addLabelInfo(IP_LEFT, IP_TOP);
	nameLbl->setText(m_name);
	connect(this, &PlotComponent::nameChanged, nameLbl, &QLabel::setText);

	m_fftInfo = new FFTSamplingInfo(m_fftPlot);
	m_fftPlot->getPlotInfo()->addCustomInfo(m_fftInfo, IP_RIGHT);

	auto m_timeStampInfo = new TimestampInfo(m_fftPlot, m_fftPlot);
	m_fftPlot->getPlotInfo()->addCustomInfo(m_timeStampInfo, IP_RIGHT);

	m_plotMenu = new FFTPlotComponentSettings(this, parent);
	addComponent(m_plotMenu);

	connect(m_plotMenu, &FFTPlotComponentSettings::requestDeletePlot, this, [=]() { Q_EMIT requestDeletePlot(); });
	m_cursor = new CursorController(m_fftPlot, this);
}

FFTPlotComponent::~FFTPlotComponent() {}

PlotWidget *FFTPlotComponent::fftPlot() { return m_plots[0]; }

void FFTPlotComponent::addChannel(ChannelComponent *c)
{
	PlotComponent::addChannel(c);
	m_plotMenu->addChannel(c);
}

void FFTPlotComponent::removeChannel(ChannelComponent *c)
{
	PlotComponent::removeChannel(c);
	m_plotMenu->removeChannel(c);
}

FFTPlotComponentSettings *FFTPlotComponent::createPlotMenu(QWidget *parent) { return m_plotMenu; }

FFTPlotComponentSettings *FFTPlotComponent::plotMenu() { return m_plotMenu; }

FFTSamplingInfo *FFTPlotComponent::fftPlotInfo() const { return m_fftInfo; }

#include "moc_fftplotcomponent.cpp"
