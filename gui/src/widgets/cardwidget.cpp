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

#include "cardwidget.h"
#include <style.h>

using namespace scopy;

CardWidget::CardWidget(QWidget *parent)
	: QFrame(parent)
{
	m_layout = new QVBoxLayout(this);
	m_layout->setSpacing(10);

	QFrame *titleW = createTitleW(this);
	m_description = new QLabel(this);
	m_description->setWordWrap(true);

	m_layout->addWidget(titleW);
	m_layout->addWidget(m_description);

	Style::setStyle(this, style::properties::widget::cardWidget);
}

CardWidget::~CardWidget() {}

QLineEdit *CardWidget::title() const { return m_title; }

QLabel *CardWidget::subtitle() const { return m_subtitle; }

QLabel *CardWidget::description() const { return m_description; }

QComboBox *CardWidget::versCb() const { return m_versCb; }

void CardWidget::setId(const QString &newId) { m_id = newId; }

QString CardWidget::id() const { return m_id; }

QFrame *CardWidget::createTitleW(QWidget *parent)
{
	QFrame *w = new QFrame(parent);
	w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	QGridLayout *lay = new QGridLayout(w);
	lay->setMargin(0);
	lay->setSpacing(0);

	m_title = new QLineEdit(w);
	m_title->setReadOnly(true);
	m_title->setEnabled(false);
	m_title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	Style::setStyle(m_title, style::properties::lineedit::headerLineEdit);

	m_subtitle = new QLabel(w);
	Style::setStyle(m_subtitle, style::properties::label::subtle);

	m_versCb = new QComboBox(w);
	m_versCb->setDisabled(true);
	m_versCb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
	Style::setStyle(m_versCb, style::properties::combobox::versionCb);

	lay->addWidget(m_title, 0, 0, 1, Qt::AlignLeft);
	lay->addWidget(m_versCb, 0, 1, 0, Qt::AlignRight);
	lay->addWidget(m_subtitle, 1, 0, Qt::AlignLeft);

	return w;
}

#include "moc_cardwidget.cpp"
