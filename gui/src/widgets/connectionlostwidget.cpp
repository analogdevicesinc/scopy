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
 */

#include "widgets/connectionlostwidget.h"
#include <QHBoxLayout>
#include <style.h>

using namespace scopy;

ConnectionLostWidget::ConnectionLostWidget(QWidget *parent)
	: QWidget(parent)
{
	setupUi("Connection to device lost!");
}

ConnectionLostWidget::ConnectionLostWidget(const QString &message, QWidget *parent)
	: QWidget(parent)
{
	setupUi(message);
}

ConnectionLostWidget::~ConnectionLostWidget() {}

void ConnectionLostWidget::setupUi(const QString &message)
{
	setFixedHeight(30);

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(10);

	m_warningLabel = new QLabel(message, this);
	m_disconnectBtn = new QPushButton("Disconnect", this);
	m_disconnectBtn->setFixedHeight(24);
	m_disconnectBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	Style::setStyle(m_disconnectBtn, style::properties::button::basicButton);

	layout->addWidget(m_warningLabel);
	layout->addWidget(m_disconnectBtn);

	connect(m_disconnectBtn, &QPushButton::clicked, this, &ConnectionLostWidget::disconnectClicked);
}

void ConnectionLostWidget::setMessage(const QString &message) { m_warningLabel->setText(message); }

void ConnectionLostWidget::setButtonText(const QString &text) { m_disconnectBtn->setText(text); }

QPushButton *ConnectionLostWidget::disconnectButton() const { return m_disconnectBtn; }

#include "moc_connectionlostwidget.cpp"
