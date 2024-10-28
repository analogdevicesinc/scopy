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

#include <widgets/verticalchannelmanager.h>
#include <QScrollArea>

using namespace scopy;
VerticalChannelManager::VerticalChannelManager(QWidget *parent)
	: QWidget(parent)
{
	lay = new QVBoxLayout(this);
	lay->setMargin(0);
	lay->setSpacing(0);

	setLayout(lay);
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

	spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Expanding);

	QWidget *m_container = new QWidget(this);
	m_contLayout = new QVBoxLayout(m_container);
	m_contLayout->addSpacerItem(spacer);
	m_contLayout->setMargin(0);
	m_contLayout->setSpacing(6);
	m_container->setLayout(m_contLayout);

	m_scrollArea = new QScrollArea(this);
	m_scrollArea->setWidget(m_container);
	m_scrollArea->setWidgetResizable(true);
	m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	lay->addWidget(m_scrollArea);
}

VerticalChannelManager::~VerticalChannelManager() {}

void VerticalChannelManager::add(QWidget *ch)
{
	int position = m_contLayout->indexOf(spacer);
	m_contLayout->insertWidget(position, ch);
	ch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void VerticalChannelManager::addEnd(QWidget *ch)
{
	m_contLayout->addWidget(ch);
	ch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void VerticalChannelManager::remove(QWidget *ch) { m_contLayout->removeWidget(ch); }

#include "moc_verticalchannelmanager.cpp"
