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

#include "calibrationswidget.h"

#include <QLabel>
#include <style.h>
#include <ad9371advutils.h>

using namespace scopy;
using namespace ad9371;

CalibrationsWidget::CalibrationsWidget(iio_device *device, IIOWidgetGroup *group, QWidget *parent)
	: m_device(device)
	, m_group(group)
	, QWidget{parent}
{
	Style::setBackgroundColor(this, json::theme::background_primary);
	m_layout = new QVBoxLayout(this);
	m_layout->setMargin(0);
	m_layout->setContentsMargins(0, 0, 0, 0);

	QWidget *widget = new QWidget(this);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setContentsMargins(5, 5, 5, 5);
	layout->setSpacing(10);
	Style::setStyle(widget, style::properties::widget::border_interactive);
	m_layout->addWidget(widget);

	QLabel *title = new QLabel("INIT Calibrations", widget);
	Style::setStyle(title, style::properties::label::menuBig);
	layout->addWidget(title);

	IIOWidget *w0 = makeDebugCheck(widget, m_device, "adi,init-cal-rx-qec-en", "RX QEC", m_group);
	layout->addWidget(w0);
	IIOWidget *w1 = makeDebugCheck(widget, m_device, "adi,init-cal-tx-qec-en", "TX QEC", m_group);
	layout->addWidget(w1);
	IIOWidget *w2 = makeDebugCheck(widget, m_device, "adi,init-cal-tx-lol-en", "TX LOL", m_group);
	layout->addWidget(w2);
	IIOWidget *w3 = makeDebugCheck(widget, m_device, "adi,init-cal-external-tx-lol-en", "External TX LOL", m_group);
	layout->addWidget(w3);
	IIOWidget *w4 = makeDebugCheck(widget, m_device, "adi,init-cal-rx-gain-delay-en", "RX Gain Delay", m_group);
	layout->addWidget(w4);
	IIOWidget *w5 = makeDebugCheck(widget, m_device, "adi,init-cal-tx-bb-filter-en", "TX BB Filter", m_group);
	layout->addWidget(w5);
	IIOWidget *w6 = makeDebugCheck(widget, m_device, "adi,init-cal-adc-tuner-en", "ADC Tuner", m_group);
	layout->addWidget(w6);

	m_layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	connect(this, &CalibrationsWidget::readRequested, this, [=, this]() {
		w0->readAsync();
		w1->readAsync();
		w2->readAsync();
		w3->readAsync();
		w4->readAsync();
		w5->readAsync();
		w6->readAsync();
	});
}

CalibrationsWidget::~CalibrationsWidget() {}
