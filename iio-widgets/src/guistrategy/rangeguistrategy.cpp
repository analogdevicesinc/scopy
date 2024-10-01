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

#include "guistrategy/rangeguistrategy.h"
#include <QSpinBox>

using namespace scopy;

Q_LOGGING_CATEGORY(CAT_ATTR_GUI_STRATEGY, "AttrGuiStrategy")

RangeAttrUi::RangeAttrUi(IIOWidgetFactoryRecipe recipe, bool isCompact, QWidget *parent)
	: QObject(parent)
	, m_ui(new QWidget(parent))
{
	m_recipe = recipe;
	if(!isValid()) {
		qCritical(CAT_ATTR_GUI_STRATEGY)
			<< "The data you sent to this range gui strategy is not complete. Cannot create object";
	}

	m_ui->setLayout(new QVBoxLayout(m_ui));
	m_ui->layout()->setContentsMargins(0, 0, 0, 0);

	m_spinBox = new gui::MenuSpinbox(m_recipe.data.toUpper(), 0, "", 0, 1, true, false, m_ui);
	m_spinBox->setIncrementMode(gui::MenuSpinbox::IS_FIXED);
	m_spinBox->setScaleRange(1, 1);
	m_spinBox->setScalingEnabled(false);
	m_spinBox->setLineVisible(false);
	m_ui->layout()->addWidget(m_spinBox);

	connect(m_spinBox, &gui::MenuSpinbox::valueChanged, this,
		[&](double value) { Q_EMIT emitData(Util::doubleToQString(value)); });
	Q_EMIT requestData();
}

RangeAttrUi::~RangeAttrUi() {}

QWidget *RangeAttrUi::ui() { return m_ui; }

bool RangeAttrUi::isValid()
{
	if((m_recipe.channel || m_recipe.device) && m_recipe.data != "" &&
	   (m_recipe.iioDataOptions != "" || m_recipe.constDataOptions != "")) {
		return true;
	}
	return false;
}

void RangeAttrUi::receiveData(QString currentData, QString optionalData)
{
	QSignalBlocker blocker(m_spinBox);
	QString availableAttributeValue = QString(optionalData).mid(1, QString(optionalData).size() - 2);
	QStringList optionsList = availableAttributeValue.split(" ", Qt::SkipEmptyParts);
	int optionListSize = optionsList.size();

	if(optionListSize != 3) {
		// Ideally the build factory system will catch this and init this as a lineedit, but we will
		// treatthis just in case. The following rules will apply:
		// 	- The optionalData will be ignored
		// 	- The step will be set to 1
		// 	- The min and the max will not be set
		qCritical(CAT_ATTR_GUI_STRATEGY)
			<< "The Range ui strategy cannot be initialized with the optional data ("
			<< availableAttributeValue << ") and will try to partially initialize.";

		bool ok;
		double value = currentData.toDouble(&ok);
		if(!ok) {
			qCritical(CAT_ATTR_GUI_STRATEGY)
				<< "Cannot partially initialize, something is very wrong here. " << currentData
				<< " is not a double.";
			m_spinBox->setValueString(currentData);
			return;
		}

		m_spinBox->setValue(value);
		return;
	}

	bool ok = true, finalOk = true;

	double min = tryParse(optionsList[0], &ok);
	finalOk &= ok;

	double step = tryParse(optionsList[1], &ok);
	finalOk &= ok;

	double max = tryParse(optionsList[2], &ok);
	finalOk &= ok;

	double currentNum = QString(currentData).toDouble(&ok);
	finalOk &= ok;

	if(!finalOk) {
		qCritical(CAT_ATTR_GUI_STRATEGY)
			<< "Could not parse the values from" << availableAttributeValue << "as double or int.";
		m_spinBox->setDisabled(true);
		m_spinBox->setValueString(currentData);
	} else {
		m_spinBox->setDisabled(false);
		m_spinBox->setMinValue(min);
		m_spinBox->setMaxValue(max);
		m_spinBox->incrementStrategy()->setScale(step);
		m_spinBox->setValue(currentNum);
	}

	Q_EMIT displayedNewData(currentData, optionalData);
}

double RangeAttrUi::tryParse(QString number, bool *success)
{
	// Try to parse as double first
	bool ok = true;
	double result = number.toDouble(&ok);
	if(ok) {
		*success = true;
		return result;
	}

	// Try to parse as int and cast to double
	int result_int = number.toInt(&ok);
	if(ok) {
		*success = true;
		result = static_cast<double>(result_int);
		return result;
	}

	*success = false;
	return -1;
}

#include "moc_rangeguistrategy.cpp"
