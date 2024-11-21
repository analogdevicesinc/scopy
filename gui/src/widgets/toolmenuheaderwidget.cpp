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

#include "toolmenuheaderwidget.h"

#include <style.h>
#include <stylehelper.h>

using namespace scopy;

ToolMenuHeaderWidget::ToolMenuHeaderWidget(QString title, QWidget *parent)
{
	QHBoxLayout *hLay = new QHBoxLayout(this);
	hLay->setMargin(0);
	hLay->setSpacing(10);

	QWidget *titleWidget = new QWidget(this);
	QVBoxLayout *titleLay = new QVBoxLayout(titleWidget);
	titleLay->setMargin(0);
	titleLay->setSpacing(0);

	m_title = new QLineEdit(title, this);
	m_title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_title->setEnabled(false);
	m_title->setReadOnly(false);
	StyleHelper::MenuCollapseHeaderLineEdit(m_title, "menuCollapseLabel");

	m_uriLabel = new QLabel(titleWidget);
	titleLay->addWidget(m_title);
	titleLay->addWidget(m_uriLabel);

	m_ledBtn = new LedButton(this);
	m_deviceBtn = new QPushButton(this);
	m_deviceBtn->setCheckable(true);

	int offset = Style::getDimension(json::global::unit_0_5) / 2;
	HoverWidget *ledHover = new HoverWidget(m_ledBtn, m_deviceBtn, this);
	ledHover->setStyleSheet("background-color: transparent; border: 0px;");
	ledHover->setAnchorPos(HoverPosition::HP_BOTTOMRIGHT);
	ledHover->setContentPos(HoverPosition::HP_TOPLEFT);
	ledHover->setVisible(true);
	ledHover->setAnchorOffset({offset, offset});
	ledHover->raise();

	hLay->addWidget(m_deviceBtn);
	hLay->addWidget(titleWidget);
	hLay->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

	m_timer = new QTimer(this);
	m_timer->setInterval(LED_ON_MSEC * WAITING_FACTOR);
	connect(m_timer, &QTimer::timeout, m_timer, &QTimer::stop);

	connect(this, &ToolMenuHeaderWidget::blinkLed, this, &ToolMenuHeaderWidget::onBlinkLed);
}

ToolMenuHeaderWidget::~ToolMenuHeaderWidget() {}

void ToolMenuHeaderWidget::setTitle(QString title) { m_title->setText(title); }

QString ToolMenuHeaderWidget::title() { return m_title->text(); }

void ToolMenuHeaderWidget::setDeviceIcon(QPixmap icon)
{
	m_deviceBtn->setIcon(QIcon(icon));
	m_deviceBtn->setIconSize(
		QSize(Style::getDimension(json::global::unit_2), Style::getDimension(json::global::unit_2)));
	m_deviceBtn->setFixedSize(
		QSize(Style::getDimension(json::global::unit_2), Style::getDimension(json::global::unit_2)));
}

void ToolMenuHeaderWidget::setUri(QString uri)
{
	m_uriLabel->clear();
	m_uriLabel->setText(uri);
}

void ToolMenuHeaderWidget::onBlinkLed(int retCode, IIOCallType type)
{
	switch(type) {
	case IIOCallType::SINGLE:
		m_timer->stop();
		break;
	case IIOCallType::STREAM:
		if(m_timer->isActive()) {
			return;
		}
		m_timer->start();
		break;
	default:
		break;
	}
	m_ledBtn->ledOn(retCode >= 0, LED_ON_MSEC);
}

#include "moc_toolmenuheaderwidget.cpp"
