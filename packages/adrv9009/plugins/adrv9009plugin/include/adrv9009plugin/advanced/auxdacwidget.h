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

#ifndef AUXDACWIDGET_H
#define AUXDACWIDGET_H

#include "scopy-adrv9009plugin_export.h"
#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QCheckBox>
#include <iio.h>
#include <iio-widgets/iiowidget.h>
#include <gui/widgets/menusectionwidget.h>

namespace scopy::adrv9009 {

class SCOPY_ADRV9009PLUGIN_EXPORT AuxDacWidget : public QWidget
{
	Q_OBJECT

public:
	explicit AuxDacWidget(iio_device *device, QWidget *parent = nullptr);

Q_SIGNALS:
	void readRequested();

private:
	iio_device *m_device;
	MenuSectionCollapseWidget *m_section;

	void setupUi();
};

} // namespace scopy::adrv9009

#endif // AUXDACWIDGET_H
