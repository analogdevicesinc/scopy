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

#ifndef LAZYSTACKEDWIDGET_H
#define LAZYSTACKEDWIDGET_H

#include "scopy-gui_export.h"
#include "mapstackedwidget.h"
#include "stackedwidgetprovider.h"

namespace scopy {

class SCOPY_GUI_EXPORT LazyStackedWidget : public MapStackedWidget
{
	Q_OBJECT
public:
	explicit LazyStackedWidget(StackedWidgetProvider *provider, QWidget *parent = nullptr);
	~LazyStackedWidget();

	bool show(QString key) override;
	bool remove(QString key) override;

private:
	StackedWidgetProvider *m_provider;
};

} // namespace scopy

#endif // LAZYSTACKEDWIDGET_H
