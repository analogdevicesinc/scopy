/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#include "infoiconwidget.h"

using namespace scopy;

InfoIconWidget::InfoIconWidget(QString infoMessage, QWidget *parent)
	: QWidget{parent}
{
	m_layout = new QVBoxLayout(this);
	m_layout->setMargin(0);
	m_layout->setSpacing(0);
	this->setLayout(m_layout);

	m_infoIcon = new QLabel(this);
	QPixmap pixmap(":/gui/icons/info.svg");
	m_infoIcon->setPixmap(pixmap);
	m_layout->addWidget(m_infoIcon);

	m_infoIcon->setVisible(false);

	setInfoMessage(infoMessage);
}

void InfoIconWidget::setInfoMessage(QString infoMessage)
{
	if(!infoMessage.isEmpty()) {
		m_infoIcon->setVisible(true);
		m_infoIcon->setToolTip(infoMessage);
	} else {
		m_infoIcon->setVisible(false);
	}
}

QString InfoIconWidget::getInfoMessage() { return m_infoIcon->toolTip(); }

#include "moc_infoiconwidget.cpp"
