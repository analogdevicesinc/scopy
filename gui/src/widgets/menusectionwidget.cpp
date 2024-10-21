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

#include <style.h>
#include <stylehelper.h>
#include <widgets/menusectionwidget.h>

using namespace scopy;
MenuSectionWidget::MenuSectionWidget(QWidget *parent)
	: QWidget(parent)
{
	m_layout = new QVBoxLayout(this);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	setLayout(m_layout);
	m_layout->setSpacing(0);
	m_layout->setMargin(0);
	StyleHelper::MenuSectionWidget(this, "menuSection");
	Style::setStyle(this, style::properties::widget::border);
}

MenuSectionWidget::~MenuSectionWidget() {}

QVBoxLayout *MenuSectionWidget::contentLayout() const { return m_layout; }

MenuVScrollArea::MenuVScrollArea(QWidget *parent)
{
	QWidget *w = new QWidget(this);
	m_layout = new QVBoxLayout(w);
	setWidget(w);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	setLayout(m_layout);
	m_layout->setSpacing(0);
	m_layout->setMargin(0);
}

MenuVScrollArea::~MenuVScrollArea() {}

QVBoxLayout *MenuVScrollArea::contentLayout() const { return m_layout; }

MenuSectionCollapseWidget::MenuSectionCollapseWidget(QString title, MenuCollapseSection::MenuHeaderCollapseStyle style,
						     MenuCollapseSection::MenuHeaderWidgetType headerType,
						     QWidget *parent)
	: QWidget(parent)
{
	m_layout = new QVBoxLayout(this);
	m_layout->setSpacing(0);
	m_layout->setMargin(0);
	setLayout(m_layout);
	m_section = new MenuSectionWidget(parent);
	m_collapse = new MenuCollapseSection(title, style, headerType, m_section);
	m_layout->addWidget(m_section);
	m_section->contentLayout()->addWidget(m_collapse);
}

MenuSectionCollapseWidget::~MenuSectionCollapseWidget() {}

QVBoxLayout *MenuSectionCollapseWidget::contentLayout() const { return m_collapse->contentLayout(); }

void MenuSectionCollapseWidget::add(QWidget *w) { m_collapse->contentLayout()->addWidget(w); }

void MenuSectionCollapseWidget::remove(QWidget *w) { m_collapse->contentLayout()->removeWidget(w); }

bool MenuSectionCollapseWidget::collapsed() { return !m_collapse->header()->isChecked(); }

void MenuSectionCollapseWidget::setCollapsed(bool b) { m_collapse->header()->setChecked(!b); }

MenuCollapseSection *MenuSectionCollapseWidget::collapseSection() { return m_collapse; }
#include "moc_menusectionwidget.cpp"
