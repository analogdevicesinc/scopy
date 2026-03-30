/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#include "lazystackedwidget.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_LAZYSTACKEDWIDGET, "LazyStackedWidget")

using namespace scopy;

LazyStackedWidget::LazyStackedWidget(StackedWidgetProvider *provider, QWidget *parent)
	: MapStackedWidget(parent)
	, m_provider(provider)
{}

LazyStackedWidget::~LazyStackedWidget() {}

bool LazyStackedWidget::show(QString key)
{
	if(!contains(key) && m_provider) {
		QWidget *w = m_provider->createWidget(key);
		if(w) {
			add(key, w);
		} else {
			qWarning(CAT_LAZYSTACKEDWIDGET) << "Provider returned nullptr for key:" << key;
			return false;
		}
	}

	bool result = MapStackedWidget::show(key);

	if(result && m_provider) {
		m_provider->onShow(key, get(key));
	}

	return result;
}

bool LazyStackedWidget::remove(QString key)
{
	if(m_provider && contains(key)) {
		m_provider->onRemove(key, get(key));
	}
	return MapStackedWidget::remove(key);
}

#include "moc_lazystackedwidget.cpp"
