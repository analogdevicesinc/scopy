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

#include "registermaptable.hpp"

#include "../logging_categories.h"
#include "recyclerview.hpp"

#include <QLabel>

#include <src/register/registersimplewidget.hpp>
#include <src/register/registersimplewidgetfactory.hpp>

using namespace scopy;
using namespace regmap;

RegisterMapTable::RegisterMapTable(QMap<uint32_t, RegisterModel *> *registerModels, QWidget *parent)
	: registerModels(registerModels)
{
	registersMap = new QMap<uint32_t, RegisterSimpleWidget *>();

	QList<int> *widgets = new QList<int>();
	foreach(uint32_t index, registerModels->keys()) {
		widgets->push_back((int)index);
	}

	recyclerView = new RecyclerView(widgets, parent);
	recyclerView->setMaxrowCount(10);

	QObject::connect(recyclerView, &RecyclerView::requestWidget, this, &RegisterMapTable::generateWidget);
	QObject::connect(this, &RegisterMapTable::widgetGenerated, recyclerView, &RecyclerView::addWidget);
	QObject::connect(
		recyclerView, &RecyclerView::initDone, this,
		[=]() {
			RegisterSimpleWidget *registerWidget = registersMap->value(registersMap->firstKey());
			RegisterModel *registerModel = registerWidget->getRegisterModel();
			selectedAddress = registerModel->getAddress();
			registersMap->value(selectedAddress)->setRegisterSelected(true);
		},
		Qt::QueuedConnection);

	Q_EMIT recyclerView->requestInit();
}

QWidget *RegisterMapTable::getWidget() { return recyclerView; }

void RegisterMapTable::setFilters(QList<uint32_t> filters)
{
	qDebug(CAT_REGISTER_MAP_TABLE) << "Apply filters ";
	QList<int> *widgets = new QList<int>();
	foreach(uint32_t index, filters) {
		widgets->push_back((int)index);
	}

	recyclerView->hideAll();
	recyclerView->setActiveWidgets(widgets);
	recyclerView->populateMap();
}

void RegisterMapTable::valueUpdated(uint32_t address, uint32_t value)
{
	qDebug(CAT_REGISTER_MAP_TABLE) << "Update value for register at address " << address;
	if(registersMap->contains(address)) {
		registersMap->value(address)->valueUpdated(value);
	} else {
		qDebug(CAT_REGISTER_MAP_TABLE) << "No register was found for address " << address;
	}
}

void RegisterMapTable::scrollTo(uint32_t index)
{
	if(registerModels->keys().contains(index)) {

		QMap<uint32_t, RegisterModel *>::iterator iterator = registerModels->begin();
		int i = 0;
		while(iterator != registerModels->end()) {
			if(iterator.key() == index) {
				recyclerView->scrollTo(i);
				setRegisterSelected(iterator.key());
				break;
			}
			i++;
			iterator++;
		}
	}
}

void RegisterMapTable::setRegisterSelected(uint32_t address)
{
	if(registersMap->value(selectedAddress)) {
		registersMap->value(selectedAddress)->setRegisterSelected(false);
	}
	selectedAddress = address;
	if(registersMap->value(address)) {
		registersMap->value(address)->setRegisterSelected(true);
	}
}

void RegisterMapTable::generateWidget(int index)
{
	qDebug(CAT_REGISTER_MAP_TABLE) << "Generate new widget";
	RegisterSimpleWidgetFactory registerSimpleWidgetFactory;
	RegisterSimpleWidget *registerSimpleWidget =
		registerSimpleWidgetFactory.buildWidget(registerModels->value(index));

	QObject::connect(registerSimpleWidget, &RegisterSimpleWidget::registerSelected, this,
			 &RegisterMapTable::registerSelected);

	registersMap->insert(index, registerSimpleWidget);
	Q_EMIT widgetGenerated(index, registerSimpleWidget);
}
