/*
 * Copyright (c) 2023 Analog Devices Inc.
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
 */

#include "ad74413r/channelplotscalescontroller.h"

#include <QHBoxLayout>
#include <QSpacerItem>
using namespace scopy::swiotrefactor;

ChannelPlotScalesController::ChannelPlotScalesController(QWidget *parent)
	: QWidget(parent)
	, m_layout(nullptr)
{
	auto layout = new QHBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(10);
	layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));
	this->setLayout(m_layout);
}

ChannelPlotScalesController::~ChannelPlotScalesController()
{
	if(m_layout) {
		QLayoutItem *child;
		while((child = m_layout->takeAt(0)) != nullptr) {
			delete child->widget();
			delete child;
		}
		auto oldLayout = m_layout;
		oldLayout->deleteLater();
	}
}

void ChannelPlotScalesController::addChannel(int index, QColor color, QString unit, bool enabled)
{
	ChannelPlotScale *chnPlotScale = new ChannelPlotScale(index, unit, color, this);
	chnPlotScale->setEnabled(enabled);
	m_channelPlotScales.push_back(chnPlotScale);
	updateLayout();
}

void ChannelPlotScalesController::updateLayout()
{
	QHBoxLayout *mainLayout = (QHBoxLayout *)this->layout();
	if(m_layout) {
		QLayoutItem *child;
		while((child = m_layout->takeAt(0)) != nullptr) {
			delete child;
		}
		mainLayout->removeItem(m_layout);
		delete m_layout;
	}
	m_layout = new QHBoxLayout(this);
	m_layout->setMargin(0);
	m_layout->setSpacing(20);
	for(auto chnPlotScale : qAsConst(m_channelPlotScales)) {
		m_layout->addWidget(chnPlotScale);
	}
	mainLayout->insertLayout(0, m_layout);
}

void ChannelPlotScalesController::setChannelEnabled(int channel, bool enable)
{
	for(auto chn : qAsConst(m_channelPlotScales)) {
		if(chn->getChannelId() == channel) {
			chn->setEnabled(enable);
			chn->setVisible(enable);
			break;
		}
	}
	updateLayout();
}

void ChannelPlotScalesController::setUnitPerDivision(int channel, double unitPerDivision)
{
	for(auto chn : qAsConst(m_channelPlotScales)) {
		if(chn->getChannelId() == channel) {
			chn->setUnitPerDivision(unitPerDivision);
			break;
		}
	}
}

void ChannelPlotScalesController::setInstantValue(int channel, double value)
{
	for(auto chn : qAsConst(m_channelPlotScales)) {
		if(chn->getChannelId() == channel) {
			chn->setInstantValue(value);
			break;
		}
	}
}
