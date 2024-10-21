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

#include "tooltemplate.h"

#include "ui_tooltemplate.h"

#include <style.h>

using namespace scopy;
ToolTemplate::ToolTemplate(QWidget *parent)
	: QWidget(parent)
{
	m_ui = new Ui::ToolTemplate();
	m_ui->setupUi(this);
	setStyleSheet(".QWidget { background-color: " + Style::getAttribute(json::theme::background_subtle) + "; }");

	m_ui->bottomCentral->setVisible(false);
	m_ui->topCentral->setVisible(false);
	m_ui->bottomContainer->setVisible(false);
	m_ui->topContainer->setVisible(false);
	m_ui->leftContainer->setVisible(false);
	m_ui->rightContainer->setVisible(false);
	m_leftStack = new MapStackedWidget(m_ui->leftContainer);
	m_rightStack = new MapStackedWidget(m_ui->rightContainer);
	m_topStack = new MapStackedWidget(m_ui->topCentral);
	m_bottomStack = new MapStackedWidget(m_ui->bottomCentral);

	m_ui->leftContainer->layout()->addWidget(m_leftStack);
	m_ui->rightContainer->layout()->addWidget(m_rightStack);
	m_ui->topCentral->layout()->addWidget(m_topStack);
	m_ui->bottomCentral->layout()->addWidget(m_bottomStack);
	m_leftStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_rightStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_topStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_bottomStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

ToolTemplate::~ToolTemplate() { delete m_ui; }

QWidget *ToolTemplate::bottomContainer() { return m_ui->bottomContainer; }

QWidget *ToolTemplate::topContainer() { return m_ui->topContainer; }

QWidget *ToolTemplate::topCentral() { return m_ui->topCentral; }

QWidget *ToolTemplate::bottomCentral() { return m_ui->bottomCentral; }

QWidget *ToolTemplate::rightContainer() { return m_ui->rightContainer; }

QWidget *ToolTemplate::topContainerMenuControl() { return m_ui->topContainerMenuControl; }

QWidget *ToolTemplate::centralContainer() { return m_ui->centralContainer; }

MapStackedWidget *ToolTemplate::leftStack() { return m_leftStack; }

MapStackedWidget *ToolTemplate::rightStack() { return m_rightStack; }

MapStackedWidget *ToolTemplate::topStack() { return m_topStack; }

MapStackedWidget *ToolTemplate::bottomStack() { return m_bottomStack; }

QWidget *ToolTemplate::leftContainer() { return m_ui->leftContainer; }

void ToolTemplate::setLeftContainerWidth(int w)
{
	dynamic_cast<MenuHAnim *>(m_ui->leftContainer)->setAnimMin(0);
	dynamic_cast<MenuHAnim *>(m_ui->leftContainer)->setAnimMax(w);
	m_ui->leftContainer->setMaximumWidth(w);
	m_ui->leftContainer->setMinimumWidth(w);
}

void ToolTemplate::setRightContainerWidth(int w)
{
	m_ui->topContainerMenuControl->setMaximumWidth(w);
	m_ui->topContainerMenuControl->setMinimumWidth(w);
	dynamic_cast<MenuHAnim *>(m_ui->rightContainer)->setAnimMin(0);
	dynamic_cast<MenuHAnim *>(m_ui->rightContainer)->setAnimMax(w);
	m_ui->rightContainer->setMaximumWidth(w);
	m_ui->rightContainer->setMinimumWidth(w);
}

void ToolTemplate::setTopContainerHeight(int h)
{
	dynamic_cast<MenuVAnim *>(m_ui->topCentral)->setAnimMin(0);
	dynamic_cast<MenuVAnim *>(m_ui->topCentral)->setAnimMax(h);
	m_ui->topCentral->setMaximumHeight(h);
	m_ui->topCentral->setMinimumHeight(h);
}

void ToolTemplate::setBottomContainerHeight(int h)
{
	dynamic_cast<MenuVAnim *>(m_ui->bottomCentral)->setAnimMin(0);
	dynamic_cast<MenuVAnim *>(m_ui->bottomCentral)->setAnimMax(h);
	m_ui->bottomCentral->setMaximumHeight(h);
	m_ui->bottomCentral->setMinimumHeight(h);
}

void ToolTemplate::openLeftContainerHelper(bool open) { m_ui->leftContainer->toggleMenu(open); }

void ToolTemplate::openRightContainerHelper(bool open) { m_ui->rightContainer->toggleMenu(open); }

void ToolTemplate::openTopContainerHelper(bool open) { m_ui->topCentral->toggleMenu(open); }

void ToolTemplate::openBottomContainerHelper(bool open) { m_ui->bottomCentral->toggleMenu(open); }

QSpacerItem *ToolTemplate::getContainerSpacer(QWidget *container)
{
	if(m_ui->topContainer == container)
		return m_ui->topContainerSpacer;
	else if(m_ui->topContainerMenuControl == container)
		return m_ui->topContainerMenuControlSpacer;
	else if(m_ui->bottomContainer == container)
		return m_ui->bottomContainerSpacer;
	else
		return nullptr;
}

void ToolTemplate::addWidgetToTopContainerHelper(QWidget *w, ToolTemplateAlignment a)
{
	auto lay1 = dynamic_cast<QHBoxLayout *>(topContainer()->layout());
	Q_ASSERT(lay1 != nullptr);

	auto idx = lay1->indexOf(m_ui->topContainerSpacer);
	int offset;
	if(a == TTA_LEFT) {
		lay1->insertWidget(idx, w);
	} else {
		lay1->insertWidget(-1, w);
	}
}

void ToolTemplate::addWidgetToTopContainerMenuControlHelper(QWidget *w, ToolTemplateAlignment a)
{
	auto lay1 = dynamic_cast<QHBoxLayout *>(topContainerMenuControl()->layout());
	Q_ASSERT(lay1 != nullptr);
	auto idx = lay1->indexOf(m_ui->topContainerMenuControlSpacer);
	int offset;
	if(a == TTA_LEFT) {
		lay1->insertWidget(idx, w);
	} else {
		lay1->insertWidget(-1, w);
	}
}

void ToolTemplate::addWidgetToBottomContainerHelper(QWidget *w, ToolTemplateAlignment a)
{
	auto lay1 = dynamic_cast<QHBoxLayout *>(bottomContainer()->layout());
	Q_ASSERT(lay1 != nullptr);
	auto idx = lay1->indexOf(m_ui->bottomContainerSpacer);
	int offset;
	if(a == TTA_LEFT) {
		lay1->insertWidget(idx, w);
	} else {
		lay1->insertWidget(-1, w);
	}
}

void ToolTemplate::addWidgetToCentralContainerHelper(QWidget *w)
{
	auto lay1 = dynamic_cast<QVBoxLayout *>(centralContainer()->layout());
	Q_ASSERT(lay1 != nullptr);

	lay1->addWidget(w);
}

void ToolTemplate::requestMenu(QString key)
{
	if(m_leftStack->contains(key)) {
		m_leftStack->show(key);
		return;
	}

	if(m_rightStack->contains(key)) {
		m_rightStack->show(key);
		return;
	}

	if(m_topStack->contains(key)) {
		m_topStack->show(key);
		return;
	}

	if(m_bottomStack->contains(key)) {
		m_bottomStack->show(key);
		return;
	}
}
