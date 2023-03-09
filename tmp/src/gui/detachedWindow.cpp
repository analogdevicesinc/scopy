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

#include "detachedWindow.hpp"

#include <QHBoxLayout>

#include <QApplication>

using namespace adiscope;

DetachedWindow::DetachedWindow(QWidget *parent):
	QMainWindow(parent),
	state(Qt::WindowState::WindowNoState),
	name("")
{
	this->setWindowIcon(QApplication::windowIcon());
	this->setWindowTitle("Scopy - " + getName());
}


DetachedWindow::~DetachedWindow()
{
}

void DetachedWindow::closeEvent(QCloseEvent *event)
{
	Q_EMIT closed();
}

void DetachedWindow::hideEvent(QHideEvent *event)
{
	if (isMaximized())
		state = Qt::WindowState::WindowMaximized;
	else
		state = Qt::WindowState::WindowNoState;
}

QString DetachedWindow::getName() const
{
	return name;
}

void DetachedWindow::setName(const QString &value)
{
	name = value;
}

void DetachedWindow::setCentralWidget(QWidget *widget)
{
	QWidget *child = new QWidget(this);
	QHBoxLayout *layout = new QHBoxLayout(child);

        layout->setSpacing(0);
        layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(widget);

	QMainWindow::setCentralWidget(child);
}

void DetachedWindow::showWindow()
{
	if (isMinimized())
		setWindowState(state);
	activateWindow();
}

DetachedWindowState::DetachedWindowState():
	name(""),
	geometry(QRect()),
	maximized(false),
	minimized(true)
{
}

DetachedWindowState::DetachedWindowState(DetachedWindow *detachedWindow)
{
	name = detachedWindow->getName();
	geometry = detachedWindow->geometry();
	minimized = detachedWindow->isMinimized();
	maximized = detachedWindow->isMaximized();
}

QString DetachedWindowState::getName() const
{
	return name;
}

void DetachedWindowState::setName(const QString &value)
{
	name = value;
}

QRect DetachedWindowState::getGeometry() const
{
	return geometry;
}

void DetachedWindowState::setGeometry(const QRect &value)
{
	geometry = value;
}

bool DetachedWindowState::getMaximized() const
{
	return maximized;
}

void DetachedWindowState::setMaximized(bool value)
{
	maximized = value;
}

bool DetachedWindowState::getMinimized() const
{
	return minimized;
}

void DetachedWindowState::setMinimized(bool value)
{
	minimized = value;
}
