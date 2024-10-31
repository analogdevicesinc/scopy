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

#ifndef FLEXGRIDLAYOUT_HPP
#define FLEXGRIDLAYOUT_HPP

#include "scopy-gui_export.h"

#include <QResizeEvent>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

namespace scopy {
class SCOPY_GUI_EXPORT FlexGridLayout : public QWidget
{
	Q_OBJECT

public:
	explicit FlexGridLayout(int maxCols, QWidget *parent = nullptr);
	~FlexGridLayout() override;

	void toggleAll(bool toggled);
	void addWidget(int index);
	void removeWidget(int index);
	QWidget *getWidget(int index);
	bool isWidgetActive(int index);
	int addQWidgetToList(QWidget *widget);

	void setMaxColumnNumber(int maxColumns);
	int getMaxColumnNumber();

	int fullRows() const;
	int rows() const;
	int columns() const;
	int columnsOnLastRow() const;

	int count() const;

Q_SIGNALS:
	void reqestLayoutUpdate();

public Q_SLOTS:
	void updateLayout();
	void itemSizeChanged();

private:
	int m_maxCols;
	int currentNumberOfCols;
	int col;
	int row;
	double colWidth;
	double availableWidth;
	std::vector<QWidget *> m_widgetList;
	QVector<int> m_activeWidgetList;
	QGridLayout *m_gridLayout;
	QSpacerItem *m_hspacer;
	QSpacerItem *m_vspacer;

	// resize related
	bool updatePending;
	void redrawWidgets();
	void recomputeColCount();
	void computeCols(double width);
	void resizeEvent(QResizeEvent *event) override;
	void repositionWidgets(int index, int row, int col);
};
} // namespace scopy
#endif // FLEXGRIDLAYOUT_HPP
