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

#include "guistrategy/switchguistrategy.h"
#include <QLoggingCategory>

using namespace scopy;

Q_LOGGING_CATEGORY(CAT_SWITCHGUISTRATEGY, "SwitchGuiStrategy")

SwitchAttrUi::SwitchAttrUi(IIOWidgetFactoryRecipe recipe, QWidget *parent)
	: QWidget(parent)
	, m_ui(new QWidget(nullptr))
	, m_optionsList(new QStringList)
{
	m_recipe = recipe;
	m_ui->setLayout(new QVBoxLayout(m_ui));
	m_ui->layout()->setMargin(0);
	m_menuBigSwitch = new CustomSwitch(m_ui);
	m_ui->layout()->addWidget(m_menuBigSwitch);
	Q_EMIT requestData();

	connect(m_menuBigSwitch, &QPushButton::clicked, this, [this]() {
		QString currentSelection = (m_menuBigSwitch->isChecked()) ? m_optionsList->at(0) : m_optionsList->at(1);
		Q_EMIT emitData(currentSelection);
	});
}

SwitchAttrUi::~SwitchAttrUi()
{
	delete m_optionsList;
	m_ui->deleteLater();
}

QWidget *SwitchAttrUi::ui() { return m_ui; }

bool SwitchAttrUi::isValid()
{
	if(m_recipe.channel != nullptr && m_recipe.data != "" && m_recipe.iioDataOptions != "") {
		return true;
	}
	return false;
}

void SwitchAttrUi::receiveData(QString currentData, QString optionalData)
{
	QSignalBlocker blocker(m_menuBigSwitch);
	QStringList optionsList = QString(optionalData).split(" ", Qt::SkipEmptyParts);
	if(optionsList.size() < 2) {
		qWarning(CAT_SWITCHGUISTRATEGY) << "Received less than 2 options, should this be a switch?";
		return;
	}
	if(optionsList.size() > 2) {
		qDebug(CAT_SWITCHGUISTRATEGY) << "Received more than 2 options, only the first 2 will be used.";
	}
	*m_optionsList = optionsList;
	m_menuBigSwitch->setOnText(optionsList[0]);
	m_menuBigSwitch->setOffText(optionsList[1]);
	if(optionsList[0] == QString(currentData)) {
		m_menuBigSwitch->setChecked(true);
	} else {
		m_menuBigSwitch->setChecked(false);
	}

	Q_EMIT displayedNewData(currentData, optionalData);
}

#include "moc_switchguistrategy.cpp"
