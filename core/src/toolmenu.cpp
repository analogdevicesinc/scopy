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

#include "toolmenu.h"
#include <QScrollBar>

using namespace scopy;

ToolMenu::ToolMenu(QWidget *parent)
	: QWidget(parent)
	, m_btnGroup(new QButtonGroup(this))
{
	m_uuid = 0;
	QVBoxLayout *lay = new QVBoxLayout();
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

	m_scroll = new QScrollArea(parent);
	QWidget *wScroll = new QWidget(m_scroll);

	m_layScroll = new QVBoxLayout();
	m_layScroll->setMargin(0);
	m_layScroll->setSpacing(10);

	wScroll->setLayout(m_layScroll);
	m_scroll->setWidgetResizable(true);
	m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	m_scroll->setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContents);

	m_scroll->setWidget(wScroll);
	m_scroll->verticalScrollBar()->setVisible(false);

	lay->setMargin(0);
	lay->setSpacing(10);
	setLayout(lay);

	m_spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
	m_layScroll->addSpacerItem(m_spacer);

	lay->addWidget(m_scroll);

	connect(m_scroll->verticalScrollBar(), &QScrollBar::rangeChanged, this, &ToolMenu::onScrollRangeChanged);
}

ToolMenu::~ToolMenu() {}

void ToolMenu::add(QWidget *w)
{
	int spacerIndex = m_layScroll->indexOf(m_spacer);
	m_layScroll->insertWidget(spacerIndex, w);
	m_uuid++;
}

void ToolMenu::add(int index, QString itemId, QWidget *w)
{
	m_widgetMap.insert(itemId, w);
	if(index < 0) {
		add(w);
	} else {
		add(index, w);
	}
}

void ToolMenu::add(int index, QWidget *w)
{
	m_layScroll->insertWidget(index, w);
	m_uuid++;
}
void ToolMenu::remove(QWidget *w)
{
	m_widgetMap.remove(widgetName(w));
	m_layScroll->removeWidget(w);
}

int ToolMenu::indexOf(QWidget *w) { return m_layScroll->indexOf(w); }

void ToolMenu::colapseAll()
{
	for(QWidget *w : qAsConst(m_widgetMap)) {
		Collapsable *c = dynamic_cast<Collapsable *>(w);
		if(c != nullptr) {
			c->setCollapsed(true);
		}
	}
}

QButtonGroup *ToolMenu::btnGroup() const { return m_btnGroup; }

QString ToolMenu::widgetName(QWidget *w) { return m_widgetMap.key(w, ""); }

// Used to display the scrollbar when needed and to maintain its size in the scroll area when not needed.
// We chose this approach because for the Qt::ScrollBarAsNeeded policy the size of the scrollball cannot be retained
// with Util::retainWidgetSizeWhenHidden because the resizing of the scrollbar is done dynamically (withoud using the
// show/hide default functions).
void ToolMenu::onScrollRangeChanged(int min, int max)
{
	if(max > min) {
		m_scroll->verticalScrollBar()->setVisible(true);
	} else {
		m_scroll->verticalScrollBar()->setVisible(false);
	}
}

#include "moc_toolmenu.cpp"
