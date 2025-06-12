/*
 * Copyright (c) 2025 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef ENSMMODECLOCKSWIDGET_H
#define ENSMMODECLOCKSWIDGET_H

#include <QBoxLayout>
#include <QWidget>
#include <iio.h>

namespace scopy {
namespace pluto {

class EnsmModeClocksWidget : public QWidget
{
	Q_OBJECT
public:
	explicit EnsmModeClocksWidget(iio_device *device, QWidget *parent = nullptr);
	~EnsmModeClocksWidget();

Q_SIGNALS:
	void readRequested();

private:
	QVBoxLayout *m_layout;
	iio_device *m_device = nullptr;
	QWidget *generateEnsmModeWidget(QWidget *parent);
	QWidget *generateModeWidget(QWidget *parent);
	QWidget *generateClocksWidget(QWidget *parent);
};
} // namespace pluto
} // namespace scopy

#endif // ENSMMODECLOCKSWIDGET_H
