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

#include <QVBoxLayout>
#include <style.h>

#include <widgets/menucombo.h>

using namespace scopy;

MenuComboWidget::MenuComboWidget(QString title, QWidget *parent)
	: QWidget(parent)
{
	auto lay = new QVBoxLayout(this);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setLayout(lay);
	lay->setSpacing(0);
	lay->setMargin(0);

	m_combo = new MenuCombo(title, this);

	lay->addWidget(m_combo);
}

MenuComboWidget::~MenuComboWidget() {}

QComboBox *MenuComboWidget::combo() { return m_combo->combo(); }

QString MenuComboWidget::title() const { return m_combo->title(); }

void MenuComboWidget::setTitle(const QString &newTitle) { m_combo->setTitle(newTitle); }

void MenuComboWidget::setInfoMessage(QString infoMessage) { m_combo->setInfoMessage(infoMessage); }

MenuCombo::MenuCombo(QString title, QWidget *parent)
	: QWidget(parent)
{
	auto lay = new QVBoxLayout(this);
	m_mouseWheelGuard = new MouseWheelWidgetGuard(this);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setLayout(lay);
	lay->setSpacing(0);
	lay->setMargin(0);

	m_label = new QLabel(title, this);
	Style::setStyle(m_label, style::properties::label::subtle);

	m_infoIcon = new InfoIconWidget("", parent);

	m_combo = new QComboBox(this);

	QHBoxLayout *labelLayout = new QHBoxLayout();
	labelLayout->setContentsMargins(0, 0, 0, 0);
	labelLayout->setSpacing(5);

	labelLayout->addWidget(m_label);
	labelLayout->addWidget(m_infoIcon);
	labelLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Preferred));

	lay->addLayout(labelLayout);
	lay->addWidget(m_combo);
	m_mouseWheelGuard->installEventRecursively(this);
}

MenuCombo::~MenuCombo() {}
QComboBox *MenuCombo::combo() { return m_combo; }

QString MenuCombo::title() const { return m_label->text(); }

void MenuCombo::setTitle(const QString &newTitle) { m_label->setText(newTitle); }

void MenuCombo::setInfoMessage(QString infoMessage) { m_infoIcon->setInfoMessage(infoMessage); }

#include "moc_menucombo.cpp"
