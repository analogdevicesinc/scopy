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

#include "plotbuttonmanager.h"

PlotButtonManager::PlotButtonManager(QWidget *parent)
	: QWidget(parent)
{

	m_collapsableContainer = new QWidget(this);
	m_collapsablelay = new QHBoxLayout(m_collapsableContainer);
	m_collapsablelay->setSpacing(0);
	m_collapsablelay->setMargin(0);
	m_collapsableContainer->setLayout(m_collapsablelay);

	m_lay = new QHBoxLayout(this);
	m_collapseBtn = new QPushButton("", this);
	m_collapseBtn->setCheckable(true);
	m_collapseBtn->setChecked(true);
	m_collapseBtn->setFixedSize(4, 16);
	m_collapseBtn->setStyleSheet("background-color: #AAAAAAAA");

	m_lay->addWidget(m_collapseBtn);
	m_lay->addWidget(m_collapsableContainer);
	m_lay->setSpacing(0);
	m_lay->setMargin(0);

	connect(m_collapseBtn, &QAbstractButton::toggled, this, &PlotButtonManager::collapsePriv);

	setMouseTracking(true);
	m_collapsableContainer->setVisible(false);
	m_collapseBtn->setVisible(false);
}

PlotButtonManager::~PlotButtonManager() {}

void PlotButtonManager::add(QWidget *w)
{
	m_collapseBtn->setVisible(m_collapsablelay->count() > 0);
	m_collapsablelay->addWidget(w);
}

void PlotButtonManager::remove(QWidget *w)
{
	m_collapseBtn->setVisible(m_collapsablelay->count() > 0);
	m_collapsablelay->removeWidget(w);
}

bool PlotButtonManager::event(QEvent *event)
{
	if(event->type() == QEvent::Enter) {
		collapsePriv(false);
	}
	if(event->type() == QEvent::Leave) {
		collapsePriv(true);
	}

	return QWidget::event(event);
}

void PlotButtonManager::setCollapseOrientation(CollapseButtonOrientation p)
{
	if(p == PBM_LEFT) {
		m_lay->addWidget(m_collapseBtn);
		m_lay->addWidget(m_collapsableContainer);
	} else {
		m_lay->addWidget(m_collapsableContainer);
		m_lay->addWidget(m_collapseBtn);
	}
}

bool PlotButtonManager::collapsed() { return m_collapseBtn->isChecked(); }

void PlotButtonManager::setCollapsed(bool b) { m_collapseBtn->setChecked(true); }

void PlotButtonManager::collapsePriv(bool b) { m_collapsableContainer->setVisible(!b); }

// eventfilter

#include "moc_plotbuttonmanager.cpp"
