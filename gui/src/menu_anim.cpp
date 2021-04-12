/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "menu_anim.hpp"

#include <QSignalTransition>
#include <QSizePolicy>

using namespace scopy::gui;

MenuAnim::MenuAnim(QWidget* parent)
	: ColoredQWidget(parent)
	, m_openAnimMax(this, "maximumWidth")
	, m_openAnimMin(this, "minimumWidth")
	, m_closeAnimMax(this, "maximumWidth")
	, m_closeAnimMin(this, "minimumWidth")
	, m_minWidth(-1)
	, m_animInProg(false)
{
	m_openAnimMax.setDuration(500);
	m_openAnimMax.setEasingCurve(QEasingCurve::InOutExpo);

	m_openAnimMin.setDuration(500);
	m_openAnimMin.setEasingCurve(QEasingCurve::InOutExpo);

	m_closeAnimMax.setDuration(500);
	m_closeAnimMax.setEasingCurve(QEasingCurve::InOutExpo);

	m_closeAnimMin.setDuration(500);
	m_closeAnimMin.setEasingCurve(QEasingCurve::InOutExpo);

	connect(&m_openAnimMax, SIGNAL(finished()), this, SLOT(openAnimFinished()));
	connect(&m_closeAnimMax, SIGNAL(finished()), this, SLOT(closeAnimFinished()));
}

void MenuAnim::toggleMenu(bool open)
{
	int start, stop;

	m_animInProg = true;

	if (m_minWidth < 0)
		m_minWidth = 0;

	if (open) {
		start = m_minWidth;
		stop = sizeHint().width();

		m_closeAnimMax.stop();
		m_closeAnimMin.stop();

		m_openAnimMax.setStartValue(start);
		m_openAnimMax.setEndValue(stop);
		m_openAnimMax.start();

		m_openAnimMin.setStartValue(start);
		m_openAnimMin.setEndValue(stop);
		m_openAnimMin.start();
	} else {
		start = sizeHint().width();
		stop = m_minWidth;

		m_openAnimMax.stop();
		m_openAnimMin.stop();

		m_closeAnimMax.setStartValue(start);
		m_closeAnimMax.setEndValue(stop);
		m_closeAnimMax.start();

		m_closeAnimMin.setStartValue(start);
		m_closeAnimMin.setEndValue(stop);
		m_closeAnimMin.start();
	}
}

void MenuAnim::setMinimumSize(QSize size)
{
	QWidget::setMinimumSize(size);

	/* Memorize the min width set in the .ui file */
	if (m_minWidth < 0)
		m_minWidth = size.width();
}

bool MenuAnim::animInProgress() const { return m_animInProg; }

void MenuAnim::closeAnimFinished()
{
	m_animInProg = false;
	Q_EMIT finished(false);
}

void MenuAnim::openAnimFinished()
{
	m_animInProg = false;
	Q_EMIT finished(true);
}
