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

#include "ad74413r/channelplotscale.h"

#include <QVBoxLayout>

using namespace scopy::swiotrefactor;

ChannelPlotScale::ChannelPlotScale(int channel, QString unit, QColor color, QWidget *parent)
	: QWidget(parent)
	, m_channel(channel)
	, m_unit(unit)
	, m_enabled(true)
	, m_channelColor(color)
{
	m_formatter = new MetricPrefixFormatter();
	auto layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(5);
	this->setLayout(layout);

	m_unitPerDivisionLbl = new QLabel(this);
	m_instantValueLbl = new QLabel(this);
	m_unitPerDivisionLbl->setVisible(true);
	m_instantValueLbl->setVisible(true);
	layout->addWidget(m_unitPerDivisionLbl);
	layout->addWidget(m_instantValueLbl);

	connect(this, &ChannelPlotScale::requestUpdate, this, &ChannelPlotScale::update);
	m_unitPerDivisionLbl->setStyleSheet(QString("QLabel {"
						    "color: %1;"
						    "font-weight: bold;"
						    "}")
						    .arg(m_channelColor.name()));
	m_instantValueLbl->setStyleSheet(QString("QLabel {"
						 "color: %1;"
						 "font-weight: bold;"
						 "}")
						 .arg(m_channelColor.name()));
	setInstantValue(0);
	setUnitPerDivision(0);
}

ChannelPlotScale::~ChannelPlotScale() { delete m_formatter; }

void ChannelPlotScale::setInstantValue(double value)
{
	if(qFuzzyCompare(m_instantValue, value))
		return;
	m_instantValue = value;
	Q_EMIT requestUpdate();
}

void ChannelPlotScale::setUnitPerDivision(double value)
{
	if(qFuzzyCompare(m_unitPerDivision, value))
		return;
	m_unitPerDivision = value;
	Q_EMIT requestUpdate();
}

void ChannelPlotScale::update()
{
	m_unitPerDivisionLbl->setText(m_formatter->format(m_unitPerDivision, m_unit + "/div", 2));
	m_instantValueLbl->setText(m_formatter->format(m_instantValue, m_unit, 2));
}

int ChannelPlotScale::getChannelId() { return m_channel; }

bool ChannelPlotScale::getEnabled() { return m_enabled; }

void ChannelPlotScale::setEnabled(bool en) { m_enabled = en; }
