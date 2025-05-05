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

#include "subsection_separator.hpp"

#include "ui_subsection_separator.h"

#include <QHBoxLayout>

using namespace scopy::m2kgui;

SubsectionSeparator::SubsectionSeparator(QWidget *parent)
	: QWidget(parent)
	, m_ui(new Ui::SubsectionSeparator)
{
	m_ui->setupUi(this);

	m_ui->btnSubsectionSeparator->setChecked(true);
	connect(m_ui->btnSubsectionSeparator, &QPushButton::toggled,
		[=](bool toggled) { m_ui->widgetSubsectionContent->setVisible(toggled); });
}

SubsectionSeparator::SubsectionSeparator(const QString &text, const bool buttonVisible, QWidget *parent)
	: SubsectionSeparator(parent)
{
	setLabel(text);
	setButtonVisible(buttonVisible);
}

SubsectionSeparator::~SubsectionSeparator() { delete m_ui; }

QPushButton *SubsectionSeparator::getButton() { return m_ui->btnSubsectionSeparator; }

void SubsectionSeparator::setButtonVisible(bool buttonVisible)
{
	m_ui->btnSubsectionSeparator->setVisible(buttonVisible);
}

bool SubsectionSeparator::getButtonChecked() { return m_ui->btnSubsectionSeparator->isChecked(); }

void SubsectionSeparator::setButtonChecked(bool checked) { m_ui->btnSubsectionSeparator->setChecked(checked); }

QLabel *SubsectionSeparator::getLabel() { return m_ui->lblSubsectionSeparator; }

void SubsectionSeparator::setLabel(const QString &text) { m_ui->lblSubsectionSeparator->setText(text); }

void SubsectionSeparator::setLabelVisible(bool visible) { m_ui->lblSubsectionSeparator->setVisible(visible); }

void SubsectionSeparator::setLineVisible(bool visible) { m_ui->lineSubsectionSeparator->setVisible(visible); }

void SubsectionSeparator::setContent(QWidget *content) { m_ui->vLayoutContent->addWidget(content); }

QWidget *SubsectionSeparator::getContentWidget() { return m_ui->widgetSubsectionContent; }

#include "moc_subsection_separator.cpp"
