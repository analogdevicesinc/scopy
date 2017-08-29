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

#ifndef SCOPY_DETACHED_WINDOW_HPP
#define SCOPY_DETACHED_WINDOW_HPP

#include <QMainWindow>

class QCloseEvent;

namespace adiscope {
class DetachedWindow: public QMainWindow
{
	Q_OBJECT

Q_SIGNALS:
	void closed();

public:
	explicit DetachedWindow(QIcon icon, QWidget *parent = Q_NULLPTR);
	~DetachedWindow();

	void setCentralWidget(QWidget *widget);

private:
	void closeEvent(QCloseEvent *event);
};
}

#endif /* SCOPY_DETACHED_WINDOW_HPP */
