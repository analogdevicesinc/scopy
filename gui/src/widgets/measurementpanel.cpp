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

#include "widgets/measurementpanel.h"

#include <QGridLayout>
#include <QScrollArea>
#include <QWidget>

#include <measurementlabel.h>

using namespace scopy;

MeasurementsPanel::MeasurementsPanel(QWidget *parent)
	: QWidget(parent)
{
	QVBoxLayout *lay = new QVBoxLayout(this);
	setLayout(lay);

	QScrollBar *scrollBar = new QScrollBar(this);
	scrollBar->setOrientation(Qt::Horizontal);

	lay->setMargin(0);
	lay->setSpacing(0);
	lay->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

	stackSize = 4;

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setHorizontalScrollBar(scrollBar);
	scrollBar->setVisible(false);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	QWidget *panel = new QWidget(this);
	panelLayout = new QHBoxLayout(panel);
	panelLayout->setMargin(0);
	panelLayout->setSpacing(12);
	panelLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	panel->setLayout(panelLayout);
	panel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	scrollArea->setWidget(panel);
	scrollArea->setWidgetResizable(true);

	panelLayout->setAlignment(Qt::AlignLeft);
	lay->addWidget(scrollBar);
	lay->addWidget(scrollArea);

	connect(scrollArea->horizontalScrollBar(), &QAbstractSlider::rangeChanged, scrollBar,
		[=](double min, double max) {
			auto singleStep = scrollArea->horizontalScrollBar()->singleStep();
			scrollBar->setVisible(singleStep < (max - min));
		});

	m_cursor = new QWidget(panel);
	panelLayout->addWidget(m_cursor);

	spacer = new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding);
	panelLayout->addSpacerItem(spacer);

	int idx = panelLayout->indexOf(spacer);
	m_stacks.append(new VerticalWidgetStack(stackSize, this));
	panelLayout->insertWidget(idx, m_stacks.last());
}

void MeasurementsPanel::addWidget(QWidget *meas)
{
	bool createStack = false;
	if(m_stacks.count() == 0) {
		createStack = true;
	} else if(m_stacks.last()->full()) {
		createStack = true;
	}

	if(createStack) {
		m_stacks.append(new VerticalWidgetStack(stackSize, this));

		int idx = panelLayout->indexOf(spacer);
		panelLayout->insertWidget(idx, m_stacks.last());
	}
	m_stacks.last()->addWidget(meas);
}

void MeasurementsPanel::addMeasurement(MeasurementLabel *meas)
{
	if(!m_inhibitUpdates) {
		addWidget(meas);
	}
	m_labels.append(meas);
}

void MeasurementsPanel::removeMeasurement(MeasurementLabel *meas)
{

	int i = 0;
	for(i = 0; i < m_stacks.count(); i++) {
		if(m_stacks[i]->indexOf(meas) != -1) {
			break;
		}
	}

	m_labels.removeAll(meas);

	if(m_inhibitUpdates) {
		return;
	}
	int next_stack = i;
	int remaining_stacks = m_stacks.count() - next_stack;
	for(i = 0; i < remaining_stacks; i++) {
		m_stacks.last()->reparentWidgets(nullptr);
		delete m_stacks.last();
		m_stacks.removeLast();
	}

	int idx = next_stack * stackSize;
	for(i = idx; i < m_labels.count(); i++) {
		addWidget(m_labels[i]);
	}
}

void MeasurementsPanel::sort(int sortType)
{

	if(sortType == 0) {
		std::sort(m_labels.begin(), m_labels.end(), [=](MeasurementLabel *first, MeasurementLabel *second) {
			if(first->idx() == second->idx()) {
				return first->color().name() > second->color().name();
			}
			return first->idx() < second->idx();
		});
	} else {
		std::sort(m_labels.begin(), m_labels.end(), [=](MeasurementLabel *first, MeasurementLabel *second) {
			if(first->color().name() == second->color().name()) {
				return first->idx() < second->idx();
			}
			return first->color().name() > second->color().name();
		});
	}
	refreshUi();
}

bool MeasurementsPanel::inhibitUpdates() const { return m_inhibitUpdates; }

void MeasurementsPanel::setInhibitUpdates(bool newInhibitUpdates)
{
	m_inhibitUpdates = newInhibitUpdates;
	if(m_inhibitUpdates) {
		clear();
	} else {
		refreshUi();
	}
}

/*void MeasurementsPanel::inhibitUpdates(bool b) {
	m_inhibitUpdates = b;
	if(!b)
		updateOrder();
}*/

void MeasurementsPanel::clear()
{
	for(VerticalWidgetStack *stack : m_stacks) {
		stack->reparentWidgets(nullptr);
		panelLayout->removeWidget(stack);
		delete stack;
	}
	m_stacks.clear();
}

void MeasurementsPanel::refreshUi()
{
	for(VerticalWidgetStack *stack : m_stacks) {
		stack->reparentWidgets(nullptr);
		panelLayout->removeWidget(stack);
		delete stack;
	}
	m_stacks.clear();

	int idx = panelLayout->indexOf(spacer);
	m_stacks.append(new VerticalWidgetStack(stackSize, this));
	panelLayout->insertWidget(idx, m_stacks.last());

	for(QWidget *label : qAsConst(m_labels)) {
		addWidget(label);
	}
}

QWidget *MeasurementsPanel::cursorArea() { return m_cursor; }

StatsPanel::StatsPanel(QWidget *parent)
{
	QVBoxLayout *lay = new QVBoxLayout(this);
	setLayout(lay);

	lay->setMargin(0);
	lay->setSpacing(0);
	lay->setAlignment(Qt::AlignTop);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	QWidget *panel = new QWidget(this);
	panelLayout = new QHBoxLayout(panel);
	panelLayout->setMargin(0);
	panelLayout->setSpacing(6);
	panel->setLayout(panelLayout);
	scrollArea->setWidget(panel);
	scrollArea->setWidgetResizable(true);

	panelLayout->setAlignment(Qt::AlignLeft);
	lay->addWidget(scrollArea);
}

StatsPanel::~StatsPanel() {}

void StatsPanel::addStat(StatsLabel *stat)
{
	m_labels.append(stat);
	panelLayout->addWidget(stat);
}

void StatsPanel::removeStat(StatsLabel *stat)
{
	m_labels.removeAll(stat);
	panelLayout->removeWidget(stat);
}

void StatsPanel::updateOrder()
{
	for(StatsLabel *label : m_labels) {
		panelLayout->removeWidget(label);
	}

	for(StatsLabel *label : m_labels) {
		panelLayout->addWidget(label);
	}
}

void StatsPanel::sort(int sortType)
{
	if(sortType == 0) {
		std::sort(m_labels.begin(), m_labels.end(), [=](StatsLabel *first, StatsLabel *second) {
			if(first->idx() == second->idx()) {
				return first->color().name() > second->color().name();
			}
			return first->idx() < second->idx();
		});
	} else {
		std::sort(m_labels.begin(), m_labels.end(), [=](StatsLabel *first, StatsLabel *second) {
			if(first->color().name() == second->color().name()) {
				return first->idx() < second->idx();
			}
			return first->color().name() > second->color().name();
		});
	}
	updateOrder();
}

#include "moc_measurementpanel.cpp"
