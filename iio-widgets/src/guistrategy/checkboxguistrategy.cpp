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
 */

#include "guistrategy/checkboxguistrategy.h"
#include <style.h>
#include <QLoggingCategory>

using namespace scopy;

Q_LOGGING_CATEGORY(CAT_CHECKBOXGUISTRATEGY, "CheckBoxGuiStrategy")

CheckBoxAttrUi::CheckBoxAttrUi(IIOWidgetFactoryRecipe recipe, bool isCompact, QWidget *parent)
	: QObject(parent)
	, m_ui(new QWidget(parent))
	, m_isCompact(isCompact)
{
	m_recipe = recipe;
	m_ui->setLayout(new QHBoxLayout(m_ui));
	m_ui->layout()->setMargin(0);
	m_menuOnOffSwitch = new MenuOnOffSwitch("", m_ui);

	if(!m_isCompact) {
		m_toolTipTitle = new ToolTipTitle(recipe.data, "", m_ui);
		m_toolTipTitle->setFixedWidth(m_menuOnOffSwitch->width());
		m_ui->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		m_ui->layout()->addWidget(m_toolTipTitle);
	}
	m_ui->layout()->addWidget(m_menuOnOffSwitch);

	connect(m_menuOnOffSwitch->onOffswitch(), &QAbstractButton::clicked, this, [this]() {
		QString currentSelection = QString::number(m_menuOnOffSwitch->onOffswitch()->isChecked());
		Q_EMIT emitData(currentSelection);
	});
	Q_EMIT requestData();
}

CheckBoxAttrUi::~CheckBoxAttrUi() {}

QWidget *CheckBoxAttrUi::ui() { return m_ui; }

bool CheckBoxAttrUi::isValid()
{
	if(m_recipe.channel != nullptr && m_recipe.data != "" && m_recipe.iioDataOptions != "") {
		return true;
	}
	return false;
}

void CheckBoxAttrUi::setCustomTitle(QString title)
{
	if(!m_isCompact) {
		m_toolTipTitle->setTitle(title);
	}
}

void CheckBoxAttrUi::setToolTip(QString toolTip)
{
	if(!m_isCompact) {
		m_toolTipTitle->setToolTip(toolTip);
	}
}

void CheckBoxAttrUi::receiveData(QString currentData, QString optionalData)
{
	QSignalBlocker blocker(m_menuOnOffSwitch);
	bool ok;
	int val = currentData.toInt(&ok);
	if(ok) {
		m_menuOnOffSwitch->onOffswitch()->setChecked(val);
		Q_EMIT displayedNewData(currentData, optionalData);
	} else {
		qCritical(CAT_CHECKBOXGUISTRATEGY) << "Could not parse the value as a boolean.";
	}
}

#include "moc_checkboxguistrategy.cpp"
