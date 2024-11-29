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

#ifndef SCOPY_DETACHED_WINDOW_HPP
#define SCOPY_DETACHED_WINDOW_HPP

#include <QMainWindow>
#include <QEvent>

class QCloseEvent;

namespace adiscope {
class DetachedWindow: public QMainWindow
{
	Q_OBJECT

Q_SIGNALS:
	void closed();

public:
	explicit DetachedWindow(QWidget *parent = Q_NULLPTR);
	~DetachedWindow();

	void setCentralWidget(QWidget *widget);
	void showWindow();

	QString getName() const;
	void setName(const QString &value);

private:
	void closeEvent(QCloseEvent *event);
	void hideEvent(QHideEvent *event);
	Qt::WindowState state;
	QString name;
};

class DetachedWindowState {
public:

	DetachedWindowState();
	~DetachedWindowState() {}

	DetachedWindowState(DetachedWindow *detachedWindow);

	QString getName() const;
	void setName(const QString &value);

	QRect getGeometry() const;
	void setGeometry(const QRect &value);

	bool getMaximized() const;
	void setMaximized(bool value);

	bool getMinimized() const;
	void setMinimized(bool value);

private:
	QString name;
	QRect geometry;
	bool maximized;
	bool minimized;
};

}

#endif /* SCOPY_DETACHED_WINDOW_HPP */
