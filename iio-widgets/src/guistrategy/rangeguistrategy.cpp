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
#include <QTimer>

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

	m_spinBox = new gui::MenuSpinbox(m_recipe.data, 0, "", 0, 1, true, false, false, m_ui);
	m_spinBox->setIncrementMode(gui::MenuSpinbox::IS_FIXED);
	m_spinBox->setScaleRange(1, 1);
	m_spinBox->setScalingEnabled(false);
	m_spinBox->enableRangeLimits(false);
	m_ui->layout()->addWidget(m_spinBox);

	connect(m_spinBox, &gui::MenuSpinbox::valueChanged, this, &RangeAttrUi::onValueChanged);
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

void RangeAttrUi::setCustomTitle(QString title) { m_spinBox->setName(title); }

void RangeAttrUi::setInfoMessage(QString infoMessage) { m_spinBox->setInfoMessage(infoMessage); }

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
		QString unit;
		double value = tryParse(currentData, &ok, &unit);
		if(!ok) {
			qCritical(CAT_ATTR_GUI_STRATEGY)
				<< "Cannot partially initialize, something is very wrong here. " << currentData
				<< " is not a double.";
			m_spinBox->setValueString(currentData);
			return;
		}

		if(!unit.isEmpty()) {
			QString currentTitle = m_spinBox->name();
			if(!currentTitle.contains("(")) {
				m_spinBox->setName(currentTitle + " (" + unit + ")");
			}
		}

		m_spinBox->setValue(value);
		return;
	}

	bool ok = true, finalOk = true;
	QString unit;

	double min = tryParse(optionsList[0], &ok);
	finalOk &= ok;

	double step = tryParse(optionsList[1], &ok);
	finalOk &= ok;

	double max = tryParse(optionsList[2], &ok);
	finalOk &= ok;

	double currentNum = tryParse(currentData, &ok, &unit);
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

		if(!unit.isEmpty()) {
			QString currentTitle = m_spinBox->name();
			if(!currentTitle.contains("(")) {
				m_spinBox->setName(currentTitle + " (" + unit + ")");
			}
		}

		m_spinBox->setValue(currentNum);
	}

	Q_EMIT displayedNewData(currentData, optionalData);
}

double RangeAttrUi::tryParse(QString number, bool *success, QString *unit)
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

	if(unit) {
		// Try to parse value with unit suffix (e.g., "100 dB", "50 Hz")
		QString trimmed = number.trimmed();
		int unitStartIndex = -1;
		for(int i = 0; i < trimmed.length(); ++i) {
			QChar ch = trimmed.at(i);
			if(ch.isSpace()) {
				unitStartIndex = i;
				break;
			}
			if(ch.isLetter() && i > 0) {
				unitStartIndex = i;
				break;
			}
		}

		if(unitStartIndex > 0) {
			QString numberPart = trimmed.left(unitStartIndex).trimmed();
			QString unitPart = trimmed.mid(unitStartIndex).trimmed();
			*unit = unitPart;

			// Try to parse the number part
			double unitResult = numberPart.toDouble(&ok);
			if(ok) {
				*success = true;
				return unitResult;
			}
			// Try as int
			int intVal = numberPart.toInt(&ok);
			if(ok) {
				*success = true;
				return static_cast<double>(intVal);
			}
		}
	}

	*success = false;
	return -1;
}

void RangeAttrUi::onValueChanged(double value)
{
	m_currentValueToProcess = value;
	Q_EMIT requestData();

	// we use QTimer::singleShot to execute processValueChange() after the event loop finished,
	// to make sure the data was updated
	QTimer::singleShot(0, this, SLOT(processValueChange()));
}

void RangeAttrUi::processValueChange()
{
	// manually clamp value since clamping within meniSpinBox is disabled
	double clampedValue = std::min(std::max(m_spinBox->min(), m_currentValueToProcess), m_spinBox->max());

	// set the value in UI again after it was updated from requestData()
	m_spinBox->setValueSilent(clampedValue);
	Q_EMIT emitData(Util::doubleToQString(clampedValue));
}

#include "moc_rangeguistrategy.cpp"
