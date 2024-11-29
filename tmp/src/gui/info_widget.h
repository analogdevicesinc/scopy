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
#ifndef INFO_WIDGET_H
#define INFO_WIDGET_H

#include <QWidget>

namespace Ui {
class InfoWidget;
}
namespace adiscope {
class InfoWidget : public QWidget
{
	Q_OBJECT

public:
	explicit InfoWidget(QWidget *parent = 0);
	~InfoWidget();

private:
	Ui::InfoWidget *ui;
	QTimer *timer;

public Q_SLOTS:
	void enable(bool);
	void setText(QString);

private Q_SLOTS:
	void updatePosition();
};
}

#endif // INFO_WIDGET_H
