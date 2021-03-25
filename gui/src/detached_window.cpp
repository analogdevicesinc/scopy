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

#include "detached_window.hpp"

#include <QApplication>
#include <QHBoxLayout>

using namespace scopy::gui;

DetachedWindow::DetachedWindow(QWidget* parent)
	: QMainWindow(parent)
	, m_state(Qt::WindowState::WindowNoState)
	, m_name("")
{
	this->setWindowIcon(QApplication::windowIcon());
	this->setWindowTitle("Scopy - " + getName());
}

DetachedWindow::~DetachedWindow() {}

void DetachedWindow::closeEvent(QCloseEvent* event) { Q_EMIT closed(); }

void DetachedWindow::hideEvent(QHideEvent* event)
{
	if (isMaximized())
		m_state = Qt::WindowState::WindowMaximized;
	else
		m_state = Qt::WindowState::WindowNoState;
}

QString DetachedWindow::getName() const { return m_name; }

void DetachedWindow::setName(const QString& value) { m_name = value; }

void DetachedWindow::setCentralWidget(QWidget* widget)
{
	QWidget* child = new QWidget(this);
	QHBoxLayout* layout = new QHBoxLayout(child);

	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(widget);

	QMainWindow::setCentralWidget(child);
}

void DetachedWindow::showWindow()
{
	if (isMinimized())
		setWindowState(m_state);
	activateWindow();
}

DetachedWindowState::DetachedWindowState()
	: m_name("")
	, m_geometry(QRect())
	, m_maximized(false)
	, m_minimized(true)
{}

DetachedWindowState::DetachedWindowState(DetachedWindow* detachedWindow)
{
	m_name = detachedWindow->getName();
	m_geometry = detachedWindow->geometry();
	m_minimized = detachedWindow->isMinimized();
	m_maximized = detachedWindow->isMaximized();
}

QString DetachedWindowState::getName() const { return m_name; }

void DetachedWindowState::setName(const QString& value) { m_name = value; }

QRect DetachedWindowState::getGeometry() const { return m_geometry; }

void DetachedWindowState::setGeometry(const QRect& value) { m_geometry = value; }

bool DetachedWindowState::getMaximized() const { return m_maximized; }

void DetachedWindowState::setMaximized(bool value) { m_maximized = value; }

bool DetachedWindowState::getMinimized() const { return m_minimized; }

void DetachedWindowState::setMinimized(bool value) { m_minimized = value; }
