/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "utils.hpp"

#include "ui_base_menu.h"

#include <QMimeData>

#include <scopy/gui/base_menu.hpp>

using namespace scopy::gui;

BaseMenu::BaseMenu(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::BaseMenu)
	, m_items(0)
{
	m_ui->setupUi(this);
	setAcceptDrops(true);

	Util::retainWidgetSizeWhenHidden(m_ui->lineBottomSeparator);
	m_ui->lineBottomSeparator->setVisible(false);
}

BaseMenu::~BaseMenu() { delete m_ui; }

void BaseMenu::insertMenuItem(BaseMenuItem* menuItem, int position)
{
	menuItem->setOwner(this);

	for (int i = 0; i < m_ui->mainLayout->count(); ++i) {
		if (menuItem == m_ui->mainLayout->itemAt(i)->widget()) {
			return;
		}
	}

	m_ui->mainLayout->insertWidget(position, menuItem);
	m_items = m_ui->mainLayout->count();

	menuItem->setVisible(true);

	connect(menuItem, &BaseMenuItem::moveItem, this, &BaseMenu::moveItem);

	_updateItemsPosition();
}

void BaseMenu::insertMenuItem(QVector<BaseMenuItem*> items, QVector<int> positions)
{
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

void BaseMenu::removeMenuItem(BaseMenuItem* menuItem)
{
	m_ui->mainLayout->removeWidget(menuItem);
	disconnect(menuItem, &BaseMenuItem::moveItem, this, &BaseMenu::moveItem);
	m_items = m_ui->mainLayout->count();
	menuItem->setVisible(false);

	_updateItemsPosition();
}

void BaseMenu::removeMenuItem(QVector<BaseMenuItem*> items)
{
	for (auto const& item : items) {
		removeMenuItem(item);
	}
}

int BaseMenu::positionOf(BaseMenuItem* menuItem)
{
	// return the position of a menuItem in the menu
	return m_ui->mainLayout->indexOf(menuItem);
}

void BaseMenu::setMargins(int left, int top, int right, int bottom)
{
	m_ui->mainLayout->setContentsMargins(left, top, right, bottom);
}

int BaseMenu::spacing() const { return m_ui->mainLayout->spacing(); }

void BaseMenu::setSpacing(int spacing) { m_ui->mainLayout->setSpacing(spacing); }

void BaseMenu::dragEnterEvent(QDragEnterEvent* event)
{
	if (!event->source()) {
		event->ignore();
		return;
	}

	if (!m_ui->widget->geometry().contains(event->pos()) ||
	    !event->mimeData()->hasFormat(BaseMenuItem::menuItemMimeDataType)) {
		event->ignore();
		return;
	}
	m_ui->lineBottomSeparator->setVisible(true);
	event->accept();
}

void BaseMenu::dragLeaveEvent(QDragLeaveEvent* event) { m_ui->lineBottomSeparator->setVisible(false); }

void BaseMenu::dropEvent(QDropEvent* event)
{
	m_ui->lineBottomSeparator->setVisible(false);

	if (!event->source()) {
		return;
	}

	if (event->mimeData()->hasFormat(BaseMenuItem::menuItemMimeDataType)) {
		short from = (short)event->mimeData()->data(BaseMenuItem::menuItemMimeDataType)[1];
		moveItem(from, m_items);
	}
}

void BaseMenu::_updateItemsPosition()
{
	for (auto& item : findChildren<BaseMenuItem*>()) {
		item->setPosition(positionOf(item));
	}
}

void BaseMenu::moveItem(short from, short to)
{
	if (to > from)
		to--;
	QWidget* widget = m_ui->mainLayout->itemAt(from)->widget();
	m_ui->mainLayout->removeWidget(widget);
	m_ui->mainLayout->insertWidget(to, widget);

	_updateItemsPosition();

	Q_EMIT itemMovedFromTo(from, to);
}
