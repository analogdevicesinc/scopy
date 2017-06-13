/*
 * Copyright 2017 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "detachedWindow.hpp"

#include <QHBoxLayout>

using namespace adiscope;

DetachedWindow::DetachedWindow(QWidget *parent) : QMainWindow(parent)
{
}

DetachedWindow::~DetachedWindow()
{
}

void DetachedWindow::closeEvent(QCloseEvent *event)
{
	Q_EMIT closed();
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
