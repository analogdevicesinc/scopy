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

#include "pluginenablewidget.h"

#include "qboxlayout.h"

using namespace scopy;

PluginEnableWidget::PluginEnableWidget(QWidget *parent)
	: QWidget(parent)
{
	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setContentsMargins(5, 5, 5, 5);

	m_checkBox = new QCheckBox();
	m_checkBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	m_descriptionLabel = new QLabel();
	m_descriptionLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	m_descriptionLabel->setWordWrap(true);

	layout->addWidget(m_checkBox);
	layout->setAlignment(m_checkBox, Qt::AlignTop);
	layout->addWidget(m_descriptionLabel);
	layout->setAlignment(m_descriptionLabel, Qt::AlignTop);
	layout->setStretch(0, 1);
	layout->setStretch(1, 3);
}

PluginEnableWidget::~PluginEnableWidget() {}

void PluginEnableWidget::setDescription(QString description)
{
	m_descriptionLabel->clear();
	m_descriptionLabel->setText(description);
}

QCheckBox *PluginEnableWidget::checkBox() const { return m_checkBox; }

#include "moc_pluginenablewidget.cpp"
