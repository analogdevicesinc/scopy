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

#ifndef PAPROTECTIONWIDGET_H
#define PAPROTECTIONWIDGET_H

#include "scopy-adrv9009plugin_export.h"
#include <QWidget>
#include <gui/widgets/menusectionwidget.h>
#include <iio-widgets/iiowidget.h>
#include <iio.h>

namespace scopy::adrv9009 {

class SCOPY_ADRV9009PLUGIN_EXPORT PaProtectionWidget : public QWidget
{
	Q_OBJECT
public:
	PaProtectionWidget(iio_device *device, QWidget *parent = nullptr);
	~PaProtectionWidget();

	MenuSectionCollapseWidget *section() const;

Q_SIGNALS:
	void readRequested();

private:
	iio_device *m_device;
	MenuSectionCollapseWidget *m_section;

	void setupUi();
};

} // namespace scopy::adrv9009

#endif // PAPROTECTIONWIDGET_H