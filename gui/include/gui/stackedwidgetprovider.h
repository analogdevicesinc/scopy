/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#ifndef STACKEDWIDGETPROVIDER_H
#define STACKEDWIDGETPROVIDER_H

#include "scopy-gui_export.h"

#include <QString>
#include <QWidget>

namespace scopy {

class SCOPY_GUI_EXPORT StackedWidgetProvider
{
public:
	virtual ~StackedWidgetProvider() = default;

	virtual QWidget *createWidget(const QString &key) = 0;
	virtual void onShow(const QString &key, QWidget *widget) = 0;
	virtual void onRemove(const QString &key, QWidget *widget) = 0;
};

} // namespace scopy

#endif // STACKEDWIDGETPROVIDER_H
