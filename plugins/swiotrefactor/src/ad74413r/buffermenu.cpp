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
#include "swiot_logging_categories.h"

#include <QLabel>
#include <iio-widgets/iiowidgetfactory.h>
#include <iio-widgets/datastrategy/cmdqchannelattrdatastrategy.h>
#include <iio-widgets/guistrategy/editableguistrategy.h>
#include <guistrategy/comboguistrategy.h>
#include <guistrategy/rangeguistrategy.h>

using namespace scopy::swiotrefactor;
BufferMenu::BufferMenu(QWidget *parent, QString chnlFunction, Connection *conn, QMap<QString, iio_channel *> chnls)
	: QWidget(parent)
	, m_chnlFunction(chnlFunction)
	, m_connection(conn)
	, m_chnls(chnls)
{

	if(m_chnls.contains(INPUT_CHNL)) {
		m_samplingFreq =
			IIOWidgetFactory::buildSingle(IIOWidgetFactory::CMDQAttrData | IIOWidgetFactory::ComboUi,
						      {.connection = const_cast<Connection *>(m_connection),
						       .channel = const_cast<iio_channel *>(m_chnls[INPUT_CHNL]),
						       .data = "sampling_frequency",
						       .iioDataOptions = "sampling_frequency_available"},
						      this);
		addMenuWidget(m_samplingFreq);
		connect(dynamic_cast<CmdQChannelAttrDataStrategy *>(m_samplingFreq->getDataStrategy()),
			&CmdQChannelAttrDataStrategy::sendData, this, &BufferMenu::onSamplingFreqWrite);
		connect(dynamic_cast<ComboAttrUi *>(m_samplingFreq->getUiStrategy()), &ComboAttrUi::emitData, this,
			&BufferMenu::freqChangeStart);
		connect(dynamic_cast<CmdQChannelAttrDataStrategy *>(m_samplingFreq->getDataStrategy()),
			&CmdQChannelAttrDataStrategy::emitStatus, this, &BufferMenu::freqChangeEnd);

		m_samplingFreq->getUiStrategy()->requestData();
	}
}

BufferMenu::~BufferMenu() {}

QString BufferMenu::getInfoMessage()
{
	QString defaultMessage = "\"" + m_chnlFunction +
		"\" configuration generates a buffer capable channel which is directly related to the plot. The "
		"channel attributes can be changed through this menu.";
	return defaultMessage;
}

void BufferMenu::addMenuWidget(QWidget *widget) { m_widgetsList.push_back(widget); }

void BufferMenu::onBroadcastThreshold() {}

void BufferMenu::onDiagSamplingChange(QString samplingFreq) {}

void BufferMenu::onSamplingFreqWrite(QString data, QString dataOptions)
{
	Q_EMIT samplingFrequencyUpdated(data.toInt());
}

void BufferMenu::onRunBtnsPressed(bool en)
{
	dynamic_cast<ComboAttrUi *>(m_samplingFreq->getUiStrategy())->ui()->setEnabled(!en);
}

void BufferMenu::setOffsetScalePair(const std::pair<double, double> &newOffsetScalePair)
{
	m_offsetScalePair = newOffsetScalePair;
}

double BufferMenu::convertFromRaw(double rawValue)
{
	return (rawValue + m_offsetScalePair.first) * m_offsetScalePair.second;
}

QList<QWidget *> BufferMenu::getWidgetsList() { return m_widgetsList; }

CurrentInLoopMenu::CurrentInLoopMenu(QWidget *parent, QString chnlFunction, Connection *conn,
				     QMap<QString, iio_channel *> chnls)
	: BufferMenu(parent, chnlFunction, conn, chnls)
{
	// dac code - output channel
	IIOWidget *dacCode = IIOWidgetFactory::buildSingle(IIOWidgetFactory::CMDQAttrData | IIOWidgetFactory::RangeUi,
							   {.connection = const_cast<Connection *>(m_connection),
							    .channel = const_cast<iio_channel *>(m_chnls[OUTPUT_CHNL]),
							    .data = "raw",
							    .constDataOptions = "[0 1 8191]"},
							   this);
	QLayoutItem *item = dacCode->getUiStrategy()->ui()->layout()->itemAt(0);
	PositionSpinButton *dacSpin = nullptr;
	if(item) {
		dacSpin = dynamic_cast<PositionSpinButton *>(item->widget());
	}
	if(dacSpin) {
		dacSpin->nameLabel()->setText("DAC Code");
		dacSpin->comboBox()->clear();
		dacSpin->comboBox()->addItem("raw value");
		dacSpin->comboBox()->setEnabled(false);
	}
	addMenuWidget(dacCode);

	QWidget *cnvtWidget = new QWidget(this);
	QHBoxLayout *cnvtLayout = new QHBoxLayout(cnvtWidget);
	cnvtWidget->setLayout(cnvtLayout);
	cnvtLayout->setMargin(0);

	m_cnvtLabel = new QLabel(cnvtWidget);
	cnvtLayout->addItem(new QSpacerItem(20, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	cnvtLayout->addWidget(m_cnvtLabel);
	addMenuWidget(cnvtWidget);

	connect(dynamic_cast<RangeAttrUi *>(dacCode->getUiStrategy()), &RangeAttrUi::emitData, this,
		&CurrentInLoopMenu::updateCnvtLabel);
	connect(dynamic_cast<CmdQChannelAttrDataStrategy *>(dacCode->getDataStrategy()),
		&CmdQChannelAttrDataStrategy::sendData, this, &CurrentInLoopMenu::updateCnvtLabel);
}

CurrentInLoopMenu::~CurrentInLoopMenu() {}

QString CurrentInLoopMenu::getInfoMessage()
{
	QString infoMessage = "\"" + m_chnlFunction +
		"\" configuration generates 2 context channels. One of them is an input buffer capable channel "
		"(current_in) which is related to the plot and the other one is an output channel (current_out) whose "
		"attributes can be changed from this menu.";
	return infoMessage;
}

void CurrentInLoopMenu::updateCnvtLabel(QString data)
{
	double convertedData = convertFromRaw(data.toDouble());
	m_cnvtLabel->setText(QString::number(convertedData) + " mA");
}

DigitalInLoopMenu::DigitalInLoopMenu(QWidget *parent, QString chnlFunction, Connection *conn,
				     QMap<QString, iio_channel *> chnls)
	: BufferMenu(parent, chnlFunction, conn, chnls)
{
	// threshold - input channel
	m_threshold = IIOWidgetFactory::buildSingle(IIOWidgetFactory::CMDQAttrData | IIOWidgetFactory::EditableUi,
						    {.connection = const_cast<Connection *>(m_connection),
						     .channel = const_cast<iio_channel *>(m_chnls[INPUT_CHNL]),
						     .data = "threshold",
						     .constDataOptions = "[0 1 16000]"},
						    this);
	addMenuWidget(m_threshold);

	CmdQChannelAttrDataStrategy *dataStrategy =
		dynamic_cast<CmdQChannelAttrDataStrategy *>(m_threshold->getDataStrategy());
	connect(dynamic_cast<EditableGuiStrategy *>(m_threshold->getUiStrategy()), &EditableGuiStrategy::emitData, this,
		&BufferMenu::thresholdChangeStart);
	connect(dataStrategy, &CmdQChannelAttrDataStrategy::emitStatus, this, &DigitalInLoopMenu::onEmitStatus);
	// dac code - output channel
	IIOWidget *dacCode = IIOWidgetFactory::buildSingle(IIOWidgetFactory::CMDQAttrData | IIOWidgetFactory::RangeUi,
							   {.connection = const_cast<Connection *>(m_connection),
							    .channel = const_cast<iio_channel *>(m_chnls[OUTPUT_CHNL]),
							    .data = "raw",
							    .constDataOptions = "[0 1 8191]"},
							   this);
	QLayoutItem *item = dacCode->getUiStrategy()->ui()->layout()->itemAt(0);
	PositionSpinButton *dacSpin = nullptr;
	if(item) {
		dacSpin = dynamic_cast<PositionSpinButton *>(item->widget());
	}
	if(dacSpin) {
		dacSpin->nameLabel()->setText("DAC Code");
		dacSpin->comboBox()->clear();
		dacSpin->comboBox()->addItem("raw value");
		dacSpin->comboBox()->setEnabled(false);
	}
	addMenuWidget(dacCode);

	QWidget *cnvtWidget = new QWidget(this);
	QHBoxLayout *cnvtLayout = new QHBoxLayout(cnvtWidget);
	cnvtWidget->setLayout(cnvtLayout);
	cnvtLayout->setMargin(0);

	m_cnvtLabel = new QLabel(cnvtWidget);
	cnvtLayout->addItem(new QSpacerItem(20, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	cnvtLayout->addWidget(m_cnvtLabel);
	addMenuWidget(cnvtWidget);

	connect(dynamic_cast<RangeAttrUi *>(dacCode->getUiStrategy()), &RangeAttrUi::emitData, this,
		&DigitalInLoopMenu::updateCnvtLabel);
	connect(dynamic_cast<CmdQChannelAttrDataStrategy *>(dacCode->getDataStrategy()),
		&CmdQChannelAttrDataStrategy::sendData, this, &DigitalInLoopMenu::updateCnvtLabel);
}

DigitalInLoopMenu::~DigitalInLoopMenu() {}

QString DigitalInLoopMenu::getInfoMessage()
{
	QString infoMessage = "\"" + m_chnlFunction +
		"\" configuration generates 2 context channels. One of them is an input buffer capable channel "
		"(voltage_in) which is related to the plot and the other one is an output channel (current_out). The "
		"threshold is set for the input channel and the DAC Code for the output channel.";
	return infoMessage;
}

void DigitalInLoopMenu::updateCnvtLabel(QString data)
{
	double convertedData = convertFromRaw(data.toDouble());
	m_cnvtLabel->setText(QString::number(convertedData) + " mA");
}

void DigitalInLoopMenu::onBroadcastThreshold()
{
	CmdQChannelAttrDataStrategy *dataStrategy =
		dynamic_cast<CmdQChannelAttrDataStrategy *>(m_threshold->getDataStrategy());
	dataStrategy->requestData();
}

void DigitalInLoopMenu::onRunBtnsPressed(bool en)
{
	BufferMenu::onRunBtnsPressed(en);
	m_threshold->getUiStrategy()->ui()->setEnabled(!en);
}

void DigitalInLoopMenu::onEmitStatus(int retCode)
{
	if(retCode != 0) {
		qWarning(CAT_SWIOT_AD74413R) << "[" << m_chnlFunction << "] Treshold value cannot be written!";
		return;
	}
	Q_EMIT thresholdChangeEnd();
}

VoltageOutMenu::VoltageOutMenu(QWidget *parent, QString chnlFunction, Connection *conn,
			       QMap<QString, iio_channel *> chnls)
	: BufferMenu(parent, chnlFunction, conn, chnls)
{
	// dac code - output channel
	IIOWidget *dacCode = IIOWidgetFactory::buildSingle(IIOWidgetFactory::CMDQAttrData | IIOWidgetFactory::RangeUi,
							   {.connection = const_cast<Connection *>(m_connection),
							    .channel = const_cast<iio_channel *>(m_chnls[OUTPUT_CHNL]),
							    .data = "raw",
							    .constDataOptions = "[0 1 8191]"},
							   this);
	QLayoutItem *item = dacCode->getUiStrategy()->ui()->layout()->itemAt(0);
	PositionSpinButton *dacSpin = nullptr;
	if(item) {
		dacSpin = dynamic_cast<PositionSpinButton *>(item->widget());
	}
	if(dacSpin) {
		dacSpin->nameLabel()->setText("DAC Code");
		dacSpin->comboBox()->clear();
		dacSpin->comboBox()->addItem("raw value");
		dacSpin->comboBox()->setEnabled(false);
	}
	addMenuWidget(dacCode);

	QWidget *cnvtWidget = new QWidget(this);
	QHBoxLayout *cnvtLayout = new QHBoxLayout(cnvtWidget);
	cnvtWidget->setLayout(cnvtLayout);
	cnvtLayout->setMargin(0);

	m_cnvtLabel = new QLabel(cnvtWidget);
	cnvtLayout->addItem(new QSpacerItem(20, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	cnvtLayout->addWidget(m_cnvtLabel);
	addMenuWidget(cnvtWidget);

	connect(dynamic_cast<RangeAttrUi *>(dacCode->getUiStrategy()), &RangeAttrUi::emitData, this,
		&VoltageOutMenu::updateCnvtLabel);
	connect(dynamic_cast<CmdQChannelAttrDataStrategy *>(dacCode->getDataStrategy()),
		&CmdQChannelAttrDataStrategy::sendData, this, &VoltageOutMenu::updateCnvtLabel);

	// slew - output channel
	IIOWidget *slewOptions =
		IIOWidgetFactory::buildSingle(IIOWidgetFactory::CMDQAttrData | IIOWidgetFactory::ComboUi,
					      {.connection = const_cast<Connection *>(m_connection),
					       .channel = const_cast<iio_channel *>(m_chnls[OUTPUT_CHNL]),
					       .data = "slew_en",
					       .constDataOptions = "0 1"},
					      this);

	addMenuWidget(slewOptions);
	// slew step - output channel
	IIOWidget *slewStep = IIOWidgetFactory::buildSingle(IIOWidgetFactory::CMDQAttrData | IIOWidgetFactory::ComboUi,
							    {.connection = const_cast<Connection *>(m_connection),
							     .channel = const_cast<iio_channel *>(m_chnls[OUTPUT_CHNL]),
							     .data = "slew_step",
							     .iioDataOptions = "slew_step_available"},
							    this);
	addMenuWidget(slewStep);
	// slew rate - output channel
	IIOWidget *slewRate = IIOWidgetFactory::buildSingle(IIOWidgetFactory::CMDQAttrData | IIOWidgetFactory::ComboUi,
							    {.connection = const_cast<Connection *>(m_connection),
							     .channel = const_cast<iio_channel *>(m_chnls[OUTPUT_CHNL]),
							     .data = "slew_rate",
							     .iioDataOptions = "slew_rate_available"},
							    this);
	addMenuWidget(slewRate);
}

VoltageOutMenu::~VoltageOutMenu() {}

QString VoltageOutMenu::getInfoMessage()
{
	QString infoMessage = "\"" + m_chnlFunction +
		"\" configuration generates 2 context channels. One of them is an input buffer capable channel "
		"(current_in) which is related to the plot and the other one is an output channel whose attributes can "
		"be changed from this menu.";
	return infoMessage;
}

void VoltageOutMenu::updateCnvtLabel(QString data)
{
	double convertedData = convertFromRaw(data.toDouble()) * 0.001;
	m_cnvtLabel->setText(QString::number(convertedData) + " V");
}

CurrentOutMenu::CurrentOutMenu(QWidget *parent, QString chnlFunction, Connection *conn,
			       QMap<QString, iio_channel *> chnls)
	: BufferMenu(parent, chnlFunction, conn, chnls)
{
	// dac code - output channel
	IIOWidget *dacCode = IIOWidgetFactory::buildSingle(IIOWidgetFactory::CMDQAttrData | IIOWidgetFactory::RangeUi,
							   {.connection = const_cast<Connection *>(m_connection),
							    .channel = const_cast<iio_channel *>(m_chnls[OUTPUT_CHNL]),
							    .data = "raw",
							    .constDataOptions = "[0 1 8191]"},
							   this);
	QLayoutItem *item = dacCode->getUiStrategy()->ui()->layout()->itemAt(0);
	PositionSpinButton *dacSpin = nullptr;
	if(item) {
		dacSpin = dynamic_cast<PositionSpinButton *>(item->widget());
	}
	if(dacSpin) {
		dacSpin->nameLabel()->setText("DAC Code");
		dacSpin->comboBox()->clear();
		dacSpin->comboBox()->addItem("raw value");
		dacSpin->comboBox()->setEnabled(false);
	}
	addMenuWidget(dacCode);

	QWidget *cnvtWidget = new QWidget(this);
	QHBoxLayout *cnvtLayout = new QHBoxLayout(cnvtWidget);
	cnvtWidget->setLayout(cnvtLayout);
	cnvtLayout->setMargin(0);

	m_cnvtLabel = new QLabel(cnvtWidget);
	cnvtLayout->addItem(new QSpacerItem(20, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	cnvtLayout->addWidget(m_cnvtLabel);
	addMenuWidget(cnvtWidget);

	connect(dynamic_cast<RangeAttrUi *>(dacCode->getUiStrategy()), &RangeAttrUi::emitData, this,
		&CurrentOutMenu::updateCnvtLabel);
	connect(dynamic_cast<CmdQChannelAttrDataStrategy *>(dacCode->getDataStrategy()),
		&CmdQChannelAttrDataStrategy::sendData, this, &CurrentOutMenu::updateCnvtLabel);

	// slew - output channel
	IIOWidget *slewOptions =
		IIOWidgetFactory::buildSingle(IIOWidgetFactory::CMDQAttrData | IIOWidgetFactory::ComboUi,
					      {.connection = const_cast<Connection *>(m_connection),
					       .channel = const_cast<iio_channel *>(m_chnls[OUTPUT_CHNL]),
					       .data = "slew_en",
					       .constDataOptions = "0 1"},
					      this);

	addMenuWidget(slewOptions);
	// slew step - output channel
	IIOWidget *slewStep = IIOWidgetFactory::buildSingle(IIOWidgetFactory::CMDQAttrData | IIOWidgetFactory::ComboUi,
							    {.connection = const_cast<Connection *>(m_connection),
							     .channel = const_cast<iio_channel *>(m_chnls[OUTPUT_CHNL]),
							     .data = "slew_step",
							     .iioDataOptions = "slew_step_available"},
							    this);
	addMenuWidget(slewStep);
	// slew rate - output channel
	IIOWidget *slewRate = IIOWidgetFactory::buildSingle(IIOWidgetFactory::CMDQAttrData | IIOWidgetFactory::ComboUi,
							    {.connection = const_cast<Connection *>(m_connection),
							     .channel = const_cast<iio_channel *>(m_chnls[OUTPUT_CHNL]),
							     .data = "slew_rate",
							     .iioDataOptions = "slew_rate_available"},
							    this);
	addMenuWidget(slewRate);
}

CurrentOutMenu::~CurrentOutMenu() {}

QString CurrentOutMenu::getInfoMessage()
{
	QString infoMessage = "\"" + m_chnlFunction +
		"\" configuration generates 2 context channels. One of them is an input buffer capable channel "
		"(voltage_in) which is related to the plot and the other one is an output channel whose attributes can "
		"be changed from this menu.";
	return infoMessage;
}

void CurrentOutMenu::updateCnvtLabel(QString data)
{
	double convertedData = convertFromRaw(data.toDouble());
	m_cnvtLabel->setText(QString::number(convertedData) + " mA");
}

DiagnosticMenu::DiagnosticMenu(QWidget *parent, QString chnlFunction, Connection *conn,
			       QMap<QString, iio_channel *> chnls)
	: BufferMenu(parent, chnlFunction, conn, chnls)
{
	// diag options - input channel
	IIOWidget *diagOptions =
		IIOWidgetFactory::buildSingle(IIOWidgetFactory::CMDQAttrData | IIOWidgetFactory::ComboUi,
					      {.connection = const_cast<Connection *>(m_connection),
					       .channel = const_cast<iio_channel *>(m_chnls[INPUT_CHNL]),
					       .data = "diag_function",
					       .iioDataOptions = "diag_function_available"},
					      this);
	addMenuWidget(diagOptions);

	connect(dynamic_cast<CmdQChannelAttrDataStrategy *>(diagOptions->getDataStrategy()),
		&CmdQChannelAttrDataStrategy::sendData, this,
		[=, this](QString data, QString dataOptions) { Q_EMIT diagnosticFunctionUpdated(); });
}

DiagnosticMenu::~DiagnosticMenu() {}

// TBD - it is possible that there is a better option
void DiagnosticMenu::onDiagSamplingChange(QString samplingFreq)
{
	QWidget *w = m_samplingFreq->getUiStrategy()->ui()->layout()->itemAt(0)->widget();
	if(w) {
		dynamic_cast<MenuCombo *>(w)->combo()->setCurrentText(samplingFreq);
	}
}

void DiagnosticMenu::onSamplingFreqWrite(QString data, QString dataOptions) { Q_EMIT diagSamplingFreqChange(data); }

WithoutAdvSettings::WithoutAdvSettings(QWidget *parent, QString chnlFunction, Connection *conn,
				       QMap<QString, iio_channel *> chnls)
	: BufferMenu(parent, chnlFunction, conn, chnls)
{
	QLabel *msgLabel = new QLabel("No advanced settings available", this);
	StyleHelper::MenuSmallLabel(msgLabel);
	addMenuWidget(msgLabel);
}

WithoutAdvSettings::~WithoutAdvSettings() {}

DigitalInMenu::DigitalInMenu(QWidget *parent, QString chnlFunction, Connection *conn,
			     QMap<QString, iio_channel *> chnls)
	: BufferMenu(parent, chnlFunction, conn, chnls)
{
	// threshold - input channel
	m_threshold = IIOWidgetFactory::buildSingle(IIOWidgetFactory::CMDQAttrData | IIOWidgetFactory::EditableUi,
						    {.connection = const_cast<Connection *>(m_connection),
						     .channel = const_cast<iio_channel *>(m_chnls[INPUT_CHNL]),
						     .data = "threshold",
						     .constDataOptions = "[0 1 16000]"},
						    this);
	addMenuWidget(m_threshold);
	CmdQChannelAttrDataStrategy *dataStrategy =
		dynamic_cast<CmdQChannelAttrDataStrategy *>(m_threshold->getDataStrategy());
	connect(dynamic_cast<EditableGuiStrategy *>(m_threshold->getUiStrategy()), &EditableGuiStrategy::emitData, this,
		&BufferMenu::thresholdChangeStart);
	connect(dataStrategy, &CmdQChannelAttrDataStrategy::emitStatus, this, &DigitalInMenu::onEmitStatus);
}

DigitalInMenu::~DigitalInMenu() {}

void DigitalInMenu::onBroadcastThreshold()
{
	CmdQChannelAttrDataStrategy *dataStrategy =
		dynamic_cast<CmdQChannelAttrDataStrategy *>(m_threshold->getDataStrategy());
	dataStrategy->requestData();
}

void DigitalInMenu::onRunBtnsPressed(bool en)
{
	BufferMenu::onRunBtnsPressed(en);
	m_threshold->getUiStrategy()->ui()->setEnabled(!en);
}

void DigitalInMenu::onEmitStatus(int retCode)
{
	if(retCode != 0) {
		qWarning(CAT_SWIOT_AD74413R) << "[" << m_chnlFunction << "] Treshold value cannot be written!";
		return;
	}
	Q_EMIT thresholdChangeEnd();
}
