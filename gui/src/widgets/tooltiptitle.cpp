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

#include "tooltiptitle.h"

#include <style.h>

using namespace scopy;

ToolTipTitle::ToolTipTitle(QString title, QString toolTip, QWidget *parent)
	: QWidget{parent}
{
	m_layout = new QHBoxLayout(this);
	m_layout->setMargin(0);
	m_layout->setSpacing(1);
	this->setLayout(m_layout);

	m_title = new QLabel(title, this);
	m_title->setWordWrap(true);
	Style::setStyle(m_title, style::properties::label::subtle);

	m_toolTipIcon = new QLabel(this);
	QPixmap pixmap(":/gui/icons/Frame.svg");
	m_toolTipIcon->setPixmap(pixmap);

	m_toolTipIcon->setVisible(false);

	setToolTip(toolTip);

	m_layout->addWidget(m_title);
	m_layout->addWidget(m_toolTipIcon);
	m_layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Preferred));
}

void ToolTipTitle::setTitle(QString title) { m_title->setText(title); }

void ToolTipTitle::setToolTip(QString toolTip)
{
	if(!toolTip.isEmpty()) {
		m_toolTipIcon->setVisible(true);
		m_toolTipIcon->setToolTip(toolTip);
	}
}

QString ToolTipTitle::getTitle() { return m_title->text(); }

QString ToolTipTitle::getToolTip() { return m_toolTipIcon->toolTip(); }

#include "moc_tooltiptitle.cpp"
