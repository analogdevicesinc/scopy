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

#include "baseheaderwidget.h"
#include <smallOnOffSwitch.h>
#include <style.h>
#include <widgets/menucollapsesection.h>
#include <QLoggingCategory>
#include <compositeheaderwidget.h>

Q_LOGGING_CATEGORY(CAT_MENU_COLLAPSE_SECTION, "MenuCollapseSection")

using namespace scopy;

MenuCollapseHeader::MenuCollapseHeader(QString title, MenuCollapseSection::MenuHeaderCollapseStyle style,
				       MenuCollapseSection::MenuHeaderWidgetType headerType, QWidget *parent)
	: QAbstractButton(parent)
{
	lay = new QHBoxLayout(this);
	lay->setMargin(0);
	lay->setContentsMargins(0, 2, 0, 6);
	lay->setSpacing(0);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	setCheckable(true);
	setLayout(lay);

	switch(headerType) {
	case MenuCollapseSection::MHW_BASEWIDGET:
		m_headerWidget = new BaseHeaderWidget(title, this);
		break;
	case MenuCollapseSection::MHW_COMPOSITEWIDGET:
		m_headerWidget = new CompositeHeaderWidget(title, this);
		break;
	}

	switch(style) {
	case MenuCollapseSection::MHCW_ARROW:
		m_ctrl = new QCheckBox(this);
		StyleHelper::CollapseCheckbox(dynamic_cast<QCheckBox *>(m_ctrl), "menuCollapseButton");
		connect(this, &QAbstractButton::toggled, this, [=](bool b) { m_ctrl->setChecked(b); });
		m_ctrl->setChecked(true);
		break;
	case MenuCollapseSection::MHCW_ONOFF:
		m_ctrl = new SmallOnOffSwitch(this);
		StyleHelper::MenuOnOffSwitchButton(dynamic_cast<SmallOnOffSwitch *>(m_ctrl), "menuCollapseButton");
		connect(this, &QAbstractButton::toggled, [=](bool b) { m_ctrl->setChecked(b); });
		m_ctrl->setChecked(true);
		break;
	default:
		m_ctrl = new QCheckBox(this);
		m_ctrl->setVisible(false);
		break;
	}

	setChecked(true);
	m_ctrl->setAttribute(Qt::WA_TransparentForMouseEvents);

	lay->addWidget(m_headerWidget);
	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Maximum));
	lay->addWidget(m_ctrl);
}

MenuCollapseHeader::~MenuCollapseHeader() {}

QString MenuCollapseHeader::title()
{
	BaseHeader *baseHeader = dynamic_cast<BaseHeader *>(m_headerWidget);
	if(!baseHeader) {
		qDebug(CAT_MENU_COLLAPSE_SECTION) << "Header widget doesn't implement the BaseHeader interface!";
		return "";
	}
	return baseHeader->title();
}

QWidget *MenuCollapseHeader::headerWidget() const { return m_headerWidget; }

MenuCollapseSection::MenuCollapseSection(QString title, MenuCollapseSection::MenuHeaderCollapseStyle style,
					 MenuCollapseSection::MenuHeaderWidgetType headerType, QWidget *parent)
	: QWidget(parent)
	, m_title(title)
{
	StyleHelper::MenuCollapseSection(this, "menuCollapse");
	m_lay = new QVBoxLayout(this);
	m_lay->setMargin(0);
	m_lay->setSpacing(0);
	setLayout(m_lay);
	m_header = new MenuCollapseHeader(m_title, style, headerType, this);
	m_lay->addWidget(m_header);

	QFrame *line = new QFrame();
	StyleHelper::MenuHeaderLine(line, QPen(Style::getColor(json::theme::interactive_subtle_idle)));
	m_lay->addWidget(line);
	m_lay->addSpacing(Style::getDimension(json::global::unit_1));

	QWidget *container = new QWidget(this);
	m_lay->addWidget(container);
	m_contLayout = new QVBoxLayout(container);
	container->setLayout(m_contLayout);
	m_contLayout->setMargin(0);
	m_contLayout->setSpacing(0);

	connect(m_header, &QAbstractButton::toggled, container, &QWidget::setVisible);
}

MenuCollapseSection::~MenuCollapseSection() {}
QAbstractButton *MenuCollapseSection::header() { return m_header; }

QVBoxLayout *MenuCollapseSection::contentLayout() const { return m_contLayout; }

QString MenuCollapseSection::title() { return m_title; }

void MenuCollapseSection::setTitle(QString s)
{
	m_title = s;
	BaseHeader *baseHeader = dynamic_cast<BaseHeader *>(m_header->headerWidget());
	if(baseHeader) {
		baseHeader->setTitle(s);
	}
}

#include "moc_menucollapsesection.cpp"
