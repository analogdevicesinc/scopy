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
 */

#include "guistrategy/editableguistrategy.h"
#include <gui/style.h>

using namespace scopy;

EditableGuiStrategy::EditableGuiStrategy(IIOWidgetFactoryRecipe recipe, bool isCompact, QWidget *parent)
	: QObject(parent)
	, m_ui(new QWidget(parent))
	, m_lineEdit(new MenuLineEdit(m_ui))
{
	m_recipe = recipe;
	m_title = new QLabel(recipe.data, m_ui);
	m_infoIcon = new InfoIconWidget("", m_ui);

	if(isCompact) {
		m_ui->setLayout(new QHBoxLayout(m_ui));
		m_lineEdit->edit()->setAlignment(Qt::AlignRight);
	} else {
		Style::setStyle(m_title, style::properties::label::subtle);
		m_ui->setLayout(new QVBoxLayout(m_ui));
	}

	Style::setStyle(m_lineEdit->edit(), style::properties::iiowidgets::lineEdit, true, true);
	m_lineEdit->edit()->setCursorPosition(0);
	m_ui->layout()->setContentsMargins(0, 0, 0, 0);

	QHBoxLayout *titleLayout = new QHBoxLayout();
	titleLayout->setContentsMargins(0, 0, 0, 0);
	titleLayout->setMargin(0);
	titleLayout->setSpacing(5);

	titleLayout->addWidget(m_title);
	titleLayout->addWidget(m_infoIcon);
	titleLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Preferred));

	m_ui->layout()->addItem(titleLayout);

	m_ui->layout()->addWidget(m_lineEdit);
	m_ui->layout()->setSpacing(0);

	connect(m_lineEdit->edit(), &QLineEdit::editingFinished, this, [this]() {
		QString currentText = m_lineEdit->edit()->text();
		if(currentText != m_lastEmittedText) {
			m_lastEmittedText = currentText;
			Q_EMIT emitData(currentText);
		}
	});

	Q_EMIT requestData();
}

EditableGuiStrategy::~EditableGuiStrategy() {}

QWidget *EditableGuiStrategy::ui() { return m_ui; }

bool EditableGuiStrategy::isValid()
{
	if(m_recipe.data != "" && m_recipe.channel != nullptr) {
		return true;
	}
	return false;
}

void EditableGuiStrategy::setCustomTitle(QString title) { m_title->setText(title); }

void EditableGuiStrategy::receiveData(QString currentData, QString optionalData)
{
	QSignalBlocker blocker(m_lineEdit);
	m_lastEmittedText = currentData;
	m_lineEdit->edit()->setText(currentData);
	Q_EMIT displayedNewData(currentData, optionalData);
}

#include "moc_editableguistrategy.cpp"

void EditableGuiStrategy::setInfoMessage(QString infoMessage) { m_infoIcon->setInfoMessage(infoMessage); }
