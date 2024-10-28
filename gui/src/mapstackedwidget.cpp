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

#include "mapstackedwidget.h"

#include <QDebug>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_MAPSTACKWIDGET, "MapStackedWidget")

using namespace scopy;

MapStackedWidget::MapStackedWidget(QWidget *parent)
	: QStackedWidget(parent)
{}

MapStackedWidget::~MapStackedWidget() {}

QString MapStackedWidget::getKey(QWidget *w)
{
	QString key = map.key(w, nullptr);
	return key;
}

void MapStackedWidget::add(QString key, QWidget *w)
{
	map[key] = w;
	w->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	addWidget(w);
	qDebug(CAT_MAPSTACKWIDGET) << key << "added to stack";
}

bool MapStackedWidget::remove(QString key)
{
	QWidget *w = map.take(key);
	if(w) {
		//		if(indexOf(w) == currentIndex())
		//			setCurrentIndex(0);
		removeWidget(w);
	} else {
		qWarning(CAT_MAPSTACKWIDGET) << key << "not found in MapStackWidget. cannot remove";
		return false;
	}
	qDebug(CAT_MAPSTACKWIDGET) << key << "removed from to stack";
	return true;
}

bool MapStackedWidget::contains(QString key) { return map.contains(key); }

QWidget *MapStackedWidget::get(QString key)
{
	if(map.contains(key))
		return map[key];
	return nullptr;
}

QSize MapStackedWidget::sizeHint() const
{
	if(currentWidget()) {
		return currentWidget()->sizeHint();
	}
	return QStackedWidget::sizeHint();
}

QSize MapStackedWidget::minimumSizeHint() const
{
	if(currentWidget()) {
		return currentWidget()->minimumSizeHint();
	}
	return QStackedWidget::minimumSizeHint();
}

bool MapStackedWidget::show(QString key)
{
	QWidget *w = map[key];
	if(w) {
		if(indexOf(map[key]) != -1) {
			this->currentWidget()->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
			setCurrentWidget(w);
			w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
			qDebug(CAT_MAPSTACKWIDGET) << key << " found - showing";
			return true;
		} else {
			qWarning(CAT_MAPSTACKWIDGET) << key << " found in stack but not found in qwidgetstack";
		}
	} else {
		qWarning(CAT_MAPSTACKWIDGET) << key << "not found in MapStackWidget. cannot show";
	}
	return false;
}

#include "moc_mapstackedwidget.cpp"
