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

#include "epromconfigtool.h"

#include <QVBoxLayout>

#include <style.h>

using namespace scopy;

EpromConfigTool::EpromConfigTool(QWidget *parent)
	: QWidget(parent)
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->topContainerMenuControl()->setVisible(false);
	mainLayout->addWidget(m_tool);

	m_syncBtn = new QPushButton("Sync", this);
	Style::setStyle(m_syncBtn, style::properties::button::basicButton);
	m_tool->addWidgetToTopContainerHelper(m_syncBtn, TTA_LEFT);

	m_configureBtn = new QPushButton("Configure", this);
	Style::setStyle(m_configureBtn, style::properties::button::basicButton);
	m_configureBtn->setEnabled(false); // nothing to apply before Sync
	m_tool->addWidgetToTopContainerHelper(m_configureBtn, TTA_LEFT);

	// simulates the board behind the dongle being replaced, without disconnecting
	m_switchDeviceBtn = new QPushButton("Switch device", this);
	Style::setStyle(m_switchDeviceBtn, style::properties::button::basicButton);
	m_tool->addWidgetToTopContainerHelper(m_switchDeviceBtn, TTA_RIGHT);

	QWidget *central = new QWidget(this);
	QVBoxLayout *centralLayout = new QVBoxLayout(central);
	centralLayout->setContentsMargins(0, 0, 0, 0);

	m_board = new QLabel(central);
	Style::setStyle(m_board, style::properties::label::menuBig);
	centralLayout->addWidget(m_board);

	m_status = new QLabel("Press Sync to read the EEPROM of the board on the dongle.", central);
	centralLayout->addWidget(m_status);

	m_structureView = new QPlainTextEdit(central);
	m_structureView->setReadOnly(true);
	m_structureView->setPlaceholderText("No configuration synced");
	centralLayout->addWidget(m_structureView);

	m_tool->addWidgetToCentralContainerHelper(central);

	connect(m_syncBtn, &QPushButton::clicked, this, &EpromConfigTool::syncRequested);
	connect(m_configureBtn, &QPushButton::clicked, this, &EpromConfigTool::configureRequested);
	connect(m_switchDeviceBtn, &QPushButton::clicked, this, &EpromConfigTool::switchDeviceRequested);
}

void EpromConfigTool::setStructure(const QString &text) { m_structureView->setPlainText(text); }

void EpromConfigTool::setStatus(const QString &text) { m_status->setText(text); }

void EpromConfigTool::setConfigureEnabled(bool enabled) { m_configureBtn->setEnabled(enabled); }

void EpromConfigTool::setBoard(const QString &board) { m_board->setText("Connected board: " + board); }

#include "moc_epromconfigtool.cpp"
