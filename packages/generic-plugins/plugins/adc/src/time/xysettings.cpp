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

#include "xysettings.h"
#include <style.h>
#include <infoiconwidget.h>

using namespace scopy;
using namespace scopy::adc;

XYSettings::XYSettings(QWidget *parent)
	: QWidget(parent)
{
	initUI();
}

XYSettings::~XYSettings() {}

void XYSettings::initUI()
{
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	setFixedWidth(250);
	m_layout = new QVBoxLayout(this);
	m_layout->setMargin(0);
	setLayout(m_layout);

	m_controls = new MenuSectionWidget(this);
	Style::setStyle(m_controls, style::properties::widget::border);
	m_layout->addWidget(m_controls);
	m_controls->contentLayout()->setSpacing(10);

	m_xAxisSrc = new MenuCombo("X Axis source", m_controls);
	InfoIconWidget::addHoveringInfoToWidget(
		m_xAxisSrc->label(), "Selects which channel provides the X-axis values for the XY plot", m_xAxisSrc);
	m_controls->contentLayout()->addWidget(m_xAxisSrc);

	connect(m_xAxisSrc->combo(), qOverload<int>(&QComboBox::currentIndexChanged), this,
		&XYSettings::xAxisSourceChanged);

	m_xAxisShow = new MenuOnOffSwitch("Plot X source", m_controls, false);
	InfoIconWidget::addHoveringInfoToWidget(m_xAxisShow->label(),
						"Plots selected channel by itself over the current XY plot", m_xAxisShow);
	m_controls->contentLayout()->addWidget(m_xAxisShow);

	connect(m_xAxisShow->onOffswitch(), &QAbstractButton::toggled, this, &XYSettings::showXSourceToggled);
}

MenuCombo *XYSettings::xAxisSource() { return m_xAxisSrc; }

QAbstractButton *XYSettings::showXSource() { return m_xAxisShow->onOffswitch(); }

#include "moc_xysettings.cpp"
