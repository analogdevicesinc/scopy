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

#include "scopy-adrv9009plugin_export.h"
#include <QWidget>
#include <iio.h>
#include <iio-widgets/iiowidgetgroup.h>

class QVBoxLayout;

namespace scopy {
class MenuSectionCollapseWidget;

namespace adrv9009 {

class SCOPY_ADRV9009PLUGIN_EXPORT ArmGpioWidget : public QWidget
{
	Q_OBJECT

public:
	ArmGpioWidget(iio_device *device, IIOWidgetGroup *group = nullptr, QWidget *parent = nullptr);
	~ArmGpioWidget();

	MenuSectionCollapseWidget *section() const;

Q_SIGNALS:
	void readRequested();

private:
	void setupUi();

	iio_device *m_device;
	IIOWidgetGroup *m_widgetGroup = nullptr;
	MenuSectionCollapseWidget *m_section;
};

} // namespace adrv9009
} // namespace scopy

#endif // ARMGPIOWIDGET_H
