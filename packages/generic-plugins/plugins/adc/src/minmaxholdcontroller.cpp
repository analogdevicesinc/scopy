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

#include "minmaxholdcontroller.hpp"
#include <freq/fftplotcomponentchannel.h>
#include <freq/fftplotcomponent.h>
#include <algorithm>
#include <cstring>

using namespace scopy;
using namespace adc;

MinMaxHoldController::MinMaxHoldController(FFTPlotComponentChannel *ch, QObject *parent)
	: QObject(parent)
	, m_ch(ch)
{}

MinMaxHoldController::~MinMaxHoldController() {}

void MinMaxHoldController::setEnabled(bool enabled)
{
	if(m_enabled == enabled)
		return;

	m_enabled = enabled;
	if(enabled) {
		createChannels();
	}
	updateVisibility();
	Q_EMIT enabledChanged(m_enabled);
}

bool MinMaxHoldController::enabled() const { return m_enabled; }

void MinMaxHoldController::setMinEnabled(bool enabled)
{
	if(m_minEnabled == enabled)
		return;

	m_minEnabled = enabled;
	if(enabled) {
		createChannels();
	}
	updateVisibility();
}

void MinMaxHoldController::setMaxEnabled(bool enabled)
{
	if(m_maxEnabled == enabled)
		return;

	m_maxEnabled = enabled;
	if(enabled) {
		createChannels();
	}
	updateVisibility();
}

bool MinMaxHoldController::minEnabled() const { return m_minEnabled; }

bool MinMaxHoldController::maxEnabled() const { return m_maxEnabled; }

void MinMaxHoldController::setMainChannelHidden(bool hidden)
{
	if(m_mainChannelHidden == hidden)
		return;

	m_mainChannelHidden = hidden;
	updateVisibility();
}

void MinMaxHoldController::resetMin()
{
	m_minData.clear();
	if(m_minCh) {
		ChannelData *chData = m_ch->m_ch->chData();
		m_minCh->setSamples(chData->xData(), chData->yData(), chData->size());
		m_ch->m_plotComponent->replot();
	}
}

void MinMaxHoldController::resetMax()
{
	m_maxData.clear();
	if(m_maxCh) {
		ChannelData *chData = m_ch->m_ch->chData();
		m_maxCh->setSamples(chData->xData(), chData->yData(), chData->size());
		m_ch->m_plotComponent->replot();
	}
}

void MinMaxHoldController::update(const float *xData, const float *yData, size_t size)
{
	if(size == 0 || !m_enabled)
		return;

	if(m_xData.size() != size) {
		m_xData.resize(size);
		m_minData.clear();
		m_maxData.clear();
	}
	std::memcpy(m_xData.data(), xData, size * sizeof(float));

	if(m_minEnabled) {
		updateMinHold(yData, size);
	}
	if(m_maxEnabled) {
		updateMaxHold(yData, size);
	}
}

void MinMaxHoldController::updateMinHold(const float *yData, size_t size)
{
	if(m_minData.size() != size) {
		m_minData.resize(size);
		std::memcpy(m_minData.data(), yData, size * sizeof(float));
	} else {
		for(size_t i = 0; i < size; i++) {
			m_minData[i] = std::min(m_minData[i], yData[i]);
		}
	}
	if(m_minCh) {
		m_minCh->setSamples(m_xData.data(), m_minData.data(), size, true);
	}
}

void MinMaxHoldController::updateMaxHold(const float *yData, size_t size)
{
	if(m_maxData.size() != size) {
		m_maxData.resize(size);
		std::memcpy(m_maxData.data(), yData, size * sizeof(float));
	} else {
		for(size_t i = 0; i < size; i++) {
			m_maxData[i] = std::max(m_maxData[i], yData[i]);
		}
	}
	if(m_maxCh) {
		m_maxCh->setSamples(m_xData.data(), m_maxData.data(), size, true);
	}
}

void MinMaxHoldController::onChannelEnabled() { updateVisibility(); }

void MinMaxHoldController::onChannelDisabled()
{
	if(m_minCh) {
		m_minCh->disable();
	}
	if(m_maxCh) {
		m_maxCh->disable();
	}
}

void MinMaxHoldController::onYAxisChanged(PlotAxis *yAxis)
{
	auto fftplot = m_ch->m_plotComponent->fftPlot();
	if(m_minCh) {
		fftplot->plotChannelChangeYAxis(m_minCh, yAxis);
	}
	if(m_maxCh) {
		fftplot->plotChannelChangeYAxis(m_maxCh, yAxis);
	}
}

void MinMaxHoldController::updateVisibility()
{
	bool channelEnabled = m_ch->m_enabled;
	if(!channelEnabled)
		return;

	// Main channel: hidden only if hold is enabled AND user chose to hide
	if(m_enabled && m_mainChannelHidden) {
		m_ch->m_fftPlotCh->disable();
	} else {
		m_ch->m_fftPlotCh->enable();
	}

	// Min/max channels: visible only if hold is enabled AND individually enabled
	if(m_minCh) {
		if(m_enabled && m_minEnabled) {
			m_minCh->enable();
		} else {
			m_minCh->disable();
		}
	}
	if(m_maxCh) {
		if(m_enabled && m_maxEnabled) {
			m_maxCh->enable();
		} else {
			m_maxCh->disable();
		}
	}

	m_ch->m_plotComponent->replot();
}

void MinMaxHoldController::createChannels()
{
	if(!m_ch->m_plotComponent)
		return;

	auto fftplot = m_ch->m_plotComponent->fftPlot();
	QColor baseColor = m_ch->m_ch->pen().color();
	PlotAxis *xAxis = m_ch->m_fftPlotCh->xAxis();
	PlotAxis *yAxis = m_ch->m_fftPlotCh->yAxis();
	uint minColorDarker = 170;
	uint maxColorLighter = 140;
	uint colorAlpha = 180;

	if(!m_minCh) {
		QColor minColor = baseColor.darker(minColorDarker);
		minColor.setAlpha(colorAlpha);
		m_minCh = new PlotChannel(m_ch->m_ch->name() + " Min", QPen(minColor), xAxis, yAxis, this);
		m_minCh->init();
		fftplot->addPlotChannel(m_minCh);
	}

	if(!m_maxCh) {
		QColor maxColor = baseColor.lighter(maxColorLighter);
		maxColor.setAlpha(colorAlpha);
		m_maxCh = new PlotChannel(m_ch->m_ch->name() + " Max", QPen(maxColor), xAxis, yAxis, this);
		m_maxCh->init();
		fftplot->addPlotChannel(m_maxCh);
	}
}

PlotChannel *MinMaxHoldController::minChannel() const { return m_minCh; }

PlotChannel *MinMaxHoldController::maxChannel() const { return m_maxCh; }
