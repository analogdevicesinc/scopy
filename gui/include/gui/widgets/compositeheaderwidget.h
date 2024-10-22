/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#ifndef COMPOSITEHEADERWIDGET_H
#define COMPOSITEHEADERWIDGET_H

#include <QLineEdit>
#include <baseheader.h>
#include <compositewidget.h>
#include <scopy-gui_export.h>
#include <QVBoxLayout>
#include <baseheaderwidget.h>

namespace scopy {
class SCOPY_GUI_EXPORT CompositeHeaderWidget : public BaseHeaderWidget, public CompositeWidget
{
	Q_OBJECT
public:
	CompositeHeaderWidget(QString title, QWidget *parent = nullptr);
	~CompositeHeaderWidget();

	void add(QWidget *w) override;
	void remove(QWidget *w) override;
};
} // namespace scopy

#endif // COMPOSITEHEADERWIDGET_H
