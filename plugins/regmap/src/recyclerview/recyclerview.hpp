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

#ifndef RECYCLERVIEW_H
#define RECYCLERVIEW_H

#include <QMap>
#include <QObject>
#include <QWidget>

#include <src/verticalscrollarea.hpp>

class QSlider;
class QGridLayout;

class QHBoxLayout;
namespace scopy::regmap {

#define DEFAULT_MAX_ROW_COUNT 15

class RecyclerView : public QWidget
{
	Q_OBJECT

public:
	explicit RecyclerView(QList<int> *widgets, QWidget *parent = nullptr);
	~RecyclerView();

	QMap<int, QWidget *> *getWidgetsMap() const;
	void setWidgetMap(QMap<int, QWidget *> *newWidgets);
	void addWidget(int index, QWidget *widget);
	QWidget *getWidgetAtIndex(int index);

	void hideAll();
	void showAll();
	void setActiveWidgets(QList<int> *widgets);
	void scrollTo(int index);
	void setMaxrowCount(int maxRowCount);

	void init();
	void populateMap();

Q_SIGNALS:
	void requestWidget(int index);
	void initDone();
	void requestInit();

private:
	int m_maxRowCount;
	QSlider *slider;
	VerticalScrollArea *m_scrollArea;
	int m_scrollBarCurrentValue;

	QGridLayout *bitFieldsWidgetLayout;
	QHBoxLayout *layout;

	QMap<int, QWidget *> *widgetMap;
	QList<int> *widgets;
	QList<int>::iterator activeWidgetTop;
	QList<int>::iterator activeWidgetBottom;

	void scrollDown();
	void scrollUp();

	// QObject interface
public:
	bool eventFilter(QObject *watched, QEvent *event);
};
} // namespace scopy::regmap
#endif // RECYCLERVIEW_H
