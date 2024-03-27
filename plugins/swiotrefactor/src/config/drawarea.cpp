/*
 * Copyright (c) 2023 Analog Devices Inc.
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

#include "drawarea.h"

#include <QFile>
#include <QPainter>
#include <QPixmap>
#include <QVBoxLayout>
#include <QtXml/QDomElement>

using namespace scopy::swiotrefactor;

DrawArea::DrawArea(QWidget *parent)
	: QWidget(parent)
	, m_filePath(":/swiotrefactor/swiot_board.svg")
	, m_drawLabel(new QLabel(this))
	, m_baseImage(new QImage(m_filePath))
	, m_connectionsMap(new QMap<QPair<int, int>, QPixmap *>())
{
	this->setLayout(new QVBoxLayout(this));
	this->setStyleSheet("background-color: black;");

	// insert, for each channel, the 2 PixMaps, one for ad74413r and one for max14906
	for(int i = 1; i <= 4; ++i) {
		m_connectionsMap->insert({i, AD74413R},
					 new QPixmap(":/swiotrefactor/ad74413r_channel" + QString::number(i) + ".png"));
		m_connectionsMap->insert({i, MAX14906},
					 new QPixmap(":/swiotrefactor/max14906_channel" + QString::number(i) + ".png"));
	}

	m_boardImage = new QImage(m_filePath);
	m_drawLabel->setPixmap(QPixmap::fromImage(*m_boardImage));
	m_drawLabel->setStyleSheet("border: 20px solid #272730;");
	this->layout()->addWidget(m_drawLabel);
}

DrawArea::~DrawArea() { delete m_connectionsMap; }

void DrawArea::activateConnection(int channelIndex, DrawArea::ChannelName channelName)
{
	QPainter painter(m_boardImage);
	painter.drawPixmap(0, 0, *m_connectionsMap->value(qMakePair(channelIndex, channelName)));
	m_drawLabel->setPixmap(QPixmap::fromImage(*m_boardImage));
}

void DrawArea::deactivateConnections()
{
	m_boardImage = new QImage(*m_baseImage);
	m_drawLabel->setPixmap(QPixmap::fromImage(*m_baseImage));
}
