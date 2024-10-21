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

#ifndef PLOTBUTTONMANAGER_H
#define PLOTBUTTONMANAGER_H
#include "utils.h"
#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <compositewidget.h>

class PlotButtonManager : public QWidget, public CompositeWidget, public Collapsable
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	typedef enum
	{
		PBM_LEFT,
		PBM_RIGHT
	} CollapseButtonOrientation;
	PlotButtonManager(QWidget *parent);
	~PlotButtonManager();

	void add(QWidget *w) override;
	void remove(QWidget *w) override;
	bool event(QEvent *ev) override;
	void setCollapseOrientation(CollapseButtonOrientation);

public Q_SLOTS:
	virtual bool collapsed() override;
	virtual void setCollapsed(bool b) override;

private:
	void collapsePriv(bool);
	QHBoxLayout *m_lay;
	QHBoxLayout *m_collapsablelay;
	QPushButton *m_collapseBtn;
	QWidget *m_collapsableContainer;
};

#endif // PLOTBUTTONMANAGER_H
