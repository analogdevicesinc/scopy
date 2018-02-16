/*
 * Copyright 2018 Analog Devices, Inc.
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

#ifndef HOMEPAGE_CONTROLS_H
#define HOMEPAGE_CONTROLS_H

#include <QWidget>
#include <QString>

namespace Ui {
class HomepageControls;
}

namespace adiscope {
class HomepageControls : public QWidget
{
	Q_OBJECT

public:
	explicit HomepageControls(QWidget *parent = 0);
	~HomepageControls();

	void updatePosition();
	virtual bool eventFilter(QObject *, QEvent *);

Q_SIGNALS:
	void goLeft();
	void goRight();
	void openFile();

private:
	Ui::HomepageControls *ui;
};
}
#endif // HOMEPAGE_CONTROLS_H
