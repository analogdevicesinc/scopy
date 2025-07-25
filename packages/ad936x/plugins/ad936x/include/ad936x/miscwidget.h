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

#ifndef MISCWIDGET_H
#define MISCWIDGET_H

#include <QWidget>
#include <QBoxLayout>
#include <iio.h>

namespace scopy {
namespace ad936x {
class MiscWidget : public QWidget
{
	Q_OBJECT
public:
	explicit MiscWidget(iio_device *device, QWidget *parent = nullptr);
	~MiscWidget();

Q_SIGNALS:
	void readRequested();

private:
	QVBoxLayout *m_layout;
	iio_device *m_device = nullptr;
};
} // namespace ad936x
} // namespace scopy
#endif // MISCWIDGET_H
