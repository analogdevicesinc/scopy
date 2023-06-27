/*
 * Copyright (c) 2023 Analog Devices Inc.
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


#include "buffermenu.h"
#include "qdebug.h"

using namespace scopy::swiot;
BufferMenu::BufferMenu(QWidget *parent, QString chnlFunction)
	: QWidget(parent),
	  m_widget(parent),
	  m_chnlFunction(chnlFunction)
{}

BufferMenu::~BufferMenu()
{}

QString BufferMenu::getInfoMessage()
{
	QString defaultMessage = "\"" + m_chnlFunction + "\" configuration generates\n" +
			"a buffer capable channel which is directly\n" +
			"related to the plot. The channel attributes\n" +
			"can be changed through this menu.";
	return defaultMessage;
}

void BufferMenu::addMenuLayout(QHBoxLayout *layout)
{
	m_menuLayers.push_back(layout);
}

void BufferMenu::setAttrValues(QMap<QString, QMap<QString, QStringList>> values)
{
	m_attrValues=values;
}

QMap<QString, QMap<QString, QStringList>> BufferMenu::getAttrValues()
{
	return m_attrValues;
}

QVector<QHBoxLayout *> BufferMenu::getMenuLayers()
{
	return m_menuLayers;
}

double BufferMenu::convertFromRaw(int rawValue, QString chnlType)
{
	double value=0.0;
	if (m_attrValues.contains(chnlType)
			&& m_attrValues[chnlType].contains("offset")
			&& m_attrValues[chnlType].contains("scale")) {
		double offset = m_attrValues[chnlType]["offset"][0].toDouble();
		double scale = m_attrValues[chnlType]["scale"][0].toDouble();
		value = (rawValue + offset) * scale;
	}
	return value;
}

CurrentInLoopMenu::CurrentInLoopMenu(QWidget* parent, QString chnlFunction)
	: BufferMenu(parent, chnlFunction)
{}

CurrentInLoopMenu::~CurrentInLoopMenu()
{}

void CurrentInLoopMenu::init()
{
	//dac code
	QHBoxLayout *dacCodeLayout = new QHBoxLayout();
	m_dacCodeSpinButton = new PositionSpinButton({
							{"value", 1E0}
						     },
						     "DAC Code", 0,
						     8191,
						     true, false, m_widget);
	dacCodeLayout->addWidget(m_dacCodeSpinButton);
	addMenuLayout(dacCodeLayout);
	//dac label
	QHBoxLayout *m_dacLabelLayout = new QHBoxLayout();
	m_dacLabel = new QLabel("mA", m_widget);
	m_dacLabelLayout->addWidget(m_dacLabel);
	m_dacLabelLayout->setAlignment(Qt::AlignRight);
	addMenuLayout(m_dacLabelLayout);

	connectSignalsToSlots();
}

void CurrentInLoopMenu::connectSignalsToSlots()
{
	connect(m_dacCodeSpinButton, &PositionSpinButton::valueChanged, this, &CurrentInLoopMenu::dacCodeChanged);
}

QString CurrentInLoopMenu::getInfoMessage()
{
	QString infoMessage = "\"" + m_chnlFunction + "\" configuration generates 2 context channels.\n" +
			"One of them is an input buffer capable channel (current_in)\n" +
			"which is related to the plot and the other one is an output\n" +
			"channel (current_out) whose attributes can be changed from\n" +
			"this menu.";
	return infoMessage;
}

void CurrentInLoopMenu::dacCodeChanged(double value)
{
	QString attrName("raw");
	if (!m_attrValues.contains(OUTPUT_CHNL)) {
		qWarning() << "There is no output channel available for this function!";
		return;
	}
	m_attrValues[OUTPUT_CHNL][attrName].clear();
	m_attrValues[OUTPUT_CHNL][attrName].push_back(QString::number(value));
	double val = convertFromRaw(value) ;
	m_dacLabel->clear();
	m_dacLabel->setText(QString::number(val)+" mA");

	Q_EMIT attrValuesChanged(attrName, OUTPUT_CHNL);

}

DigitalInLoopMenu::DigitalInLoopMenu(QWidget* parent, QString chnlFunction)
	: BufferMenu(parent, chnlFunction)
{}

DigitalInLoopMenu::~DigitalInLoopMenu()
{}

void DigitalInLoopMenu::init()
{
	//threshold
	QHBoxLayout *thresholdLayout = new QHBoxLayout();
	m_thresholdSpinButton = new PositionSpinButton({
							{"mV", 1E0}
						     },
						     "Threshold", 0,
						     16000,
						     true, false, m_widget);
	thresholdLayout->addWidget(m_thresholdSpinButton);
	addMenuLayout(thresholdLayout);
	//dac code
	QHBoxLayout *dacCodeLayout = new QHBoxLayout();
	m_dacCodeSpinButton = new PositionSpinButton({
							{"value", 1E0}
						     },
						     "DAC Code", 0,
						     8191,
						     true, false, m_widget);
	dacCodeLayout->addWidget(m_dacCodeSpinButton);
	addMenuLayout(dacCodeLayout);
	//dac label
	QHBoxLayout *m_dacLabelLayout = new QHBoxLayout();
	m_dacLabel = new QLabel("mA", m_widget);
	m_dacLabelLayout->addWidget(m_dacLabel);
	m_dacLabelLayout->setAlignment(Qt::AlignRight);
	addMenuLayout(m_dacLabelLayout);

	connectSignalsToSlots();
}

void DigitalInLoopMenu::connectSignalsToSlots()
{
	connect(m_dacCodeSpinButton, &PositionSpinButton::valueChanged, this, &DigitalInLoopMenu::dacCodeChanged);
	connect(m_thresholdSpinButton, &PositionSpinButton::valueChanged, this, &DigitalInLoopMenu::thresholdChanged);
}

QString DigitalInLoopMenu::getInfoMessage()
{
	QString infoMessage = "\"" + m_chnlFunction + "\" configuration generates 2 context channels.\n" +
			"One of them is an input buffer capable channel (voltage_in)\n" +
			"which is related to the plot and the other one is an output\n" +
			"channel (current_out). The threshold is set for the\n" +
			"input channel and the DAC Code for the output channel.";
	return infoMessage;
}

void DigitalInLoopMenu::dacCodeChanged(double value)
{
	QString attrName("raw");
	if (!m_attrValues.contains(OUTPUT_CHNL)) {
		qWarning() << "There is no output channel available for this function!";
		return;
	}
	m_attrValues[OUTPUT_CHNL][attrName].clear();
	m_attrValues[OUTPUT_CHNL][attrName].push_back(QString::number(value));
	double val = convertFromRaw(value);
	m_dacLabel->clear();
	m_dacLabel->setText(QString::number(val) + " mA");

	Q_EMIT attrValuesChanged(attrName, OUTPUT_CHNL);

}

void DigitalInLoopMenu::thresholdChanged(double value)
{
	QString attrName("threshold");
	if (!m_attrValues.contains(INPUT_CHNL)) {
		qWarning() << "There is no input channel available for this function!";
		return;
	}
	if (m_attrValues[INPUT_CHNL].contains(attrName)) {
		m_attrValues[INPUT_CHNL][attrName].clear();
		m_attrValues[INPUT_CHNL][attrName].push_back(QString::number((int)value));
		Q_EMIT attrValuesChanged(attrName, INPUT_CHNL);
	} else {
		qWarning() << attrName.toUpper() + " attribute does not exist!";
	}
}


VoltageOutMenu::VoltageOutMenu(QWidget* parent, QString chnlFunction)
	: BufferMenu(parent, chnlFunction)
{}

VoltageOutMenu::~VoltageOutMenu()
{}

void VoltageOutMenu::init()
{
	//dac code
	QHBoxLayout *dacCodeLayout = new QHBoxLayout();
	m_dacCodeSpinButton = new PositionSpinButton({
							{"value", 1E0}
						     },
						     "DAC Code", 0,
						     8191,
						     true, false, m_widget);
	dacCodeLayout->addWidget(m_dacCodeSpinButton);
	addMenuLayout(dacCodeLayout);
	//dac label
	QHBoxLayout *m_dacLabelLayout = new QHBoxLayout();
	m_dacLabel = new QLabel("V", m_widget);
	m_dacLabelLayout->addWidget(m_dacLabel);
	m_dacLabelLayout->setAlignment(Qt::AlignRight);
	addMenuLayout(m_dacLabelLayout);
	//slew
	QHBoxLayout *slewLayout = new QHBoxLayout();
	m_slewOptions = new QComboBox(m_widget);
	m_slewOptions->addItem(QString("Disable"));
	m_slewOptions->addItem(QString("Enable"));
	(m_attrValues[OUTPUT_CHNL]["slew_en"].first().compare("0") == 0)
			? m_slewOptions->setCurrentText("Disable")
			: m_slewOptions->setCurrentText("Enable");
	slewLayout->addWidget(new QLabel("Slew", m_widget), 1);
	slewLayout->addWidget(m_slewOptions, 1);
	addMenuLayout(slewLayout);
	//slew step
	QHBoxLayout *slewStepLayout = new QHBoxLayout();
	m_slewStepOptions = new QComboBox(m_widget);
	setAvailableOptions(m_slewStepOptions, "slew_step_available");
	(m_attrValues[OUTPUT_CHNL].contains("slew_step") && m_attrValues[OUTPUT_CHNL]["slew_step"].size() > 0)
			? m_slewStepOptions->setCurrentText(m_attrValues[OUTPUT_CHNL]["slew_step"].first())
			: m_slewStepOptions->setCurrentText("ERROR");
	slewStepLayout->addWidget(new QLabel("Slew Step Size", m_widget), 1);
	slewStepLayout->addWidget(m_slewStepOptions, 1);
	addMenuLayout(slewStepLayout);
	//slew rate
	QHBoxLayout *slewRateLayout = new QHBoxLayout();
	m_slewRateOptions = new QComboBox(m_widget);
	setAvailableOptions(m_slewRateOptions, "slew_rate_available");
	(m_attrValues[OUTPUT_CHNL].contains("slew_rate") && m_attrValues[OUTPUT_CHNL]["slew_rate"].size() > 0)
			? m_slewRateOptions->setCurrentText(m_attrValues[OUTPUT_CHNL]["slew_rate"].first())
			: m_slewRateOptions->setCurrentText("ERROR");
	slewRateLayout->addWidget(new QLabel("Slew Rate (kHz)", m_widget), 1);
	slewRateLayout->addWidget(m_slewRateOptions, 1);
	addMenuLayout(slewRateLayout);

	connectSignalsToSlots();
}

void VoltageOutMenu::connectSignalsToSlots()
{
	connect(m_dacCodeSpinButton, &PositionSpinButton::valueChanged, this, &VoltageOutMenu::dacCodeChanged);
	connect(m_slewStepOptions, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &VoltageOutMenu::slewStepIndexChanged);
	connect(m_slewRateOptions, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &VoltageOutMenu::slewRateIndexChanged);
	connect(m_slewOptions, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &VoltageOutMenu::slewIndexChanged);
}

QString VoltageOutMenu::getInfoMessage()
{
	QString infoMessage = "\"" + m_chnlFunction + "\" configuration generates 2 context channels.\n" +
			"One of them is an input buffer capable channel (current_in)\n" +
			"which is related to the plot and the other one is an\n" +
			"output channel whose attributes can be changed from\n" +
			"this menu.";
	return infoMessage;
}

void VoltageOutMenu::setAvailableOptions(QComboBox *list, QString attrName)
{
	if (!m_attrValues.contains(OUTPUT_CHNL)) {
		qWarning() << "There is no output channel available for this function!";
		return;
	}
	QStringList availableValues = m_attrValues[OUTPUT_CHNL][attrName];
	for (const auto& slewValue : availableValues) {
		list->addItem(slewValue);
	}
}

void VoltageOutMenu::dacCodeChanged(double value)
{
	QString attrName("raw");
	if (!m_attrValues.contains(OUTPUT_CHNL)) {
		qWarning() << "There is no output channel available for this function!";
		return;
	}
	m_attrValues[OUTPUT_CHNL][attrName].clear();
	m_attrValues[OUTPUT_CHNL][attrName].push_back(QString::number((int)value));
	//the convertFromRaw return value is in mV that's why we multiply by 10^(-3)
	double val = convertFromRaw(value) * 0.001;
	m_dacLabel->clear();
	m_dacLabel->setText(QString::number(val) + " V");

	Q_EMIT attrValuesChanged(attrName, OUTPUT_CHNL);

}

void VoltageOutMenu::slewStepIndexChanged(int idx)
{
	QString attrName = "slew_step";
	if (!m_attrValues.contains(OUTPUT_CHNL)) {
		qWarning() << "There is no output channel available for this function!";
		return;
	}
	const auto& slewStep = m_attrValues[OUTPUT_CHNL]["slew_step_available"][idx];
	m_attrValues[OUTPUT_CHNL][attrName].clear();
	m_attrValues[OUTPUT_CHNL][attrName].push_back(slewStep);

	Q_EMIT attrValuesChanged(attrName, OUTPUT_CHNL);

}

void VoltageOutMenu::slewRateIndexChanged(int idx)
{
	QString attrName = "slew_rate";
	if (!m_attrValues.contains(OUTPUT_CHNL)) {
		qWarning() << "There is no output channel available for this function!";
		return;
	}
	const auto& slewStep = m_attrValues[OUTPUT_CHNL]["slew_rate_available"][idx];
	m_attrValues[OUTPUT_CHNL][attrName].clear();
	m_attrValues[OUTPUT_CHNL][attrName].push_back(slewStep);

	Q_EMIT attrValuesChanged(attrName, OUTPUT_CHNL);

}

void VoltageOutMenu::slewIndexChanged(int idx)
{
	QString attrName = "slew_en";
	if (!m_attrValues.contains(OUTPUT_CHNL) ) {
		qWarning() << "There is no output channel available for this function!";
		return;
	}
	m_attrValues[OUTPUT_CHNL][attrName].clear();
	if (idx == SLEW_DISABLE_IDX) {
		m_attrValues[OUTPUT_CHNL][attrName].push_back(QString("0"));
	}
	else {
		m_attrValues[OUTPUT_CHNL][attrName].push_back(QString("1"));
	}

	Q_EMIT attrValuesChanged(attrName, OUTPUT_CHNL);

}

CurrentOutMenu::CurrentOutMenu(QWidget* parent, QString chnlFunction)
	: BufferMenu(parent, chnlFunction)
{}

CurrentOutMenu::~CurrentOutMenu()
{}

void CurrentOutMenu::init()
{
	//dac code
	QHBoxLayout *dacCodeLayout = new QHBoxLayout();
	m_dacCodeSpinButton = new PositionSpinButton({
							{"value", 1E0}
						     },
						     "DAC Code", 0,
						     8196,
						     true, false, m_widget);
	dacCodeLayout->addWidget(m_dacCodeSpinButton);
	addMenuLayout(dacCodeLayout);
	//dac label
	QHBoxLayout *m_dacLabelLayout = new QHBoxLayout();
	m_dacLabel = new QLabel("mA", m_widget);
	m_dacLabelLayout->addWidget(m_dacLabel);
	m_dacLabelLayout->setAlignment(Qt::AlignRight);
	addMenuLayout(m_dacLabelLayout);
	//slew
	QHBoxLayout *slewLayout = new QHBoxLayout();
	m_slewOptions = new QComboBox(m_widget);
	m_slewOptions->addItem(QString("Disable"));
	m_slewOptions->addItem(QString("Enable"));
	(m_attrValues[OUTPUT_CHNL]["slew_en"].first().compare("0") == 0)
			? m_slewOptions->setCurrentText("Disable")
			: m_slewOptions->setCurrentText("Enable");
	slewLayout->addWidget(new QLabel("Slew", m_widget), 1);
	slewLayout->addWidget(m_slewOptions, 1);
	addMenuLayout(slewLayout);
	//slew step
	QHBoxLayout *slewStepLayout = new QHBoxLayout();
	m_slewStepOptions = new QComboBox(m_widget);
	setAvailableOptions(m_slewStepOptions, "slew_step_available");
	(m_attrValues[OUTPUT_CHNL].contains("slew_step") && m_attrValues[OUTPUT_CHNL]["slew_step"].size() > 0)
			? m_slewStepOptions->setCurrentText(m_attrValues[OUTPUT_CHNL]["slew_step"].first())
			: m_slewStepOptions->setCurrentText("ERROR");
	slewStepLayout->addWidget(new QLabel("Slew Step Size", m_widget), 1);
	slewStepLayout->addWidget(m_slewStepOptions, 1);
	addMenuLayout(slewStepLayout);
	//slew rate
	QHBoxLayout *slewRateLayout = new QHBoxLayout();
	m_slewRateOptions = new QComboBox(m_widget);
	setAvailableOptions(m_slewRateOptions, "slew_rate_available");
	(m_attrValues[OUTPUT_CHNL].contains("slew_rate") && m_attrValues[OUTPUT_CHNL]["slew_rate"].size() > 0)
			? m_slewRateOptions->setCurrentText(m_attrValues[OUTPUT_CHNL]["slew_rate"].first())
			: m_slewRateOptions->setCurrentText("ERROR");
	slewRateLayout->addWidget(new QLabel("Slew Rate (kHz)",m_widget),1);
	slewRateLayout->addWidget(m_slewRateOptions, 1);
	addMenuLayout(slewRateLayout);

	connectSignalsToSlots();
}

void CurrentOutMenu::connectSignalsToSlots()
{
	connect(m_dacCodeSpinButton, &PositionSpinButton::valueChanged, this, &CurrentOutMenu::dacCodeChanged);
	connect(m_slewStepOptions, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CurrentOutMenu::slewStepIndexChanged);
	connect(m_slewRateOptions, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CurrentOutMenu::slewRateIndexChanged);
	connect(m_slewOptions, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CurrentOutMenu::slewIndexChanged);
}

QString CurrentOutMenu::getInfoMessage()
{
	QString infoMessage = "\"" + m_chnlFunction + "\" configuration generates 2 context channels.\n" +
			"One of them is an input buffer capable channel (voltage_in)\n" +
			"which is related to the plot and the other one is an\n" +
			"output channel whose attributes can be changed from\n" +
			"this menu.";
	return infoMessage;
}

void CurrentOutMenu::setAvailableOptions(QComboBox *list, QString attrName)
{
	if (!m_attrValues.contains(OUTPUT_CHNL)) {
		qWarning() << "There is no output channel available for this function!";
		return;
	}
	QStringList availableValues = m_attrValues[OUTPUT_CHNL][attrName];
	for (const auto& slewValue : availableValues) {
		list->addItem(slewValue);
	}
}

void CurrentOutMenu::dacCodeChanged(double value)
{
	QString attrName("raw");
	if (!m_attrValues.contains(OUTPUT_CHNL)) {
		qWarning() << "There is no output channel available for this function!";
		return;
	}
	if (m_attrValues.contains(OUTPUT_CHNL)) {
		m_attrValues[OUTPUT_CHNL][attrName].clear();
		m_attrValues[OUTPUT_CHNL][attrName].push_back(QString::number((int)value));
		//by default the current value is measured in μA
		double val = convertFromRaw(value) * 0.001;
		m_dacLabel->clear();
		m_dacLabel->setText(QString::number(val)+" mA");

		Q_EMIT attrValuesChanged(attrName, OUTPUT_CHNL);
	}
}

void CurrentOutMenu::slewStepIndexChanged(int idx)
{
	QString attrName = "slew_step";
	if (!m_attrValues.contains(OUTPUT_CHNL)) {
		qWarning() << "There is no output channel available for this function!";
		return;
	}
	const auto& slewStep = m_attrValues[OUTPUT_CHNL]["slew_step_available"][idx];
	m_attrValues[OUTPUT_CHNL][attrName].clear();
	m_attrValues[OUTPUT_CHNL][attrName].push_back(slewStep);

	Q_EMIT attrValuesChanged(attrName, OUTPUT_CHNL);
}

void CurrentOutMenu::slewRateIndexChanged(int idx)
{
	QString attrName = "slew_rate";
	if (!m_attrValues.contains(OUTPUT_CHNL)) {
		qWarning() << "There is no output channel available for this function!";
		return;
	}
	const auto& slewStep = m_attrValues[OUTPUT_CHNL]["slew_rate_available"][idx];
	m_attrValues[OUTPUT_CHNL][attrName].clear();
	m_attrValues[OUTPUT_CHNL][attrName].push_back(slewStep);

	Q_EMIT attrValuesChanged(attrName, OUTPUT_CHNL);
}

void CurrentOutMenu::slewIndexChanged(int idx)
{
	QString attrName = "slew_en";
	if (!m_attrValues.contains(OUTPUT_CHNL)) {
		qWarning() << "There is no output channel available for this function!";
		return;
	}
	m_attrValues[OUTPUT_CHNL][attrName].clear();
	if (idx == SLEW_DISABLE_IDX) {
		m_attrValues[OUTPUT_CHNL][attrName].push_back(QString("0"));
	}
	else {
		m_attrValues[OUTPUT_CHNL][attrName].push_back(QString("1"));
	}

	Q_EMIT attrValuesChanged(attrName, OUTPUT_CHNL);
}

DiagnosticMenu::DiagnosticMenu(QWidget* parent, QString chnlFunction)
	: BufferMenu(parent, chnlFunction)
{}

DiagnosticMenu::~DiagnosticMenu()
{}

void DiagnosticMenu::init()
{
	QHBoxLayout *diagLayout = new QHBoxLayout();
	m_diagOptions = new QComboBox(m_widget);
	setAvailableOptions(m_diagOptions, "diag_function_available");
	m_diagOptions->setCurrentIndex(0);
	(m_attrValues[INPUT_CHNL].contains("diag_function") && m_attrValues[INPUT_CHNL]["diag_function"].size() > 0)
			? m_diagOptions->setCurrentText(m_attrValues[INPUT_CHNL]["diag_function"].first())
			: m_diagOptions->setCurrentText("ERROR");
	diagLayout->addWidget(new QLabel("Function", m_widget));
	diagLayout->addWidget(m_diagOptions);
	addMenuLayout(diagLayout);

	connectSignalsToSlots();
}

void DiagnosticMenu::connectSignalsToSlots()
{
	connect(m_diagOptions, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DiagnosticMenu::diagIndextChanged);
}

void DiagnosticMenu::setAvailableOptions(QComboBox *list, QString attrName)
{
	if (!m_attrValues.contains(INPUT_CHNL)) {
		qWarning() << "There is no input channel available for this function!";
		return;
	}
	QStringList availableValues = m_attrValues[INPUT_CHNL][attrName];
	for (const auto& slewValue : availableValues) {
		list->addItem(slewValue);
	}
}

void DiagnosticMenu::diagIndextChanged(int idx)
{
	QString attrName = "diag_function";
	if (!m_attrValues.contains(INPUT_CHNL)) {
		qWarning() << "There is no input channel available for this function!";
		return;
	}
	const auto& diagFunc = m_attrValues[INPUT_CHNL]["diag_function_available"][idx];
	m_attrValues[INPUT_CHNL][attrName].clear();
	m_attrValues[INPUT_CHNL][attrName].push_back(diagFunc);

	Q_EMIT attrValuesChanged(attrName, INPUT_CHNL);
}

WithoutAdvSettings::WithoutAdvSettings(QWidget* parent, QString chnlFunction)
	: BufferMenu(parent, chnlFunction)
{}

WithoutAdvSettings::~WithoutAdvSettings()
{}

void WithoutAdvSettings::init()
{
	QHBoxLayout *msgLayout = new QHBoxLayout();
	msgLayout->addWidget(new QLabel("No advanced settings available", m_widget));
	addMenuLayout(msgLayout);
}

void WithoutAdvSettings::connectSignalsToSlots()
{}

DigitalInMenu::DigitalInMenu(QWidget* parent, QString chnlFunction)
	: BufferMenu(parent, chnlFunction)
{}

DigitalInMenu::~DigitalInMenu()
{}

void DigitalInMenu::init()
{
	//threshold
	QHBoxLayout *thresholdLayout = new QHBoxLayout();
	m_thresholdSpinButton = new PositionSpinButton({
							{"mV", 1E0}
						     },
						     "Theshold", 0,
						     16000,
						     true, false, m_widget);
	thresholdLayout->addWidget(m_thresholdSpinButton);
	addMenuLayout(thresholdLayout);

	connectSignalsToSlots();
}

void DigitalInMenu::connectSignalsToSlots()
{
	connect(m_thresholdSpinButton, &PositionSpinButton::valueChanged, this, &DigitalInMenu::thresholdChanged);
}

void DigitalInMenu::thresholdChanged(double value)
{
	QString attrName("threshold");
	if (!m_attrValues.contains(INPUT_CHNL)) {
		qWarning() << "There is no input channel available for this function!";
		return;
	}
	if (m_attrValues[INPUT_CHNL].contains(attrName)) {
		m_attrValues[INPUT_CHNL][attrName].clear();
		m_attrValues[INPUT_CHNL][attrName].push_back(QString::number((int)value));
		Q_EMIT attrValuesChanged(attrName, INPUT_CHNL);
	} else {
		qWarning() << attrName.toUpper() + " attribute does not exist!";
	}
}


