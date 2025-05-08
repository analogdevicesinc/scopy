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

#include "gridwidget.h"

using namespace scopy;

GridWidget::GridWidget(QWidget *parent)
	: QWidget(parent)
	, m_maxCol(1)
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(0);

	QWidget *gridContainer = new QWidget(this);
	m_layout = new QGridLayout(gridContainer);
	m_layout->setMargin(0);

	mainLayout->addWidget(gridContainer);
	mainLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

GridWidget::~GridWidget() {}

void GridWidget::addWidget(QWidget *widget, const QString &id)
{
	int gridSize = m_widgetMap.size();
	m_layout->addWidget(widget, gridSize / m_maxCol, gridSize % m_maxCol);
	m_widgetMap.insert(id, widget);
}

void GridWidget::removeWidget(const QString &id)
{
	if(!m_widgetMap.contains(id)) {
		return;
	}
	QWidget *widget = m_widgetMap.take(id);
	m_layout->removeWidget(widget);
	widget->deleteLater();
}

void GridWidget::setColumnCount(int colCount)
{
	m_maxCol = colCount;
	rebuildLayout();
}

void GridWidget::clearWidgets()
{
	for(auto it = m_widgetMap.begin(); it != m_widgetMap.end(); ++it) {
		m_layout->removeWidget(it.value());
		it.value()->deleteLater();
	}
	m_widgetMap.clear();
}

void GridWidget::hideAll()
{
	for(auto it = m_widgetMap.begin(); it != m_widgetMap.end(); ++it) {
		it.value()->hide();
	}
}

void GridWidget::rebuildLayout()
{
	hideAll();
	int counter = 0;
	for(auto it = m_widgetMap.begin(); it != m_widgetMap.end(); ++it) {
		m_layout->addWidget(it.value(), counter / m_maxCol, counter % m_maxCol);
		it.value()->show();
		counter++;
	}
}

#include "moc_gridwidget.cpp"
