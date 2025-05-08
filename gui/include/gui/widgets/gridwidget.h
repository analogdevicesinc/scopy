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

#ifndef GRIDWIDGET_H
#define GRIDWIDGET_H

#include "scopy-gui_export.h"

#include <QGridLayout>
#include <QMap>
#include <QWidget>

namespace scopy {

class SCOPY_GUI_EXPORT GridWidget : public QWidget
{
	Q_OBJECT
public:
	GridWidget(QWidget *parent);
	~GridWidget();

	void addWidget(QWidget *widget, const QString &id);
	void removeWidget(const QString &id);
	void setColumnCount(int colCount);
	void clearWidgets();

protected:
	virtual void rebuildLayout();
	void hideAll();

	QGridLayout *m_layout;
	QMap<QString, QWidget *> m_widgetMap;
	int m_maxCol;
};

} // namespace scopy

#endif // GRIDWIDGET_H
