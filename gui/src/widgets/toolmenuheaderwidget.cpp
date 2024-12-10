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
	: QWidget(parent)
	, m_ledState(IDLE)
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

	m_refreshTimer = new QTimer(this);
	m_refreshTimer->setInterval(LED_ON_MSEC * WAITING_FACTOR);
	connect(m_refreshTimer, &QTimer::timeout, this, &ToolMenuHeaderWidget::refresh);

	m_timer = new QTimer(this);
	m_timer->setInterval(LED_ON_MSEC);
	connect(m_timer, &QTimer::timeout, this, &ToolMenuHeaderWidget::onTimeout);

	connect(this, &ToolMenuHeaderWidget::blinkLed, this, &ToolMenuHeaderWidget::onBlinkLed);
	connect(this, &ToolMenuHeaderWidget::connState, this,
		[this, parent](QString id, bool isConnected) { setState(id, isConnected, parent); });
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

void ToolMenuHeaderWidget::setDeviceId(QString deviceId) { m_id = deviceId; }

// To be implemented after IIOManager is available
void ToolMenuHeaderWidget::onBlinkLed(int retCode) {}

void ToolMenuHeaderWidget::handleSingle(bool isSuccess)
{
	if(m_ledState != IDLE && m_ledState != STREAM_RUNNING) {
		return;
	}
	if(m_ledState == IDLE) {
		m_ledBtn->ledOn();
		m_ledState = SINGLE_RUNNING;
	} else {
		m_ledBtn->ledOff();
		m_ledState = SINGLE_INTERRUPT;
	}
	m_ledBtn->setLedState(isSuccess);
	m_timer->start();
}

void ToolMenuHeaderWidget::handleStream(bool isSuccess)
{
	if(m_ledState != STREAM_RUNNING && m_ledState != IDLE) {
		return;
	}
	m_ledState = STREAM_RUNNING;
	m_ledBtn->setLedState(isSuccess);
	m_ledBtn->ledOn();
	m_timer->start();
}

void ToolMenuHeaderWidget::onTimeout()
{
	switch(m_ledState) {
	case SINGLE_INTERRUPT:
		m_ledBtn->ledOn();
		m_ledState = SINGLE_RUNNING;
		break;
	case IDLE:
		m_ledBtn->ledOff();
		m_timer->stop();
		break;
	default:
		m_ledState = IDLE;
		break;
	}
}

void ToolMenuHeaderWidget::refresh()
{
	m_refreshTimer->stop();
	m_ledBtn->ledOff();
	m_timer->start();
}

QPushButton *ToolMenuHeaderWidget::deviceBtn() const { return m_deviceBtn; }

void ToolMenuHeaderWidget::setState(QString id, bool state, QWidget *parent)
{
	if(m_id == id) {
		Style::setStyle(parent, style::properties::widget::ledBorder, state);
	}
}

#include "moc_toolmenuheaderwidget.cpp"
