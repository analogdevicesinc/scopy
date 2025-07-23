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

#include "menuitemfactory.h"
#include <QComboBox>
#include <QCheckBox>
#include <QVariant>
#include <menucombo.h>
#include <menuonoffswitch.h>
#include "gui/widgets/menuspinbox.h"
#include "gui/style.h"

using namespace scopy::extprocplugin;
using namespace scopy::gui;

MenuItemFactory::MenuItemFactory(QObject *parent)
	: QObject(parent)
{
	connect(this, &MenuItemFactory::itemParamChanged, this, &MenuItemFactory::onItemParamChanged);
}

MenuItemFactory::~MenuItemFactory() {}

QWidget *MenuItemFactory::createWidget(const QString &field, const QVariantMap &params)
{
	if(params.contains("min") && params.contains("max") && params.contains("step")) {
		return createSpinBox(field, params);
	} else if(params.contains("available_values")) {
		return createCombo(field, params);
	} else if(params["type"].toString() == "bool") {
		return createCheckBox(field, params);
	} else {
		return createLineEdit(field, params);
	}

	return nullptr;
}

QWidget *MenuItemFactory::createSpinBox(const QString &field, const QVariantMap &params)
{
	double defaultVal = params.value("default", 0).toDouble();
	MenuSpinbox *spinBox = new MenuSpinbox(field, defaultVal, "", params.value("min", 0).toInt(),
					       params.value("max", 100).toInt());
	spinBox->setIncrementMode(gui::MenuSpinbox::IS_FIXED); //?
	connect(spinBox, &MenuSpinbox::valueChanged, this,
		[this, field](double value) { Q_EMIT itemParamChanged(field, value); });

	Q_EMIT itemParamChanged(field, spinBox->value());
	return spinBox;
}

QWidget *MenuItemFactory::createLineEdit(const QString &field, const QVariantMap &params)
{
	QWidget *w = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(w);
	lay->setMargin(0);

	QLabel *label = new QLabel(field, w);
	Style::setStyle(label, style::properties::label::menuSmall);

	QLineEdit *lineEdit = new QLineEdit(w);
	lineEdit->setText(params.value("default", "0").toString());

	lay->addWidget(label);
	lay->addWidget(lineEdit);

	connect(lineEdit, &QLineEdit::textChanged, this,
		[this, field](const QString &text) { Q_EMIT itemParamChanged(field, text); });

	Q_EMIT itemParamChanged(field, lineEdit->text());
	return w;
}

QWidget *MenuItemFactory::createCombo(const QString &field, const QVariantMap &params)
{
	MenuCombo *combo = new MenuCombo(field);
	QStringList items = params.value("available_values").toStringList();
	combo->combo()->addItems(items);
	if(params.contains("default")) {
		combo->combo()->setCurrentText(params.value("default").toString());
	}
	connect(combo->combo(), &QComboBox::currentTextChanged, this,
		[this, field](const QString &text) { Q_EMIT itemParamChanged(field, text); });

	Q_EMIT itemParamChanged(field, combo->combo()->currentText());
	return combo;
}

QWidget *MenuItemFactory::createCheckBox(const QString &field, const QVariantMap &params)
{
	MenuOnOffSwitch *checkBox = new MenuOnOffSwitch(field);
	checkBox->onOffswitch()->setChecked(params.value("value", false).toBool());
	connect(checkBox->onOffswitch(), &QCheckBox::toggled, this,
		[this, field](bool checked) { Q_EMIT itemParamChanged(field, checked); });

	Q_EMIT itemParamChanged(field, checkBox->onOffswitch()->isChecked());
	return checkBox;
}

QVariantMap MenuItemFactory::toVariantMap() { return m_currentParams; }

void MenuItemFactory::onItemParamChanged(const QString &field, const QVariant &value)
{
	m_currentParams[field] = value;
}
