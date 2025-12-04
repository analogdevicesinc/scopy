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
 */

#ifndef ARMGPIOWIDGET_H
#define ARMGPIOWIDGET_H

#include <QWidget>
#include <iio.h>

class QVBoxLayout;

namespace scopy {
class MenuSectionCollapseWidget;

namespace adrv9009 {

class ArmGpioWidget : public QWidget
{
	Q_OBJECT

public:
	explicit ArmGpioWidget(iio_device *device, QWidget *parent = nullptr);
	~ArmGpioWidget();

	MenuSectionCollapseWidget *section() const;

Q_SIGNALS:
	void readRequested();

private:
	void setupUi();

	iio_device *m_device;
	MenuSectionCollapseWidget *m_section;
};

} // namespace adrv9009
} // namespace scopy

#endif // ARMGPIOWIDGET_H
