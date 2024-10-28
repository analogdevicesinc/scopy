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

#include "progresslineedit.h"

using namespace scopy;

ProgressLineEdit::ProgressLineEdit(QWidget *parent)
	: QWidget(parent)
	, m_lineEdit(new QLineEdit(this))
	, m_progressBar(new SmallProgressBar(this))
{
	StyleHelper::MenuLineEdit(m_lineEdit, "lineEdit");
	m_progressBar->setStyleSheet("background-color: grey;");

	setLayout(new QVBoxLayout(this));
	layout()->addWidget(m_lineEdit);
	layout()->addWidget(m_progressBar);
	layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Preferred, QSizePolicy::Expanding));
	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->setSpacing(0);

	connect(m_lineEdit, &QLineEdit::editingFinished, this, [this]() { m_progressBar->startProgress(); });
}

SmallProgressBar *ProgressLineEdit::getProgressBar() { return m_progressBar; }

QLineEdit *ProgressLineEdit::getLineEdit() { return m_lineEdit; }

#include "moc_progresslineedit.cpp"
