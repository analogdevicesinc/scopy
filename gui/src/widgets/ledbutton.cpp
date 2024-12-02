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

#include "ledbutton.h"
#include "style.h"

using namespace scopy;

LedButton::LedButton(QWidget *parent)
	: QPushButton(parent)
{
	setCheckable(true);
	setVisible(false);
	setFixedSize(QSize(Style::getDimension(json::global::unit_1), Style::getDimension(json::global::unit_1)));
	setAttribute(Qt::WA_TransparentForMouseEvents, true);

	QIcon ledIcon;
	ledIcon.addPixmap(Style::getPixmap(":/gui/icons/green_led.svg", Style::getColor(json::theme::content_default)),
			  QIcon::Normal, QIcon::On);
	ledIcon.addPixmap(Style::getPixmap(":/gui/icons/red_led.svg", Style::getColor(json::theme::content_default)),
			  QIcon::Normal, QIcon::Off);
	setIcon(ledIcon);

	m_timer = new QTimer(this);
	connect(m_timer, &QTimer::timeout, this, &LedButton::ledOff);
}

LedButton::~LedButton() {}

void LedButton::ledOn(bool ledState, int runningTimeMsec)
{
	if(!m_timer->isActive()) {
		setVisible(true);
	}
	if(ledState != isChecked()) {
		setChecked(ledState);
	}
	m_timer->start(runningTimeMsec);
}

void LedButton::ledOff()
{
	m_timer->stop();
	setVisible(false);
}

#include "moc_ledbutton.cpp"
