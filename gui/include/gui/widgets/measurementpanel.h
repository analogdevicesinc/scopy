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

#ifndef MEASUREMENTPANEL_H
#define MEASUREMENTPANEL_H

#include "scopy-gui_export.h"
#include "utils.h"

#include <QGridLayout>
#include <QScrollBar>
#include <QWidget>

namespace Ui {
class MeasurementsPanel;
};

namespace scopy {
class MeasurementLabel;
class StatsLabel;

class VerticalWidgetStack : public QWidget
{
	Q_OBJECT
public:
	VerticalWidgetStack(int stackSize = 4, QWidget *parent = nullptr)
	{
		lay = new QVBoxLayout(this);
		setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
		setLayout(lay);
		lay->setMargin(0);
		lay->setSpacing(6);
		spacer = new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);
		lay->addSpacerItem(spacer);
		m_stackSize = stackSize;
	}
	~VerticalWidgetStack() {}
	void addWidget(QWidget *w)
	{
		int idx = lay->indexOf(spacer);
		lay->insertWidget(idx, w, Qt::AlignTop | Qt::AlignLeft);
		m_widgets.append(w);
	}

	int indexOf(QWidget *w) { return m_widgets.indexOf(w); }

	void removeWidget(QWidget *w)
	{
		if(m_widgets.contains(w)) {
			m_widgets.removeAll(w);

			for(QWidget *w : qAsConst(m_widgets)) {
				lay->removeWidget(w);
				w->setParent(nullptr);
			}
			for(QWidget *w : qAsConst(m_widgets)) {
				int idx = lay->indexOf(spacer);
				lay->insertWidget(idx, w, Qt::AlignTop | Qt::AlignLeft);
			}
		}
	}

	void setStackSize(int val) { m_stackSize = val; }

	void reparentWidgets(QWidget *parent = nullptr)
	{
		for(QWidget *w : qAsConst(m_widgets)) {
			lay->removeWidget(w);
			w->setParent(parent);
		}
		m_widgets.clear();
	}

	bool full() { return (m_widgets.count() >= m_stackSize); }

private:
	QVBoxLayout *lay;
	QSpacerItem *spacer;
	int m_stackSize;
	QList<QWidget *> m_widgets;
};

class SCOPY_GUI_EXPORT MeasurementsPanel : public QWidget
{
	Q_OBJECT
public:
	MeasurementsPanel(QWidget *parent = nullptr);
	QWidget *cursorArea();

	bool inhibitUpdates() const;
	void setInhibitUpdates(bool newInhibitUpdates);

public Q_SLOTS:
	void addMeasurement(MeasurementLabel *meas);
	void removeMeasurement(MeasurementLabel *meas);
	void clear();
	void refreshUi();
	void sort(int sortType); // hackish

private:
	bool m_inhibitUpdates;
	QHBoxLayout *panelLayout;
	QList<MeasurementLabel *> m_labels;
	QList<VerticalWidgetStack *> m_stacks;
	QWidget *m_cursor;
	QSpacerItem *spacer;
	void addWidget(QWidget *meas);
	int stackSize;
};

class SCOPY_GUI_EXPORT StatsPanel : public QWidget
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	StatsPanel(QWidget *parent = nullptr);
	~StatsPanel();
public Q_SLOTS:
	void addStat(StatsLabel *stat);
	void removeStat(StatsLabel *stat);
	void updateOrder();
	void sort(int sortType); // hackish
private:
	QHBoxLayout *panelLayout;
	QList<StatsLabel *> m_labels;
};

} // namespace scopy
#endif // MEASUREMENTPANEL_H
