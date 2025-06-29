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

#include "style.h"
#include <dynamicWidget.h>
#include <pluginbase/preferences.h>
#include <stylehelper.h>
#include <widgets/menucontrolbutton.h>

using namespace scopy;

MenuControlButton::MenuControlButton(QWidget *parent)
	: QAbstractButton(parent)
	, m_toolTip(false)
{
	lay = new QHBoxLayout(this);
	lay->setMargin(16);
	lay->setSpacing(10);

	setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	setCheckable(true);
	setLayout(lay);

	m_chk = new QCheckBox("", this);
	m_label = new QLabel("", this);
	m_btn = new QPushButton("", this);
	m_color = Style::getAttribute(json::theme::interactive_primary_idle);
	m_cs = CS_SQUARE;

	lay->addWidget(m_chk);
	lay->addWidget(m_label);
	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	lay->addWidget(m_btn);
	applyStylesheet();

	connect(this, &QAbstractButton::toggled, this, [=](bool b) {
		setDynamicProperty(this, "selected", b);
		Style::setStyle(m_label, style::properties::label::menuMedium, b ? "selected" : "idle");
		if(m_cs == CS_CIRCLE) {
			Style::setStyle(m_chk, style::properties::checkbox::circleCB, b ? "selected" : "idle", true);
		}
	}); // Hackish - QStyle should be implemented

	dblClickToOpenMenu = QMetaObject::Connection();
	openMenuChecksThis = QMetaObject::Connection();
}

MenuControlButton::~MenuControlButton() {}

void MenuControlButton::setColor(QColor c)
{
	m_color = c;
	applyStylesheet();
}

void MenuControlButton::setCheckBoxStyle(CheckboxStyle cs)
{
	m_cs = cs;
	applyStylesheet();
}

void MenuControlButton::setName(QString s)
{
	m_label->setText(s);
	if(m_toolTip) {
		m_label->setToolTip(s);
	}
}

void MenuControlButton::setDoubleClickToOpenMenu(bool b)
{
	if(b) {
		dblClickToOpenMenu = connect(this, &MenuControlButton::doubleClicked, this, [=]() {
			setChecked(true);
			if(m_btn->isVisible()) {
				m_btn->click();
			}
		});
	} else {
		disconnect(dblClickToOpenMenu);
	}
}

void MenuControlButton::setOpenMenuChecksThis(bool b)
{
	if(b) {
		openMenuChecksThis = connect(m_btn, &QAbstractButton::toggled, this, [=](bool b) {
			if(b)
				setChecked(true);
		});
	} else {
		disconnect(openMenuChecksThis);
	}
}

void MenuControlButton::enableToolTip(bool en)
{
	m_toolTip = en;
	m_label->setToolTip(en ? m_label->text() : "");
}

void MenuControlButton::mouseDoubleClickEvent(QMouseEvent *e)
{
	if(e->button() == Qt::LeftButton) {
		if(Preferences::get("general_doubleclick_ctrl_opens_menu").toBool() == true) {
			Q_EMIT doubleClicked();
			return;
		}
	}
	QAbstractButton::mouseDoubleClickEvent(e);
}

void MenuControlButton::mousePressEvent(QMouseEvent *e)
{
	if(e->button() == Qt::LeftButton) {
		if(Preferences::get("general_doubleclick_ctrl_opens_menu").toBool() == true) {
			if(m_btn->isChecked()) {
				m_btn->setChecked(false);
				return;
			}
		}
	}
	QAbstractButton::mousePressEvent(e);
}

QCheckBox *MenuControlButton::checkBox() { return m_chk; }

QPushButton *MenuControlButton::button() { return m_btn; }

QHBoxLayout *MenuControlButton::layout() { return lay; }

void MenuControlButton::applyStylesheet()
{

	StyleHelper::MenuControlWidget(this, m_color, "controlButton");
	switch(m_cs) {
	case CS_CIRCLE:
		StyleHelper::ColoredCircleCheckbox(m_chk, m_color, "chk");
		break;
	case CS_SQUARE:
		StyleHelper::ColoredSquareCheckbox(m_chk, Style::getAttribute(json::theme::content_default), "chk");
		break;
	case CS_COLLAPSE:
		StyleHelper::CollapseCheckbox(m_chk, "chk");
		break;
	default:
		// default style
		break;
	}

	Style::setStyle(m_label, style::properties::label::menuMedium, "idle");
	StyleHelper::MenuControlButton(m_btn, "btn");
}

CollapsableMenuControlButton::CollapsableMenuControlButton(QWidget *parent)
	: QWidget(parent)
{
	m_lay = new QVBoxLayout(this);
	m_lay->setMargin(0);
	m_lay->setSpacing(0);
	setLayout(m_lay);
	m_ctrl = new MenuControlButton(this);
	m_ctrl->enableToolTip(true);
	m_ctrl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_ctrl->setCheckBoxStyle(MenuControlButton::CS_COLLAPSE);
	m_ctrl->setCheckable(false);
	m_ctrl->checkBox()->setChecked(true);
	m_onOffSwitch = new SmallOnOffSwitch(this);
	m_onOffSwitch->hide();
	m_onOffSwitch->setChecked(true);
	m_ctrl->layout()->addWidget(m_onOffSwitch);

	m_lay->addWidget(m_ctrl);
	QWidget *container = new QWidget(this);
	m_lay->addWidget(container);
	m_contLayout = new QVBoxLayout(container);
	container->setLayout(m_contLayout);
	m_contLayout->setMargin(0);
	m_contLayout->setSpacing(0);

	connect(m_ctrl->checkBox(), SIGNAL(toggled(bool)), container, SLOT(setVisible(bool)));
}

CollapsableMenuControlButton::~CollapsableMenuControlButton() {}

void CollapsableMenuControlButton::add(QWidget *ch)
{
	ch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_contLayout->addWidget(ch);
}

void CollapsableMenuControlButton::remove(QWidget *ch) { m_contLayout->removeWidget(ch); }

int CollapsableMenuControlButton::count() { return m_contLayout->count(); }

MenuControlButton *CollapsableMenuControlButton::getControlBtn() { return m_ctrl; }

SmallOnOffSwitch *CollapsableMenuControlButton::onOffSwitch() { return m_onOffSwitch; }

void CollapsableMenuControlButton::enableOnOffSwitch(bool en) { m_onOffSwitch->setVisible(en); }

#include "moc_menucontrolbutton.cpp"
