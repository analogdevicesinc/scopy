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

#include "infoheaderwidget.h"

#include <QApplication>
#include <style.h>
#include <stylehelper.h>

using namespace scopy;

InfoHeaderWidget::InfoHeaderWidget(QString title, QWidget *parent)
	: QWidget(parent)
{
	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setSpacing(0);
	layout->setMargin(0);

	m_infoBtn = new InfoBtn(this);
	m_infoBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	setupDescriptionButton();

	m_titleEdit = new QLineEdit(title, this);
	m_titleEdit->setEnabled(false);
	m_titleEdit->setReadOnly(false);
	Style::setStyle(m_titleEdit, style::properties::lineedit::headerLineEdit);
	m_titleEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	layout->addWidget(m_infoBtn);
	layout->addWidget(m_titleEdit);
	layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
}

InfoHeaderWidget::~InfoHeaderWidget() {}

void InfoHeaderWidget::setTitle(QString title) { m_titleEdit->setText(title); }

QString InfoHeaderWidget::title() { return m_titleEdit->text(); }

void InfoHeaderWidget::setDescription(const QString &newDescription) { m_description = newDescription; }

InfoBtn *InfoHeaderWidget::infoBtn() const { return m_infoBtn; }

QLineEdit *InfoHeaderWidget::titleEdit() const { return m_titleEdit; }

void InfoHeaderWidget::setupDescriptionButton()
{
	Style::setStyle(m_infoBtn, style::properties::button::squareIconButton, false);
	Style::setStyle(m_infoBtn, style::properties::button::smallSquareIconButton);
	Style::setBackgroundColor(m_infoBtn, json::theme::background_primary, true);

	connect(m_infoBtn, &InfoBtn::clicked, this, [this](bool b) {
		QWidget *parentWidget = QApplication::activeWindow();
		PopupWidget *popup = new PopupWidget(parentWidget);
		popup->enableCenterOnParent(true);
		popup->setTitle(m_titleEdit->text());
		popup->setDescription(m_description);
		popup->getExitBtn()->hide();
		popup->getContinueBtn()->hide();
		popup->enableCloseButton(true);
		popup->enableTintedOverlay(true);
		popup->show();
		popup->raise();
	});
}

#include "moc_infoheaderwidget.cpp"
