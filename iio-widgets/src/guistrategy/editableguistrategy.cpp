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
	QLabel *label = new QLabel(recipe.data, m_ui);

	if(isCompact) {
		m_ui->setLayout(new QHBoxLayout(m_ui));
		StyleHelper::IIOCompactLabel(label, "TitleLabel");
		m_lineEdit->edit()->setAlignment(Qt::AlignRight);
	} else {
		m_ui->setLayout(new QVBoxLayout(m_ui));
		StyleHelper::MenuSmallLabel(label, "MenuSmallLabel");
	}

	Style::setStyle(m_lineEdit->edit(), style::properties::iiowidgets::lineEdit, true, true);
	m_lineEdit->edit()->setCursorPosition(0);
	m_ui->layout()->setContentsMargins(0, 0, 0, 0);
	m_ui->layout()->addWidget(label);
	m_ui->layout()->addWidget(m_lineEdit);

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

void EditableGuiStrategy::receiveData(QString currentData, QString optionalData)
{
	QSignalBlocker blocker(m_lineEdit);
	m_lastEmittedText = currentData;
	m_lineEdit->edit()->setText(currentData);
	Q_EMIT displayedNewData(currentData, optionalData);
}

#include "moc_editableguistrategy.cpp"
