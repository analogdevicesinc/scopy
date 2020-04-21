/*
 * Copyright 2019 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "basemenu.h"

#include "utils.h"

#include "ui_basemenu.h"

#include <QMimeData>

using namespace adiscope;

BaseMenu::BaseMenu(QWidget *parent) : QWidget(parent), d_ui(new Ui::BaseMenu), d_items(0) {
	d_ui->setupUi(this);
	setAcceptDrops(true);

	Util::retainWidgetSizeWhenHidden(d_ui->bottomSeparator);
	d_ui->bottomSeparator->setVisible(false);
}

BaseMenu::~BaseMenu() { delete d_ui; }

void BaseMenu::insertMenuItem(BaseMenuItem *menuItem, int position) {
	for (int i = 0; i < d_ui->mainLayout->count(); ++i) {
		if (menuItem == d_ui->mainLayout->itemAt(i)->widget()) {
			return;
		}
	}

	d_ui->mainLayout->insertWidget(position, menuItem);
	d_items = d_ui->mainLayout->count();

	menuItem->setVisible(true);

	connect(menuItem, &BaseMenuItem::moveItem, this, &BaseMenu::moveItem);

	_updateItemsPosition();
}

void BaseMenu::insertMenuItem(QVector<BaseMenuItem *> items, QVector<int> positions) {
	if (!positions.empty()) {
		int i = 0;
		while (i < items.size() && positions.size()) {
			insertMenuItem(items[i], positions[i]);
			i++;
		}
		while (i < items.size()) {
			insertMenuItem(items[i]);
			i++;
		}
	} else {
		for (int i = 0; i < items.size(); ++i) {
			insertMenuItem(items[i]);
		}
	}
}

void BaseMenu::removeMenuItem(BaseMenuItem *menuItem) {
	d_ui->mainLayout->removeWidget(menuItem);
	disconnect(menuItem, &BaseMenuItem::moveItem, this, &BaseMenu::moveItem);
	d_items = d_ui->mainLayout->count();
	menuItem->setVisible(false);

	_updateItemsPosition();
}

void BaseMenu::removeMenuItem(QVector<BaseMenuItem *> items) {
	for (auto const &item : items) {
		removeMenuItem(item);
	}
}

int BaseMenu::positionOf(BaseMenuItem *menuItem) {
	// return the position of a menuItem in the menu
	return d_ui->mainLayout->indexOf(menuItem);
}

void BaseMenu::setMargins(int left, int top, int right, int bottom) {
	d_ui->mainLayout->setContentsMargins(left, top, right, bottom);
}

int BaseMenu::spacing() const { return d_ui->mainLayout->spacing(); }

void BaseMenu::setSpacing(int spacing) { d_ui->mainLayout->setSpacing(spacing); }

void BaseMenu::dragEnterEvent(QDragEnterEvent *event) {
	if (!event->source()) {
		event->ignore();
		return;
	}

	if (!d_ui->widget->geometry().contains(event->pos()) ||
	    !event->mimeData()->hasFormat(BaseMenuItem::menuItemMimeDataType)) {
		event->ignore();
		return;
	}
	d_ui->bottomSeparator->setVisible(true);
	event->accept();
}

void BaseMenu::dragLeaveEvent(QDragLeaveEvent *event) { d_ui->bottomSeparator->setVisible(false); }

void BaseMenu::dropEvent(QDropEvent *event) {
	d_ui->bottomSeparator->setVisible(false);

	if (!event->source()) {
		return;
	}

	if (event->mimeData()->hasFormat(BaseMenuItem::menuItemMimeDataType)) {
		short from = (short)event->mimeData()->data(BaseMenuItem::menuItemMimeDataType)[1];
		moveItem(from, d_items);
	}
}

void BaseMenu::_updateItemsPosition() {
	for (auto &item : findChildren<BaseMenuItem *>()) {
		item->setPosition(positionOf(item));
	}
}

void BaseMenu::moveItem(short from, short to) {
	if (to > from)
		to--;
	QWidget *widget = d_ui->mainLayout->itemAt(from)->widget();
	d_ui->mainLayout->removeWidget(widget);
	d_ui->mainLayout->insertWidget(to, widget);

	_updateItemsPosition();

	Q_EMIT itemMovedFromTo(from, to);
}
