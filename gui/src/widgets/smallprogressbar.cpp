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

#include "smallprogressbar.h"
#include <style.h>

using namespace scopy;

SmallProgressBar::SmallProgressBar(QWidget *parent)
	: QProgressBar(parent)
	, m_timer(new QTimer(this))
	, m_increment(10)
{
	setTextVisible(false);
	setFixedHeight(1);
	setMinimum(0);
	setMaximum(100);
	setValue(maximum());
	resetBarColor(); // set initial color

	connect(m_timer, &QTimer::timeout, this, [this]() {
		if(value() + m_increment <= maximum()) {
			setValue(value() + m_increment);
		} else {
			setValue(maximum());
			m_timer->stop();
			Q_EMIT progressFinished();
		}
	});
}

void SmallProgressBar::startProgress(int progressDuration, int steps)
{
	setValue(minimum());
	m_increment = maximum() / steps;
	m_timer->start(progressDuration / steps);
}

void SmallProgressBar::setBarColor(QColor color)
{
	QString style = QString("QProgressBar::chunk {background-color: %1;}").arg(color.name());
	setStyleSheet(style);
}

void SmallProgressBar::resetBarColor() { setBarColor(Style::getAttribute(json::theme::content_subtle)); }

#include "moc_smallprogressbar.cpp"

