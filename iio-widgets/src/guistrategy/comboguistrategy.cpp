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
#include "style_properties.h"
#include <gui/style.h>
#include <QLoggingCategory>

using namespace scopy;

ComboAttrUi::ComboAttrUi(IIOWidgetFactoryRecipe recipe, bool isCompact, QWidget *parent)
	: QObject(parent)
	, m_ui(new QWidget(parent))
	, m_isCompact(isCompact)
{
	m_recipe = recipe;

	if(m_isCompact) {
		m_ui->setLayout(new QHBoxLayout(m_ui));
		m_ui->layout()->setContentsMargins(0, 0, 0, 0);

		m_title = new QLabel(recipe.data, m_ui);
		Style::setStyle(m_title, style::properties::label::subtle);
		m_infoIcon = new InfoIconWidget("", m_ui);
		m_comboWidget = new QComboBox(m_ui);
		m_comboWidget->setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy::AdjustToContents);

		m_comboWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

		m_ui->layout()->addWidget(m_title);
		m_ui->layout()->addWidget(m_infoIcon);
		m_ui->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Preferred));
		m_ui->layout()->addWidget(m_comboWidget);
	} else {
		m_ui->setLayout(new QVBoxLayout(m_ui));
		m_ui->layout()->setContentsMargins(0, 0, 0, 0);

		m_comboMenuWidget = new MenuCombo(recipe.data, m_ui);
		m_comboWidget = m_comboMenuWidget->combo();

		m_ui->layout()->addWidget(m_comboMenuWidget);
	}

	Style::setStyle(m_comboWidget, style::properties::iiowidgets::comboBox, true, true);

	connect(m_comboWidget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
		QString currentData = m_comboWidget->currentText();
		Q_EMIT emitData(currentData);
	});

	Q_EMIT requestData();
}

ComboAttrUi::~ComboAttrUi() {}

QWidget *ComboAttrUi::ui() { return m_ui; }

bool ComboAttrUi::isValid()
{
	if(m_recipe.channel != nullptr && m_recipe.data != "" && m_recipe.iioDataOptions != "") {
		return true;
	}
	return false;
}

void ComboAttrUi::setCustomTitle(QString title)
{
	if(m_isCompact) {
		m_title->setText(title);
	} else {
		m_comboMenuWidget->setTitle(title);
	}
}

void ComboAttrUi::setInfoMessage(QString infoMessage)
{
	if(m_isCompact) {
		m_infoIcon->setInfoMessage(infoMessage);
	} else {
		m_comboMenuWidget->setInfoMessage(infoMessage);
	}
}

void ComboAttrUi::receiveData(QString currentData, QString optionalData)
{
	QSignalBlocker blocker(m_comboWidget);
	m_comboWidget->clear();
	QStringList optionsList = QString(optionalData).split(" ", Qt::SkipEmptyParts);
	for(const QString &item : optionsList) {
		m_comboWidget->addItem(item);
	}

	if(m_isCompact) {
		// hackish: in compact mode, the value is pushed to the right and the width is
		// at the minimum. Because of this, the viewport is smaller than the size and some
		// values might not be seen. The temporary solution is to add this line (until the
		// new gui system and the new designs).
		m_comboWidget->view()->setMinimumWidth(m_comboWidget->view()->sizeHintForColumn(0));
	}

	m_comboWidget->setCurrentText(currentData);
	Q_EMIT displayedNewData(currentData, optionalData);
}

#include "moc_comboguistrategy.cpp"
