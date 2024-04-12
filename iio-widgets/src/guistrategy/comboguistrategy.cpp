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

#include "guistrategy/comboguistrategy.h"
#include <QLoggingCategory>

using namespace scopy;

ComboAttrUi::ComboAttrUi(IIOWidgetFactoryRecipe recipe, QWidget *parent)
	: m_ui(new QWidget(nullptr))
{
	m_recipe = recipe;
	m_ui->setLayout(new QVBoxLayout(m_ui));
	m_ui->layout()->setContentsMargins(0, 0, 0, 0);

	m_comboWidget = new MenuCombo(recipe.data, m_ui);
	StyleHelper::IIOComboBox(m_comboWidget->combo(), "IIOComboBox");

	m_ui->layout()->addWidget(m_comboWidget);
	Q_EMIT requestData();

	connect(m_comboWidget->combo(), QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
		QString currentData = m_comboWidget->combo()->currentText();
		Q_EMIT emitData(currentData);
	});
}

ComboAttrUi::~ComboAttrUi() { m_ui->deleteLater(); }

QWidget *ComboAttrUi::ui() { return m_ui; }

bool ComboAttrUi::isValid()
{
	if(m_recipe.channel != nullptr && m_recipe.data != "" && m_recipe.iioDataOptions != "") {
		return true;
	}
	return false;
}

void ComboAttrUi::receiveData(QString currentData, QString optionalData)
{
	QSignalBlocker blocker(m_comboWidget->combo());
	m_comboWidget->combo()->clear();
	QStringList optionsList = QString(optionalData).split(" ", Qt::SkipEmptyParts);
	for(const QString &item : optionsList) {
		m_comboWidget->combo()->addItem(item);
	}

	m_comboWidget->combo()->setCurrentText(currentData);
	Q_EMIT displayedNewData(currentData, optionalData);
}

#include "moc_comboguistrategy.cpp"
