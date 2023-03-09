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
#ifndef DRAGZONE_H
#define DRAGZONE_H

#include <QWidget>
#include <QMouseEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>

namespace Ui {
class DragZone;
}

namespace adiscope{
class DragZone : public QWidget
{
	Q_OBJECT

public:
	explicit DragZone(QWidget *parent = 0);
	~DragZone();

	bool eventFilter(QObject *watched, QEvent *event);

	int getPosition() const;
	void setPosition(int value);

Q_SIGNALS:
	void requestPositionChange(int, int, bool);
	void highlightLastSeparator(bool);

private Q_SLOTS:
	void dragEnterEvent(QDragEnterEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dragLeaveEvent(QDragLeaveEvent *event);
	void dropEvent(QDropEvent *event);

private:
	Ui::DragZone *ui;
	int position;
};
}
#endif // DRAGZONE_H
