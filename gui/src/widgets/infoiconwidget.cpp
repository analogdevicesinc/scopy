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
#include "style.h"

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

QWidget *InfoIconWidget::addInfoToWidget(QWidget *w, QString infoMessage)
{
	InfoIconWidget *info = new InfoIconWidget(infoMessage);

	QWidget *newWidget = new QWidget();
	QHBoxLayout *lay = new QHBoxLayout();
	newWidget->setLayout(lay);
	lay->setSpacing(Style::getDimension(json::global::unit_0_5));
	lay->setMargin(0);
	lay->addWidget(w);
	lay->addWidget(info);

	return newWidget;
}

QWidget *InfoIconWidget::addHoveringInfoToWidget(QWidget *w, QString infoMessage, QWidget *parent, HoverPosition pos)
{
	InfoIconWidget *info = new InfoIconWidget(infoMessage);
	info->layout()->setMargin(5);
	HoverWidget *hover = new HoverWidget(info, w, parent);
	hover->setAnchorPos(pos);
	hover->setContentPos(pos);
	hover->setUpdateVisibility(true);
	hover->setVisible(w->isVisible());

	QObject::connect(w, &QObject::destroyed, hover, &QObject::deleteLater);

	return hover;
}

#include "moc_infoiconwidget.cpp"
