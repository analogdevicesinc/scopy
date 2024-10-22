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

#include "plotchannel.h"
#include "menuplotchannelcurvestylecontrol.h"
#include <menusectionwidget.h>
#include <menucollapsesection.h>

using namespace scopy::gui;

MenuPlotChannelCurveStyleControl::MenuPlotChannelCurveStyleControl(QWidget *parent)
	: QWidget(parent)
{
	createCurveMenu(this);
}

MenuPlotChannelCurveStyleControl::~MenuPlotChannelCurveStyleControl() {}

void MenuPlotChannelCurveStyleControl::createCurveMenu(QWidget *parent)
{
	QHBoxLayout *curveSettingsLay = new QHBoxLayout(this);
	curveSettingsLay->setMargin(0);
	curveSettingsLay->setSpacing(10);
	setLayout(curveSettingsLay);

	cbThicknessW = new MenuCombo("Thickness", this);
	QComboBox *cbThickness = cbThicknessW->combo();
	cbThickness->addItem("1");
	cbThickness->addItem("2");
	cbThickness->addItem("3");
	cbThickness->addItem("4");
	cbThickness->addItem("5");

	connect(cbThickness, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		for(auto ch : qAsConst(m_channels)) {
			ch->setThickness(cbThickness->itemText(idx).toFloat());
		}
	});

	cbStyleW = new MenuCombo("Style", this);
	auto cbStyle = cbStyleW->combo();
	cbStyle->addItem("Lines", PlotChannel::PCS_LINES);
	cbStyle->addItem("Dots", PlotChannel::PCS_DOTS);
	cbStyle->addItem("Steps", PlotChannel::PCS_STEPS);
	cbStyle->addItem("Sticks", PlotChannel::PCS_STICKS);
	cbStyle->addItem("Smooth", PlotChannel::PCS_SMOOTH);
	StyleHelper::MenuComboBox(cbStyle, "cbStyle");

	connect(cbStyle, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		for(auto ch : qAsConst(m_channels)) {
			ch->setStyle(cbStyle->itemData(idx).toInt());
		}
	});

	curveSettingsLay->addWidget(cbThicknessW);
	curveSettingsLay->addWidget(cbStyleW);
}

void MenuPlotChannelCurveStyleControl::addChannels(PlotChannel *c)
{
	c->setThickness(cbThicknessW->combo()->currentText().toInt());
	c->setStyle(cbStyleW->combo()->currentIndex());

	connect(c, &PlotChannel::styleChanged, this, &MenuPlotChannelCurveStyleControl::setStyleSlot);
	connect(c, &PlotChannel::thicknessChanged, this, &MenuPlotChannelCurveStyleControl::setThicknessSlot);

	m_channels.append(c);
}

void MenuPlotChannelCurveStyleControl::removeChannels(PlotChannel *c)
{
	disconnect(c, &PlotChannel::styleChanged, this, &MenuPlotChannelCurveStyleControl::setStyleSlot);
	disconnect(c, &PlotChannel::thicknessChanged, this, &MenuPlotChannelCurveStyleControl::setThicknessSlot);
	m_channels.removeAll(c);
}

void MenuPlotChannelCurveStyleControl::setStyleSlot()
{
	if(m_channels.count() <= 0)
		return;

	int style = m_channels[0]->style();
	for(PlotChannel *c : qAsConst(m_channels)) {
		if(style != c->style()) {
			// "Mixed style should be written here"
			return;
		}
	}

	cbStyleW->combo()->setCurrentIndex(cbStyleW->combo()->findData(style));
}

void MenuPlotChannelCurveStyleControl::setThicknessSlot()
{
	if(m_channels.count() <= 0)
		return;

	int thickness = m_channels[0]->thickness();
	for(PlotChannel *c : qAsConst(m_channels)) {
		if(thickness != c->thickness()) {
			// "Mixed thickness should be written here"
			return;
		}
	}

	cbThicknessW->combo()->setCurrentText(QString::number(thickness));
}

#include "moc_menuplotchannelcurvestylecontrol.cpp"
