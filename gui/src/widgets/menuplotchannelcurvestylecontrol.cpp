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
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

using namespace scopy::gui;

MenuPlotChannelCurveStyleControl::MenuPlotChannelCurveStyleControl(QWidget *parent)
	: QWidget(parent)
{
	createCurveMenu(this);
}

MenuPlotChannelCurveStyleControl::~MenuPlotChannelCurveStyleControl() {}

void MenuPlotChannelCurveStyleControl::createCurveMenu(QWidget *parent)
{
	QVBoxLayout *mainLay = new QVBoxLayout(this);
	mainLay->setMargin(0);
	mainLay->setSpacing(10);

	m_cbChannel = new MenuCombo("Curve", this);
	mainLay->addWidget(m_cbChannel);

	QHBoxLayout *styleLay = new QHBoxLayout();
	styleLay->setMargin(0);
	styleLay->setSpacing(10);

	m_cbThickness = new MenuCombo("Thickness", this);
	m_cbThickness->combo()->addItem("1", 1);
	m_cbThickness->combo()->addItem("2", 2);
	m_cbThickness->combo()->addItem("3", 3);
	m_cbThickness->combo()->addItem("4", 4);
	m_cbThickness->combo()->addItem("5", 5);

	m_cbStyle = new MenuCombo("Style", this);
	m_cbStyle->combo()->addItem("Lines", PlotChannel::PCS_LINES);
	m_cbStyle->combo()->addItem("Dots", PlotChannel::PCS_DOTS);
	m_cbStyle->combo()->addItem("Steps", PlotChannel::PCS_STEPS);
	m_cbStyle->combo()->addItem("Sticks", PlotChannel::PCS_STICKS);
	m_cbStyle->combo()->addItem("Smooth", PlotChannel::PCS_SMOOTH);

	styleLay->addWidget(m_cbThickness);
	styleLay->addWidget(m_cbStyle);
	mainLay->addLayout(styleLay);

	QHBoxLayout *alphaLay = new QHBoxLayout();
	alphaLay->setMargin(0);
	alphaLay->setSpacing(10);
	QLabel *alphaLabel = new QLabel("Opacity", this);
	m_alphaSlider = new QSlider(Qt::Horizontal, this);
	m_alphaSlider->setRange(0, 255);
	m_alphaSlider->setValue(255);
	alphaLay->addWidget(alphaLabel);
	alphaLay->addWidget(m_alphaSlider);
	mainLay->addLayout(alphaLay);

	connect(m_cbChannel->combo(), qOverload<int>(&QComboBox::currentIndexChanged), this,
		&MenuPlotChannelCurveStyleControl::onChannelSelected);
	connect(m_cbThickness->combo(), qOverload<int>(&QComboBox::currentIndexChanged), this,
		&MenuPlotChannelCurveStyleControl::onThicknessChanged);
	connect(m_cbStyle->combo(), qOverload<int>(&QComboBox::currentIndexChanged), this,
		&MenuPlotChannelCurveStyleControl::onStyleChanged);
	connect(m_alphaSlider, &QSlider::valueChanged, this, &MenuPlotChannelCurveStyleControl::onAlphaChanged);
}

scopy::PlotChannel *MenuPlotChannelCurveStyleControl::currentChannel() const
{
	int idx = m_cbChannel->combo()->currentIndex();
	if(idx >= 0 && idx < m_channels.size())
		return m_channels.at(idx);
	return nullptr;
}

void MenuPlotChannelCurveStyleControl::addChannels(PlotChannel *c)
{
	if(m_channels.contains(c))
		return;

	m_channels.append(c);
	m_cbChannel->combo()->addItem(c->name(), QVariant::fromValue(c));

	if(m_channels.size() == 1) {
		updateControlsFromChannel();
	}
}

void MenuPlotChannelCurveStyleControl::removeChannels(PlotChannel *c)
{
	int idx = m_channels.indexOf(c);
	if(idx >= 0) {
		m_channels.removeAt(idx);
		m_cbChannel->combo()->removeItem(idx);
	}
}

void MenuPlotChannelCurveStyleControl::onChannelSelected(int index)
{
	Q_UNUSED(index);
	updateControlsFromChannel();
}

void MenuPlotChannelCurveStyleControl::updateControlsFromChannel()
{
	PlotChannel *ch = currentChannel();
	if(!ch)
		return;

	QSignalBlocker b1(m_cbThickness->combo());
	QSignalBlocker b2(m_cbStyle->combo());
	QSignalBlocker b3(m_alphaSlider);

	int thicknessIdx = m_cbThickness->combo()->findData(ch->thickness());
	if(thicknessIdx >= 0)
		m_cbThickness->combo()->setCurrentIndex(thicknessIdx);

	int styleIdx = m_cbStyle->combo()->findData(ch->style());
	if(styleIdx >= 0)
		m_cbStyle->combo()->setCurrentIndex(styleIdx);

	m_alphaSlider->setValue(ch->pen().color().alpha());
}

void MenuPlotChannelCurveStyleControl::onThicknessChanged(int index)
{
	PlotChannel *ch = currentChannel();
	if(!ch)
		return;
	ch->setThickness(m_cbThickness->combo()->itemData(index).toInt());
}

void MenuPlotChannelCurveStyleControl::onStyleChanged(int index)
{
	PlotChannel *ch = currentChannel();
	if(!ch)
		return;
	ch->setStyle(m_cbStyle->combo()->itemData(index).toInt());
}

void MenuPlotChannelCurveStyleControl::onAlphaChanged(int value)
{
	PlotChannel *ch = currentChannel();
	if(!ch)
		return;
	QColor color = ch->pen().color();
	color.setAlpha(value);
	ch->setColor(color);
}

#include "moc_menuplotchannelcurvestylecontrol.cpp"
