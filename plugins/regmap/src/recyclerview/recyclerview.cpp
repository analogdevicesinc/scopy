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

#include "recyclerview.hpp"

#include <QDebug>
#include <QEvent>
#include <QGridLayout>
#include <QScrollBar>
#include <QScrollEvent>
#include <QSlider>
#include <qlabel.h>
#include <regmapstylehelper.hpp>

#include <src/verticalscrollarea.hpp>
#include "style_properties.h"
#include <style.h>

using namespace scopy::regmap;

RecyclerView::RecyclerView(QList<int> *widgets, QWidget *parent)
	: QWidget(parent)
	, widgets(widgets)
{
	m_maxRowCount = DEFAULT_MAX_ROW_COUNT;
	this->installEventFilter(this);
	layout = new QHBoxLayout();
	this->setLayout(layout);
	layout->setMargin(0);
	layout->setSpacing(0);

	bitFieldsWidgetLayout = new QGridLayout();
	bitFieldsWidgetLayout->setMargin(0);
	bitFieldsWidgetLayout->setSpacing(0);
	QWidget *bitFieldsWidget = new QWidget(this);
	bitFieldsWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	bitFieldsWidget->setLayout(bitFieldsWidgetLayout);

	m_scrollArea = new VerticalScrollArea(this);
	m_scrollArea->setWidget(bitFieldsWidget);
	m_scrollArea->verticalScrollBar()->setVisible(false);
	layout->addWidget(m_scrollArea);

	slider = new QSlider(this);
	slider->setInvertedAppearance(true);
	slider->setInvertedControls(true);
	slider->setFixedWidth(8);
	Style::setStyle(slider, style::properties::regmap::regmapSlider);

	QObject::connect(m_scrollArea->verticalScrollBar(), &QAbstractSlider::valueChanged, this, [=](int value) {
		if(value == m_scrollArea->verticalScrollBar()->minimum()) {
			slider->setValue(m_scrollBarCurrentValue - 1);
			m_scrollArea->verticalScrollBar()->setValue(value + 1);
		}
		if(value == m_scrollArea->verticalScrollBar()->maximum()) {
			slider->setValue(m_scrollBarCurrentValue + 1);
			m_scrollArea->verticalScrollBar()->setValue(value - 1);
		}
	});
	layout->addWidget(slider);

	QObject::connect(this, &RecyclerView::requestInit, this, &RecyclerView::init, Qt::QueuedConnection);
}

RecyclerView::~RecyclerView()
{
	delete bitFieldsWidgetLayout;
	delete widgetMap;
	delete m_scrollArea;
	delete layout;
}

void RecyclerView::init()
{
	widgetMap = new QMap<int, QWidget *>();

	populateMap();

	m_scrollBarCurrentValue = slider->value();

	QObject::connect(slider, &QAbstractSlider::valueChanged, this, [=](int value) {
		if(m_scrollBarCurrentValue < value) {
			int diff = value - (activeWidgetTop - widgets->begin());

			while(diff > 0) {
				scrollDown();
				diff--;
			}

		} else {
			int diff = (activeWidgetTop - widgets->begin()) - value;
			while(diff > 0) {
				scrollUp();
				diff--;
			}
		}
		m_scrollBarCurrentValue = value;
	});

	Q_EMIT initDone();
}

QMap<int, QWidget *> *RecyclerView::getWidgetsMap() const { return widgetMap; }

void RecyclerView::setWidgetMap(QMap<int, QWidget *> *newWidgets) { widgetMap = newWidgets; }

void RecyclerView::addWidget(int index, QWidget *widget)
{
	widgetMap->insert(index, widget);
	bitFieldsWidgetLayout->addWidget(widget, index, 0);
}

QWidget *RecyclerView::getWidgetAtIndex(int index) { return widgetMap->value(index); }

void RecyclerView::hideAll()
{
	QMap<int, QWidget *>::iterator mapIterator;
	for(mapIterator = widgetMap->begin(); mapIterator != widgetMap->end(); ++mapIterator) {
		mapIterator.value()->hide();
	}
}

void RecyclerView::showAll()
{
	QMap<int, QWidget *>::iterator mapIterator;
	for(mapIterator = widgetMap->begin(); mapIterator != widgetMap->end(); ++mapIterator) {
		mapIterator.value()->show();
	}
}

void RecyclerView::setActiveWidgets(QList<int> *widgets) { this->widgets = widgets; }

void RecyclerView::scrollTo(int index)
{
	m_scrollArea->verticalScrollBar()->setValue(0);
	slider->setValue(index);
}

void RecyclerView::setMaxrowCount(int maxRowCount) { this->m_maxRowCount = maxRowCount; }

void RecyclerView::scrollDown()
{
	if(activeWidgetBottom != widgets->end()) {
		if(widgetMap->contains(*activeWidgetBottom)) {
			widgetMap->value(*activeWidgetBottom)->show();
		} else {
			Q_EMIT requestWidget(*activeWidgetBottom);
		}

		if(widgetMap->value(*activeWidgetTop)) {
			widgetMap->value(*activeWidgetTop)->hide();
		}

		activeWidgetTop++;
		activeWidgetBottom++;
	}
}

void RecyclerView::scrollUp()
{
	if(activeWidgetTop != widgets->begin()) {
		activeWidgetTop--;
		activeWidgetBottom--;

		if(widgetMap->contains(*activeWidgetTop)) {
			widgetMap->value(*activeWidgetTop)->show();
		} else {
			Q_EMIT requestWidget(*activeWidgetTop);
		}
		if(widgetMap->value(*activeWidgetBottom)) {
			widgetMap->value(*activeWidgetBottom)->hide();
		}
	}
}

void RecyclerView::populateMap()
{
	QList<int>::iterator mapIterator = widgets->begin();
	if(widgetMap->isEmpty()) {
		Q_EMIT requestWidget(*mapIterator);
		++mapIterator;
	}

	// TODO find a way to autocompute max row count
	int i = 0;
	while(i < m_maxRowCount && mapIterator != widgets->end()) {
		if(widgetMap->contains(*mapIterator)) {
			widgetMap->value(*mapIterator)->show();
		} else {
			Q_EMIT requestWidget(*mapIterator);
		}
		++mapIterator;
		++i;
	}

	activeWidgetBottom = mapIterator;
	activeWidgetTop = widgets->begin();

	slider->setMaximum(widgets->length());
	slider->setSingleStep(1);
}

bool RecyclerView::eventFilter(QObject *watched, QEvent *event)
{
	if(event->type() == QScrollEvent::Scroll) {
		QScrollEvent *scrollEvent = static_cast<QScrollEvent *>(event);
		slider->setSliderPosition(m_scrollBarCurrentValue + 1);
	}

	return QWidget::eventFilter(watched, event);
}
