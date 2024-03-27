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

#include "ad74413r/buffermenu.h"

#include "qdebug.h"
#include "qlineedit.h"

#include <iiowidgetfactory.h>

using namespace scopy::swiotrefactor;
BufferMenu::BufferMenu(QWidget *parent, QString chnlFunction, Connection *conn, iio_channel *chnl)
	: QWidget(parent)
	, m_widget(parent)
	, m_chnlFunction(chnlFunction)
	, m_connection(conn)
	, m_chnl(chnl)
{
	// channels sampling freq
	QHBoxLayout *samplingFreqLayout = new QHBoxLayout();
	m_samplingFreqOptions = new QComboBox(m_widget);
	samplingFreqLayout->addWidget(new QLabel("Sampling frequency", m_widget));
	samplingFreqLayout->addWidget(m_samplingFreqOptions);
	addMenuLayout(samplingFreqLayout);

	connectSignalsToSlots();
}

BufferMenu::~BufferMenu() {}

void BufferMenu::connectSignalsToSlots()
{
	connect(m_samplingFreqOptions, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&BufferMenu::onSamplingFreqChanged);
}

void BufferMenu::init()
{
	setAvailableOptions(m_samplingFreqOptions, "sampling_frequency_available");
	(m_attrValues[INPUT_CHNL].contains("sampling_frequency") &&
	 !m_attrValues[INPUT_CHNL]["sampling_frequency"].isEmpty())
		? m_samplingFreqOptions->setCurrentText(m_attrValues[INPUT_CHNL]["sampling_frequency"].first())
		: m_samplingFreqOptions->setCurrentText("ERROR");
	//	m_unitPerDivision->setValue(1);
	//	Q_EMIT m_unitPerDivision->valueChanged(m_unitPerDivision->value());
}

void BufferMenu::setAvailableOptions(QComboBox *list, QString attrName)
{
	if(!m_attrValues.contains(INPUT_CHNL)) {
		return;
	}
	QStringList availableValues = m_attrValues[INPUT_CHNL][attrName];
	for(const auto &srValue : qAsConst(availableValues)) {
		list->addItem(srValue);
	}
}

void BufferMenu::onSamplingFreqChanged(int idx)
{
	QString attrName = "sampling_frequency";
	if(!m_attrValues.contains(INPUT_CHNL)) {
		qWarning() << "There is no input channel available!";
		return;
	}
	const auto &srAvail = m_attrValues[INPUT_CHNL]["sampling_frequency_available"][idx];
	m_attrValues[INPUT_CHNL][attrName].clear();
	m_attrValues[INPUT_CHNL][attrName].push_back(srAvail);

	Q_EMIT attrValuesChanged(attrName, INPUT_CHNL);
	// TBD should be emitted on readback
	Q_EMIT samplingFrequencyUpdated(srAvail.toInt());
}

QString BufferMenu::getInfoMessage()
{
	QString defaultMessage = "\"" + m_chnlFunction + "\" configuration generates\n" +
		"a buffer capable channel which is directly\n" + "related to the plot. The channel attributes\n" +
		"can be changed through this menu.";
	return defaultMessage;
}

void BufferMenu::addMenuLayout(QBoxLayout *layout) { m_menuLayers.push_back(layout); }

void BufferMenu::setupVerticalSettingsMenu(QWidget *settingsWidget, QString unit, double yMin, double yMax)
{
	// Y-UNIT-PER-DIV
	QHBoxLayout *yMinMaxLayout = new QHBoxLayout(settingsWidget);
	yMinMaxLayout->setMargin(0);
	yMinMaxLayout->setSpacing(10);

	m_unitPerDivision = new PositionSpinButton(
		{
			{unit, 1e0},
		},
		"Unit/Div", 10e-6, yMax, false, false, settingsWidget);

	yMinMaxLayout->addWidget(m_unitPerDivision);
	settingsWidget->layout()->addItem(yMinMaxLayout);

	// Connects
	connect(m_unitPerDivision, &PositionSpinButton::valueChanged, this, &BufferMenu::setUnitPerDivision);
	connect(this, &BufferMenu::unitPerDivisionChanged, this, [=, this](double unitPerDiv) {
		QSignalBlocker block(m_unitPerDivision);
		m_unitPerDivision->setValue(unitPerDiv);
	});
}

void BufferMenu::setAttrValues(QMap<QString, QMap<QString, QStringList>> values) { m_attrValues = values; }

QMap<QString, QMap<QString, QStringList>> BufferMenu::getAttrValues() { return m_attrValues; }

QVector<QBoxLayout *> BufferMenu::getMenuLayers() { return m_menuLayers; }

double BufferMenu::convertFromRaw(int rawValue, QString chnlType)
{
	double value = 0.0;
	if(m_attrValues.contains(chnlType) && m_attrValues[chnlType].contains("offset") &&
	   m_attrValues[chnlType].contains("scale")) {
		double offset = m_attrValues[chnlType]["offset"].first().toDouble();
		double scale = m_attrValues[chnlType]["scale"].first().toDouble();
		value = (rawValue + offset) * scale;
	}
	return value;
}

void BufferMenu::onAttrWritten(QMap<QString, QMap<QString, QStringList>> values)
{
	m_attrValues = values;
	Q_EMIT mapUpdated();
}

CurrentInLoopMenu::CurrentInLoopMenu(QWidget *parent, QString chnlFunction, Connection *conn, iio_channel *chnl)
	: BufferMenu(parent, chnlFunction, conn, chnl)
{
	// dac code
	QHBoxLayout *dacCodeLayout = new QHBoxLayout();
	m_dacCodeSpinButton = new PositionSpinButton({{"value", 1E0}}, "DAC Code", 0, 8191, true, false, m_widget);
	dacCodeLayout->addWidget(m_dacCodeSpinButton);
	addMenuLayout(dacCodeLayout);
	// dac label
	QHBoxLayout *m_dacLabelLayout = new QHBoxLayout();
	m_dacLabel = new QLabel("mA", m_widget);
	m_dacLabelLayout->addWidget(m_dacLabel);
	m_dacLabelLayout->setAlignment(Qt::AlignRight);
	addMenuLayout(m_dacLabelLayout);

	connectSignalsToSlots();
}

CurrentInLoopMenu::~CurrentInLoopMenu() {}

void CurrentInLoopMenu::init()
{
	BufferMenu::init();
	m_unitPerDivision->setValue(0.001);
	Q_EMIT m_unitPerDivision->valueChanged(m_unitPerDivision->value());
	(m_attrValues[OUTPUT_CHNL].contains("raw") && !m_attrValues[OUTPUT_CHNL]["raw"].isEmpty())
		? m_dacCodeSpinButton->setValue(m_attrValues[OUTPUT_CHNL]["raw"].first().toDouble())
		: m_dacCodeSpinButton->setValue(0);
}

void CurrentInLoopMenu::connectSignalsToSlots()
{
	connect(m_dacCodeSpinButton, &PositionSpinButton::valueChanged, this, &CurrentInLoopMenu::dacCodeChanged);
	connect(this, SIGNAL(mapUpdated()), this, SLOT(onMapUpdated()));
}

QString CurrentInLoopMenu::getInfoMessage()
{
	QString infoMessage = "\"" + m_chnlFunction + "\" configuration generates 2 context channels.\n" +
		"One of them is an input buffer capable channel (current_in)\n" +
		"which is related to the plot and the other one is an output\n" +
		"channel (current_out) whose attributes can be changed from\n" + "this menu.";
	return infoMessage;
}

void CurrentInLoopMenu::dacCodeChanged(double value)
{
	QString attrName("raw");
	if(!m_attrValues.contains(OUTPUT_CHNL)) {
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

void CurrentInLoopMenu::onMapUpdated()
{
	m_dacCodeSpinButton->blockSignals(true);
	(m_attrValues[OUTPUT_CHNL].contains("raw") && !m_attrValues[OUTPUT_CHNL]["raw"].isEmpty())
		? m_dacCodeSpinButton->setValue(m_attrValues[OUTPUT_CHNL]["raw"].first().toDouble())
		: m_dacCodeSpinButton->setValue(0);
	m_dacCodeSpinButton->blockSignals(false);
}

DigitalInLoopMenu::DigitalInLoopMenu(QWidget *parent, QString chnlFunction, Connection *conn, iio_channel *chnl)
	: BufferMenu(parent, chnlFunction, conn, chnl)
{
	// threshold
	QVBoxLayout *thresholdLayout = new QVBoxLayout();
	QRegExp regExp("([0-9]|[1-9][0-9]{1,3}|1[0-5][0-9]{3}|16000)");
	QRegExpValidator *validator = new QRegExpValidator(regExp, this);
	m_titleLabel = new QLabel("Threshold (0 - 16000 mV)", m_widget);
	m_titleLabel->setStyleSheet("font-size: 14px;");
	m_thresholdLineEdit = new QLineEdit(m_widget);
	m_thresholdLineEdit->setStyleSheet(
		"background: transparent; height: 20px; width: 75px; font-size: 18px; border: 0px; bottom: 10px;");
	m_thresholdLineEdit->setValidator(validator);

	QFrame *frame = new QFrame(m_widget);
	frame->setFrameShape(QFrame::HLine);
	frame->setStyleSheet("color: #4a64ff;");
	thresholdLayout->addWidget(m_titleLabel);
	thresholdLayout->addWidget(m_thresholdLineEdit);
	thresholdLayout->addWidget(frame);
	addMenuLayout(thresholdLayout);
	// dac code
	QHBoxLayout *dacCodeLayout = new QHBoxLayout();
	m_dacCodeSpinButton = new PositionSpinButton({{"value", 1E0}}, "DAC Code", 0, 8191, true, false, m_widget);

	dacCodeLayout->addWidget(m_dacCodeSpinButton);
	addMenuLayout(dacCodeLayout);
	// dac label
	QHBoxLayout *m_dacLabelLayout = new QHBoxLayout();
	m_dacLabel = new QLabel("mA", m_widget);
	m_dacLabelLayout->addWidget(m_dacLabel);
	m_dacLabelLayout->setAlignment(Qt::AlignRight);
	addMenuLayout(m_dacLabelLayout);

	connectSignalsToSlots();
}

DigitalInLoopMenu::~DigitalInLoopMenu() {}

void DigitalInLoopMenu::init()
{
	BufferMenu::init();
	m_unitPerDivision->setValue(0.001);
	Q_EMIT m_unitPerDivision->valueChanged(m_unitPerDivision->value());
	(m_attrValues[INPUT_CHNL].contains("threshold") && !m_attrValues[INPUT_CHNL]["threshold"].isEmpty())
		? m_thresholdLineEdit->setText(m_attrValues[INPUT_CHNL]["threshold"].first()),
		m_thresholdLineEdit->setPlaceholderText(m_attrValues[INPUT_CHNL]["threshold"].first())
		: m_thresholdLineEdit->setText("0"),
		m_thresholdLineEdit->setPlaceholderText("0");

	(m_attrValues[OUTPUT_CHNL].contains("raw") && !m_attrValues[OUTPUT_CHNL]["raw"].isEmpty())
		? m_dacCodeSpinButton->setValue(m_attrValues[OUTPUT_CHNL]["raw"].first().toDouble())
		: m_dacCodeSpinButton->setValue(0);
}

void DigitalInLoopMenu::connectSignalsToSlots()
{
	connect(m_dacCodeSpinButton, &PositionSpinButton::valueChanged, this, &DigitalInLoopMenu::dacCodeChanged);
	connect(m_thresholdLineEdit, &QLineEdit::returnPressed, this, &DigitalInLoopMenu::thresholdChanged);
	connect(this, SIGNAL(mapUpdated()), this, SLOT(onMapUpdated()));
	connect(this, SIGNAL(broadcastThresholdReadBackward(QString)), this, SLOT(onBroadcastThresholdRead(QString)));
	connect(this, &BufferMenu::thresholdControlEnable, this, &DigitalInLoopMenu::onThresholdControlEnable);
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
	if(!m_attrValues.contains(OUTPUT_CHNL)) {
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

void DigitalInLoopMenu::thresholdChanged()
{
	QString attrName("threshold");
	if(!m_attrValues.contains(INPUT_CHNL)) {
		qWarning() << "There is no input channel available for this function!";
		return;
	}
	if(m_attrValues[INPUT_CHNL].contains(attrName)) {
		m_attrValues[INPUT_CHNL][attrName].clear();
		m_attrValues[INPUT_CHNL][attrName].push_back(m_thresholdLineEdit->text());
		Q_EMIT attrValuesChanged(attrName, INPUT_CHNL);
	} else {
		qWarning() << attrName.toUpper() + " attribute does not exist!";
	}
}

void DigitalInLoopMenu::onMapUpdated()
{
	m_thresholdLineEdit->blockSignals(true);
	if(m_attrValues[INPUT_CHNL].contains("threshold") && !m_attrValues[INPUT_CHNL]["threshold"].isEmpty()) {
		if(m_attrValues[INPUT_CHNL]["threshold"].first().compare(m_thresholdLineEdit->text()) != 0) {
			Q_EMIT broadcastThresholdReadForward(m_attrValues[INPUT_CHNL]["threshold"].first());
		}
		m_thresholdLineEdit->setText(m_attrValues[INPUT_CHNL]["threshold"].first());
		m_thresholdLineEdit->setPlaceholderText(m_attrValues[INPUT_CHNL]["threshold"].first());
	} else {
		m_thresholdLineEdit->setText(0);
	}
	m_thresholdLineEdit->blockSignals(false);
	m_dacCodeSpinButton->blockSignals(true);
	(m_attrValues[OUTPUT_CHNL].contains("raw") && !m_attrValues[OUTPUT_CHNL]["raw"].isEmpty())
		? m_dacCodeSpinButton->setValue(m_attrValues[OUTPUT_CHNL]["raw"].first().toDouble())
		: m_dacCodeSpinButton->setValue(0);
	m_dacCodeSpinButton->blockSignals(false);
}

void DigitalInLoopMenu::onBroadcastThresholdRead(QString value)
{
	m_thresholdLineEdit->blockSignals(true);
	if(value.compare(m_thresholdLineEdit->text()) != 0) {
		m_thresholdLineEdit->setText(value);
		m_thresholdLineEdit->setPlaceholderText(value);
		m_attrValues[INPUT_CHNL]["threshold"].first() = value;
	}
	m_thresholdLineEdit->blockSignals(false);
}

void DigitalInLoopMenu::onThresholdControlEnable(bool enabled) { m_thresholdLineEdit->setEnabled(enabled); }

VoltageOutMenu::VoltageOutMenu(QWidget *parent, QString chnlFunction, Connection *conn, iio_channel *chnl)
	: BufferMenu(parent, chnlFunction, conn, chnl)
{
	// dac code
	//	IIOWidget *dacCode = IIOWidgetFactory::buildSingle(IIOWidgetFactory::CMDQAttrData |
	// IIOWidgetFactory::RangeUi,
	//							   {.connection = const_cast<Connection
	//*>(m_connection), 							    .channel = const_cast<iio_channel
	//*>(m_chnl), .data = "raw", 							    .constDataOptions = "[0 1
	// 8191]"},
	// m_widget); 	dacCodeLayout->addWidget(dacCode);
	QHBoxLayout *dacCodeLayout = new QHBoxLayout();
	m_dacCodeSpinButton = new PositionSpinButton({{"value", 1E0}}, "DAC Code", 0, 8191, true, false, m_widget);
	dacCodeLayout->addWidget(m_dacCodeSpinButton);
	addMenuLayout(dacCodeLayout);
	// dac label
	QHBoxLayout *m_dacLabelLayout = new QHBoxLayout();
	m_dacLabel = new QLabel("V", m_widget);
	m_dacLabelLayout->addWidget(m_dacLabel);
	m_dacLabelLayout->setAlignment(Qt::AlignRight);
	addMenuLayout(m_dacLabelLayout);
	// slew
	QHBoxLayout *slewLayout = new QHBoxLayout();
	//	IIOWidget *slewOptions =
	//		IIOWidgetFactory::buildSingle(IIOWidgetFactory::CMDQAttrData | IIOWidgetFactory::ComboUi,
	//					      {.connection = const_cast<Connection *>(m_connection),
	//					       .channel = const_cast<iio_channel *>(m_chnl),
	//					       .data = "slew_en",
	//					       .constDataOptions = "0 1"},
	//					      m_widget);
	m_slewOptions = new QComboBox(m_widget);
	m_slewOptions->addItem(QString("Disable"));
	m_slewOptions->addItem(QString("Enable"));

	slewLayout->addWidget(new QLabel("Slew", m_widget), 1);
	slewLayout->addWidget(m_slewOptions, 1);
	//	slewLayout->addWidget(slewOptions);
	addMenuLayout(slewLayout);
	// slew step
	QHBoxLayout *slewStepLayout = new QHBoxLayout();
	//	IIOWidget *slewStep = IIOWidgetFactory::buildSingle(IIOWidgetFactory::CMDQAttrData |
	// IIOWidgetFactory::ComboUi,
	//							    {.connection = const_cast<Connection
	//*>(m_connection), 							     .channel = const_cast<iio_channel
	//*>(m_chnl), .data = "slew_step", 							     .iioDataOptions =
	//"slew_step_available"}, 							    m_widget);
	// slewStepLayout->addWidget(slewStep);
	m_slewStepOptions = new QComboBox(m_widget);

	slewStepLayout->addWidget(new QLabel("Slew Step Size", m_widget), 1);
	slewStepLayout->addWidget(m_slewStepOptions, 1);
	addMenuLayout(slewStepLayout);
	// slew rate
	QHBoxLayout *slewRateLayout = new QHBoxLayout();
	//	IIOWidget *slewRate = IIOWidgetFactory::buildSingle(IIOWidgetFactory::CMDQAttrData |
	// IIOWidgetFactory::ComboUi,
	//							    {.connection = const_cast<Connection
	//*>(m_connection), 							     .channel = const_cast<iio_channel
	//*>(m_chnl), .data = "slew_rate", 							     .iioDataOptions =
	//"slew_rate_available"}, 							    m_widget);
	// slewRateLayout->addWidget(slewRate);
	m_slewRateOptions = new QComboBox(m_widget);

	slewRateLayout->addWidget(new QLabel("Slew Rate (kHz)", m_widget), 1);
	slewRateLayout->addWidget(m_slewRateOptions, 1);
	addMenuLayout(slewRateLayout);

	connectSignalsToSlots();
}

VoltageOutMenu::~VoltageOutMenu() {}

void VoltageOutMenu::init()
{
	BufferMenu::init();
	setAvailableOptions(m_slewStepOptions, "slew_step_available");
	setAvailableOptions(m_slewRateOptions, "slew_rate_available");

	(m_attrValues[OUTPUT_CHNL].contains("raw") && !m_attrValues[OUTPUT_CHNL]["raw"].isEmpty())
		? m_dacCodeSpinButton->setValue(m_attrValues[OUTPUT_CHNL]["raw"].first().toDouble())
		: m_dacCodeSpinButton->setValue(0);

	if(m_attrValues[OUTPUT_CHNL].contains("slew_en") && !m_attrValues[OUTPUT_CHNL]["slew_en"].isEmpty()) {
		(m_attrValues[OUTPUT_CHNL]["slew_en"].first().compare("0") == 0)
			? m_slewOptions->setCurrentText("Disable")
			: m_slewOptions->setCurrentText("Enable");
	}

	(m_attrValues[OUTPUT_CHNL].contains("slew_step") && !m_attrValues[OUTPUT_CHNL]["slew_step"].isEmpty())
		? m_slewStepOptions->setCurrentText(m_attrValues[OUTPUT_CHNL]["slew_step"].first())
		: m_slewStepOptions->setCurrentText("ERROR");

	(m_attrValues[OUTPUT_CHNL].contains("slew_rate") && !m_attrValues[OUTPUT_CHNL]["slew_rate"].isEmpty())
		? m_slewRateOptions->setCurrentText(m_attrValues[OUTPUT_CHNL]["slew_rate"].first())
		: m_slewRateOptions->setCurrentText("ERROR");
}

void VoltageOutMenu::connectSignalsToSlots()
{
	connect(m_dacCodeSpinButton, &PositionSpinButton::valueChanged, this, &VoltageOutMenu::dacCodeChanged);
	connect(m_slewStepOptions, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&VoltageOutMenu::slewStepIndexChanged);
	connect(m_slewRateOptions, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&VoltageOutMenu::slewRateIndexChanged);
	connect(m_slewOptions, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&VoltageOutMenu::slewIndexChanged);
	connect(this, SIGNAL(mapUpdated()), this, SLOT(onMapUpdated()));
}

QString VoltageOutMenu::getInfoMessage()
{
	QString infoMessage = "\"" + m_chnlFunction + "\" configuration generates 2 context channels.\n" +
		"One of them is an input buffer capable channel (current_in)\n" +
		"which is related to the plot and the other one is an\n" +
		"output channel whose attributes can be changed from\n" + "this menu.";
	return infoMessage;
}

void VoltageOutMenu::setAvailableOptions(QComboBox *list, QString attrName)
{
	if(!m_attrValues.contains(OUTPUT_CHNL)) {
		qWarning() << "There is no output channel available for this function!";
		return;
	}
	QStringList availableValues = m_attrValues[OUTPUT_CHNL][attrName];
	for(const auto &slewValue : qAsConst(availableValues)) {
		list->addItem(slewValue);
	}
}

void VoltageOutMenu::dacCodeChanged(double value)
{
	QString attrName("raw");
	if(!m_attrValues.contains(OUTPUT_CHNL)) {
		qWarning() << "There is no output channel available for this function!";
		return;
	}
	m_attrValues[OUTPUT_CHNL][attrName].clear();
	m_attrValues[OUTPUT_CHNL][attrName].push_back(QString::number((int)value));
	// the convertFromRaw return value is in mV that's why we multiply by 10^(-3)
	double val = convertFromRaw(value) * 0.001;
	m_dacLabel->clear();
	m_dacLabel->setText(QString::number(val) + " V");

	Q_EMIT attrValuesChanged(attrName, OUTPUT_CHNL);
}

void VoltageOutMenu::slewStepIndexChanged(int idx)
{
	QString attrName = "slew_step";
	if(!m_attrValues.contains(OUTPUT_CHNL)) {
		qWarning() << "There is no output channel available for this function!";
		return;
	}
	const auto &slewStep = m_attrValues[OUTPUT_CHNL]["slew_step_available"][idx];
	m_attrValues[OUTPUT_CHNL][attrName].clear();
	m_attrValues[OUTPUT_CHNL][attrName].push_back(slewStep);

	Q_EMIT attrValuesChanged(attrName, OUTPUT_CHNL);
}

void VoltageOutMenu::slewRateIndexChanged(int idx)
{
	QString attrName = "slew_rate";
	if(!m_attrValues.contains(OUTPUT_CHNL)) {
		qWarning() << "There is no output channel available for this function!";
		return;
	}
	const auto &slewStep = m_attrValues[OUTPUT_CHNL]["slew_rate_available"][idx];
	m_attrValues[OUTPUT_CHNL][attrName].clear();
	m_attrValues[OUTPUT_CHNL][attrName].push_back(slewStep);

	Q_EMIT attrValuesChanged(attrName, OUTPUT_CHNL);
}

void VoltageOutMenu::slewIndexChanged(int idx)
{
	QString attrName = "slew_en";
	if(!m_attrValues.contains(OUTPUT_CHNL)) {
		qWarning() << "There is no output channel available for this function!";
		return;
	}
	m_attrValues[OUTPUT_CHNL][attrName].clear();
	if(idx == SLEW_DISABLE_IDX) {
		m_attrValues[OUTPUT_CHNL][attrName].push_back(QString("0"));
	} else {
		m_attrValues[OUTPUT_CHNL][attrName].push_back(QString("1"));
	}

	Q_EMIT attrValuesChanged(attrName, OUTPUT_CHNL);
}

void VoltageOutMenu::onMapUpdated()
{
	m_dacCodeSpinButton->blockSignals(true);
	(m_attrValues[OUTPUT_CHNL].contains("raw") && !m_attrValues[OUTPUT_CHNL]["raw"].isEmpty())
		? m_dacCodeSpinButton->setValue(m_attrValues[OUTPUT_CHNL]["raw"].first().toDouble())
		: m_dacCodeSpinButton->setValue(0);
	m_dacCodeSpinButton->blockSignals(false);

	m_slewOptions->blockSignals(true);
	(m_attrValues[OUTPUT_CHNL]["slew_en"].first().compare("0") == 0) ? m_slewOptions->setCurrentText("Disable")
									 : m_slewOptions->setCurrentText("Enable");
	m_slewOptions->blockSignals(false);

	m_slewStepOptions->blockSignals(true);
	(m_attrValues[OUTPUT_CHNL].contains("slew_step") && !m_attrValues[OUTPUT_CHNL]["slew_step"].isEmpty())
		? m_slewStepOptions->setCurrentText(m_attrValues[OUTPUT_CHNL]["slew_step"].first())
		: m_slewStepOptions->setCurrentText("ERROR");
	m_slewStepOptions->blockSignals(false);

	m_slewRateOptions->blockSignals(true);
	(m_attrValues[OUTPUT_CHNL].contains("slew_rate") && !m_attrValues[OUTPUT_CHNL]["slew_rate"].isEmpty())
		? m_slewRateOptions->setCurrentText(m_attrValues[OUTPUT_CHNL]["slew_rate"].first())
		: m_slewRateOptions->setCurrentText("ERROR");
	m_slewRateOptions->blockSignals(false);
}

CurrentOutMenu::CurrentOutMenu(QWidget *parent, QString chnlFunction, Connection *conn, iio_channel *chnl)
	: BufferMenu(parent, chnlFunction, conn, chnl)
{
	// dac code
	QHBoxLayout *dacCodeLayout = new QHBoxLayout();
	m_dacCodeSpinButton = new PositionSpinButton({{"value", 1E0}}, "DAC Code", 0, 8196, true, false, m_widget);
	dacCodeLayout->addWidget(m_dacCodeSpinButton);
	addMenuLayout(dacCodeLayout);
	// dac label
	QHBoxLayout *m_dacLabelLayout = new QHBoxLayout();
	m_dacLabel = new QLabel("mA", m_widget);
	m_dacLabelLayout->addWidget(m_dacLabel);
	m_dacLabelLayout->setAlignment(Qt::AlignRight);
	addMenuLayout(m_dacLabelLayout);
	// slew
	QHBoxLayout *slewLayout = new QHBoxLayout();
	m_slewOptions = new QComboBox(m_widget);
	m_slewOptions->addItem(QString("Disable"));
	m_slewOptions->addItem(QString("Enable"));

	slewLayout->addWidget(new QLabel("Slew", m_widget), 1);
	slewLayout->addWidget(m_slewOptions, 1);
	addMenuLayout(slewLayout);
	// slew step
	QHBoxLayout *slewStepLayout = new QHBoxLayout();
	m_slewStepOptions = new QComboBox(m_widget);

	slewStepLayout->addWidget(new QLabel("Slew Step Size", m_widget), 1);
	slewStepLayout->addWidget(m_slewStepOptions, 1);
	addMenuLayout(slewStepLayout);
	// slew rate
	QHBoxLayout *slewRateLayout = new QHBoxLayout();
	m_slewRateOptions = new QComboBox(m_widget);

	slewRateLayout->addWidget(new QLabel("Slew Rate (kHz)", m_widget), 1);
	slewRateLayout->addWidget(m_slewRateOptions, 1);
	addMenuLayout(slewRateLayout);

	connectSignalsToSlots();
}

CurrentOutMenu::~CurrentOutMenu() {}

void CurrentOutMenu::init()
{
	BufferMenu::init();
	setAvailableOptions(m_slewStepOptions, "slew_step_available");
	setAvailableOptions(m_slewRateOptions, "slew_rate_available");

	(m_attrValues[OUTPUT_CHNL].contains("raw") && !m_attrValues[OUTPUT_CHNL]["raw"].isEmpty())
		? m_dacCodeSpinButton->setValue(m_attrValues[OUTPUT_CHNL]["raw"].first().toDouble())
		: m_dacCodeSpinButton->setValue(0);

	if(m_attrValues[OUTPUT_CHNL].contains("slew_en") && !m_attrValues[OUTPUT_CHNL]["slew_en"].isEmpty()) {
		(m_attrValues[OUTPUT_CHNL]["slew_en"].first().compare("0") == 0)
			? m_slewOptions->setCurrentText("Disable")
			: m_slewOptions->setCurrentText("Enable");
	}

	(m_attrValues[OUTPUT_CHNL].contains("slew_step") && !m_attrValues[OUTPUT_CHNL]["slew_step"].isEmpty())
		? m_slewStepOptions->setCurrentText(m_attrValues[OUTPUT_CHNL]["slew_step"].first())
		: m_slewStepOptions->setCurrentText("ERROR");

	(m_attrValues[OUTPUT_CHNL].contains("slew_rate") && !m_attrValues[OUTPUT_CHNL]["slew_rate"].isEmpty())
		? m_slewRateOptions->setCurrentText(m_attrValues[OUTPUT_CHNL]["slew_rate"].first())
		: m_slewRateOptions->setCurrentText("ERROR");
}

void CurrentOutMenu::connectSignalsToSlots()
{
	connect(m_dacCodeSpinButton, &PositionSpinButton::valueChanged, this, &CurrentOutMenu::dacCodeChanged);
	connect(m_slewStepOptions, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&CurrentOutMenu::slewStepIndexChanged);
	connect(m_slewRateOptions, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&CurrentOutMenu::slewRateIndexChanged);
	connect(m_slewOptions, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&CurrentOutMenu::slewIndexChanged);
	connect(this, SIGNAL(mapUpdated()), this, SLOT(onMapUpdated()));
}

QString CurrentOutMenu::getInfoMessage()
{
	QString infoMessage = "\"" + m_chnlFunction + "\" configuration generates 2 context channels.\n" +
		"One of them is an input buffer capable channel (voltage_in)\n" +
		"which is related to the plot and the other one is an\n" +
		"output channel whose attributes can be changed from\n" + "this menu.";
	return infoMessage;
}

void CurrentOutMenu::setAvailableOptions(QComboBox *list, QString attrName)
{
	if(!m_attrValues.contains(OUTPUT_CHNL)) {
		qWarning() << "There is no output channel available for this function!";
		return;
	}
	QStringList availableValues = m_attrValues[OUTPUT_CHNL][attrName];
	for(const auto &slewValue : availableValues) {
		list->addItem(slewValue);
	}
}

void CurrentOutMenu::dacCodeChanged(double value)
{
	QString attrName("raw");
	if(!m_attrValues.contains(OUTPUT_CHNL)) {
		qWarning() << "There is no output channel available for this function!";
		return;
	}
	if(m_attrValues.contains(OUTPUT_CHNL)) {
		m_attrValues[OUTPUT_CHNL][attrName].clear();
		m_attrValues[OUTPUT_CHNL][attrName].push_back(QString::number((int)value));
		double val = convertFromRaw(value);
		m_dacLabel->clear();
		m_dacLabel->setText(QString::number(val) + " mA");

		Q_EMIT attrValuesChanged(attrName, OUTPUT_CHNL);
	}
}

void CurrentOutMenu::slewStepIndexChanged(int idx)
{
	QString attrName = "slew_step";
	if(!m_attrValues.contains(OUTPUT_CHNL)) {
		qWarning() << "There is no output channel available for this function!";
		return;
	}
	const auto &slewStep = m_attrValues[OUTPUT_CHNL]["slew_step_available"][idx];
	m_attrValues[OUTPUT_CHNL][attrName].clear();
	m_attrValues[OUTPUT_CHNL][attrName].push_back(slewStep);

	Q_EMIT attrValuesChanged(attrName, OUTPUT_CHNL);
}

void CurrentOutMenu::slewRateIndexChanged(int idx)
{
	QString attrName = "slew_rate";
	if(!m_attrValues.contains(OUTPUT_CHNL)) {
		qWarning() << "There is no output channel available for this function!";
		return;
	}
	const auto &slewStep = m_attrValues[OUTPUT_CHNL]["slew_rate_available"][idx];
	m_attrValues[OUTPUT_CHNL][attrName].clear();
	m_attrValues[OUTPUT_CHNL][attrName].push_back(slewStep);

	Q_EMIT attrValuesChanged(attrName, OUTPUT_CHNL);
}

void CurrentOutMenu::slewIndexChanged(int idx)
{
	QString attrName = "slew_en";
	if(!m_attrValues.contains(OUTPUT_CHNL)) {
		qWarning() << "There is no output channel available for this function!";
		return;
	}
	m_attrValues[OUTPUT_CHNL][attrName].clear();
	if(idx == SLEW_DISABLE_IDX) {
		m_attrValues[OUTPUT_CHNL][attrName].push_back(QString("0"));
	} else {
		m_attrValues[OUTPUT_CHNL][attrName].push_back(QString("1"));
	}

	Q_EMIT attrValuesChanged(attrName, OUTPUT_CHNL);
}

void CurrentOutMenu::onMapUpdated()
{
	m_dacCodeSpinButton->blockSignals(true);
	(m_attrValues[OUTPUT_CHNL].contains("raw") && !m_attrValues[OUTPUT_CHNL]["raw"].isEmpty())
		? m_dacCodeSpinButton->setValue(m_attrValues[OUTPUT_CHNL]["raw"].first().toDouble())
		: m_dacCodeSpinButton->setValue(0);
	m_dacCodeSpinButton->blockSignals(false);

	m_slewOptions->blockSignals(true);
	(m_attrValues[OUTPUT_CHNL]["slew_en"].first().compare("0") == 0) ? m_slewOptions->setCurrentText("Disable")
									 : m_slewOptions->setCurrentText("Enable");
	m_slewOptions->blockSignals(false);

	m_slewStepOptions->blockSignals(true);
	(m_attrValues[OUTPUT_CHNL].contains("slew_step") && !m_attrValues[OUTPUT_CHNL]["slew_step"].isEmpty())
		? m_slewStepOptions->setCurrentText(m_attrValues[OUTPUT_CHNL]["slew_step"].first())
		: m_slewStepOptions->setCurrentText("ERROR");
	m_slewStepOptions->blockSignals(false);

	m_slewRateOptions->blockSignals(true);
	(m_attrValues[OUTPUT_CHNL].contains("slew_rate") && !m_attrValues[OUTPUT_CHNL]["slew_rate"].isEmpty())
		? m_slewRateOptions->setCurrentText(m_attrValues[OUTPUT_CHNL]["slew_rate"].first())
		: m_slewRateOptions->setCurrentText("ERROR");
	m_slewRateOptions->blockSignals(false);
}

DiagnosticMenu::DiagnosticMenu(QWidget *parent, QString chnlFunction, Connection *conn, iio_channel *chnl)
	: BufferMenu(parent, chnlFunction, conn, chnl)
{
	QHBoxLayout *diagLayout = new QHBoxLayout();
	m_diagOptions = new QComboBox(m_widget);
	diagLayout->addWidget(new QLabel("Function", m_widget));
	diagLayout->addWidget(m_diagOptions);
	addMenuLayout(diagLayout);

	connectSignalsToSlots();
}

DiagnosticMenu::~DiagnosticMenu() {}

void DiagnosticMenu::init()
{
	BufferMenu::init();
	setAvailableOptions(m_diagOptions, "diag_function_available");
	(m_attrValues[INPUT_CHNL].contains("diag_function") && !m_attrValues[INPUT_CHNL]["diag_function"].isEmpty())
		? m_diagOptions->setCurrentText(m_attrValues[INPUT_CHNL]["diag_function"].first())
		: m_diagOptions->setCurrentText("ERROR");
}

void DiagnosticMenu::connectSignalsToSlots()
{
	connect(m_diagOptions, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&DiagnosticMenu::diagIndexChanged);
	connect(this, SIGNAL(mapUpdated()), this, SLOT(onMapUpdated()));
}

void DiagnosticMenu::setAvailableOptions(QComboBox *list, QString attrName)
{
	if(!m_attrValues.contains(INPUT_CHNL)) {
		qWarning() << "There is no input channel available for this function!";
		return;
	}
	QStringList availableValues = m_attrValues[INPUT_CHNL][attrName];
	for(const auto &slewValue : qAsConst(availableValues)) {
		list->addItem(slewValue);
	}
}

void DiagnosticMenu::diagIndexChanged(int idx)
{
	QString attrName = "diag_function";
	if(!m_attrValues.contains(INPUT_CHNL)) {
		qWarning() << "There is no input channel available for this function!";
		return;
	}
	const auto &diagFunc = m_attrValues[INPUT_CHNL]["diag_function_available"][idx];
	m_attrValues[INPUT_CHNL][attrName].clear();
	m_attrValues[INPUT_CHNL][attrName].push_back(diagFunc);

	Q_EMIT attrValuesChanged(attrName, INPUT_CHNL);
}

void DiagnosticMenu::onMapUpdated()
{
	m_diagOptions->blockSignals(true);
	(m_attrValues[INPUT_CHNL].contains("diag_function") && !m_attrValues[INPUT_CHNL]["diag_function"].isEmpty())
		? m_diagOptions->setCurrentText(m_attrValues[INPUT_CHNL]["diag_function"].first())
		: m_diagOptions->setCurrentText("ERROR");
	m_diagOptions->blockSignals(false);
	Q_EMIT diagnosticFunctionUpdated();
}

WithoutAdvSettings::WithoutAdvSettings(QWidget *parent, QString chnlFunction, Connection *conn, iio_channel *chnl)
	: BufferMenu(parent, chnlFunction, conn, chnl)
{
	QHBoxLayout *msgLayout = new QHBoxLayout();
	msgLayout->addWidget(new QLabel("No advanced settings available", m_widget));
	addMenuLayout(msgLayout);
}

WithoutAdvSettings::~WithoutAdvSettings() {}

void WithoutAdvSettings::init() { BufferMenu::init(); }

void WithoutAdvSettings::connectSignalsToSlots() {}

DigitalInMenu::DigitalInMenu(QWidget *parent, QString chnlFunction, Connection *conn, iio_channel *chnl)
	: BufferMenu(parent, chnlFunction, conn, chnl)
{
	QVBoxLayout *thresholdLayout = new QVBoxLayout();
	QRegExp regExp("([0-9]|[1-9][0-9]{1,3}|1[0-5][0-9]{3}|16000)");
	QRegExpValidator *validator = new QRegExpValidator(regExp, this);

	m_titleLabel = new QLabel("Threshold (0 - 16000 mV)", m_widget);
	m_titleLabel->setStyleSheet("font-size: 14px;");
	m_thresholdLineEdit = new QLineEdit(m_widget);
	m_thresholdLineEdit->setStyleSheet(
		"background: transparent; height: 20px; width: 75px; font-size: 18px; border: 0px; bottom: 10px;");
	m_thresholdLineEdit->setValidator(validator);
	QFrame *frame = new QFrame(m_widget);
	frame->setFrameShape(QFrame::HLine);
	frame->setStyleSheet("color: #4a64ff;");
	thresholdLayout->addWidget(m_titleLabel);
	thresholdLayout->addWidget(m_thresholdLineEdit);
	thresholdLayout->addWidget(frame);
	addMenuLayout(thresholdLayout);

	connectSignalsToSlots();
}

DigitalInMenu::~DigitalInMenu() {}

void DigitalInMenu::init()
{
	BufferMenu::init();
	// threshold
	(m_attrValues[INPUT_CHNL].contains("threshold") && !m_attrValues[INPUT_CHNL]["threshold"].isEmpty())
		? m_thresholdLineEdit->setText(m_attrValues[INPUT_CHNL]["threshold"].first()),
		m_thresholdLineEdit->setPlaceholderText(m_attrValues[INPUT_CHNL]["threshold"].first())
		: m_thresholdLineEdit->setText("0"),
		m_thresholdLineEdit->setPlaceholderText("0");
}

void DigitalInMenu::connectSignalsToSlots()
{
	connect(m_thresholdLineEdit, &QLineEdit::returnPressed, this, &DigitalInMenu::thresholdChanged);
	connect(this, SIGNAL(mapUpdated()), this, SLOT(onMapUpdated()));
	connect(this, SIGNAL(broadcastThresholdReadBackward(QString)), this, SLOT(onBroadcastThresholdRead(QString)));
	connect(this, &BufferMenu::thresholdControlEnable, this, &DigitalInMenu::onThresholdControlEnable);
}

void DigitalInMenu::thresholdChanged()
{
	QString attrName("threshold");
	if(!m_attrValues.contains(INPUT_CHNL)) {
		qWarning() << "There is no input channel available for this function!";
		return;
	}
	if(m_attrValues[INPUT_CHNL].contains(attrName)) {
		m_attrValues[INPUT_CHNL][attrName].clear();
		m_attrValues[INPUT_CHNL][attrName].push_back(m_thresholdLineEdit->text());
		Q_EMIT attrValuesChanged(attrName, INPUT_CHNL);
	} else {
		qWarning() << attrName.toUpper() + " attribute does not exist!";
	}
}

void DigitalInMenu::onMapUpdated()
{
	m_thresholdLineEdit->blockSignals(true);
	if(m_attrValues[INPUT_CHNL].contains("threshold") && !m_attrValues[INPUT_CHNL]["threshold"].isEmpty()) {
		if(m_attrValues[INPUT_CHNL]["threshold"].first().compare(m_thresholdLineEdit->text()) != 0) {
			Q_EMIT broadcastThresholdReadForward(m_attrValues[INPUT_CHNL]["threshold"].first());
		}
		m_thresholdLineEdit->setText(m_attrValues[INPUT_CHNL]["threshold"].first());
		m_thresholdLineEdit->setPlaceholderText(m_attrValues[INPUT_CHNL]["threshold"].first());
	} else {
		m_thresholdLineEdit->setText(0);
	}
	m_thresholdLineEdit->blockSignals(false);
}

void DigitalInMenu::onBroadcastThresholdRead(QString value)
{
	m_thresholdLineEdit->blockSignals(true);
	if(value.compare(m_thresholdLineEdit->text()) != 0) {
		m_thresholdLineEdit->setText(value);
		m_thresholdLineEdit->setPlaceholderText(value);
		m_attrValues[INPUT_CHNL]["threshold"].first() = value;
	}
	m_thresholdLineEdit->blockSignals(false);
}

void DigitalInMenu::onThresholdControlEnable(bool enabled) { m_thresholdLineEdit->setEnabled(enabled); }
