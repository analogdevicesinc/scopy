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
#ifndef DETACHDRAGZONE_H
#define DETACHDRAGZONE_H

#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMouseEvent>
#include <QString>
#include <QWidget>

namespace scopy {
class DetachDragZone : public QWidget
{
	Q_OBJECT

public:
	explicit DetachDragZone(QWidget *parent = 0);
	~DetachDragZone();

	bool eventFilter(QObject *watched, QEvent *event);

Q_SIGNALS:
	void detachWidget(int);
	void changeText(QString);

private Q_SLOTS:
	void dragEnterEvent(QDragEnterEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dragLeaveEvent(QDragLeaveEvent *event);
	void dropEvent(QDropEvent *event);
};
} // namespace scopy

#endif // DETACHDRAGZONE_H
