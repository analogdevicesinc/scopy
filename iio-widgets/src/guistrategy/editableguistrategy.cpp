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
#include <component/attribute.h>

using namespace scopy;

// Slightly shorter than basicButton's unit_2, to sit flush with the line edit.
#define BUTTON_HEIGHT_QSS "min-height: 18px; max-height: 18px; padding: 0px 8px;"

EditableGuiStrategy::EditableGuiStrategy(IIOWidgetFactoryRecipe recipe, bool isCompact, QWidget *parent,
					 bool manualReadWrite)
	: QObject(parent)
	, m_ui(new QWidget(parent))
	, m_lineEdit(new MenuLineEdit(m_ui))
	, m_readBtn(nullptr)
	, m_writeBtn(nullptr)
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
	titleLayout->setContentsMargins(0, 0, 0, 0);
	titleLayout->setSpacing(5);

	titleLayout->addWidget(m_title);
	titleLayout->addWidget(m_infoIcon);
	titleLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Preferred));

	m_ui->layout()->addItem(titleLayout);

	m_ui->layout()->addWidget(m_lineEdit);
	m_ui->layout()->setSpacing(0);

	// A component::Attribute with no write capability is display-only.
	if(m_recipe.attribute && !m_recipe.attribute->isWritable()) {
		m_lineEdit->edit()->setReadOnly(true);
	}

	if(manualReadWrite) {
		setupManualReadWrite();
	} else {
		connect(m_lineEdit->edit(), &QLineEdit::editingFinished, this, [this]() {
			QString currentText = m_lineEdit->edit()->text();
			if(currentText != m_lastEmittedText) {
				m_lastEmittedText = currentText;
				Q_EMIT emitData(currentText);
			}
		});
	}

	Q_EMIT requestData();
}

EditableGuiStrategy::~EditableGuiStrategy() {}

void EditableGuiStrategy::setupManualReadWrite()
{
	// Context attributes are read-only; component::Attributes expose their capabilities.
	bool readable = !m_recipe.attribute || m_recipe.attribute->readCapability();
	bool writable = m_recipe.attribute ? m_recipe.attribute->isWritable() : (m_recipe.channel || m_recipe.device);

	// Swap the bare line edit for a row holding the line edit and the buttons.
	m_ui->layout()->removeWidget(m_lineEdit);
	QWidget *row = new QWidget(m_ui);
	QHBoxLayout *rowLayout = new QHBoxLayout(row);
	rowLayout->setContentsMargins(0, 0, 0, 0);
	rowLayout->setSpacing(5);
	rowLayout->addWidget(m_lineEdit);
	m_ui->layout()->addWidget(row);

	if(readable) {
		m_readBtn = new QPushButton("Read", row);
		Style::setStyle(m_readBtn, style::properties::button::basicButton);
		m_readBtn->setStyleSheet(BUTTON_HEIGHT_QSS);
		rowLayout->addWidget(m_readBtn);
		connect(m_readBtn, &QPushButton::clicked, this, &EditableGuiStrategy::requestData);
	}

	if(writable) {
		m_writeBtn = new QPushButton("Write", row);
		Style::setStyle(m_writeBtn, style::properties::button::basicButton);
		m_writeBtn->setStyleSheet(BUTTON_HEIGHT_QSS);
		rowLayout->addWidget(m_writeBtn);
		connect(m_writeBtn, &QPushButton::clicked, this, [this]() {
			m_lastEmittedText = m_lineEdit->edit()->text();
			Q_EMIT emitData(m_lastEmittedText);
		});
		connect(m_lineEdit->edit(), &QLineEdit::returnPressed, m_writeBtn, &QPushButton::click);
	} else {
		m_lineEdit->edit()->setReadOnly(true);
	}
}

QWidget *EditableGuiStrategy::ui() { return m_ui; }

bool EditableGuiStrategy::isValid()
{
	if(m_recipe.data != "" && (m_recipe.channel != nullptr || m_recipe.attribute != nullptr)) {
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
