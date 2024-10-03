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
#include <titlespinbox.h>
#include <iio-widgets/iiowidgetbuilder.h>
#include <iio-widgets/datastrategy/cmdqchannelattrdatastrategy.h>
#include <iio-widgets/guistrategy/editableguistrategy.h>
#include <guistrategy/comboguistrategy.h>
#include <guistrategy/rangeguistrategy.h>

using namespace scopy::swiot;
BufferMenu::BufferMenu(QWidget *parent, QString chnlFunction, Connection *conn, QMap<QString, iio_channel *> chnls)
	: QWidget(parent)
	, m_chnlFunction(chnlFunction)
	, m_connection(conn)
	, m_chnls(chnls)
{
	if(m_chnls.contains(INPUT_CHNL)) {
		m_samplingFreq = IIOWidgetBuilder(this)
					 .connection(const_cast<Connection *>(m_connection))
					 .channel(const_cast<iio_channel *>(m_chnls[INPUT_CHNL]))
					 .attribute("sampling_frequency")
					 .optionsAttribute("sampling_frequency_available")
					 .uiStrategy(IIOWidgetBuilder::UIS::ComboUi)
					 .buildSingle();
		addMenuWidget(m_samplingFreq);
		connect(m_samplingFreq, &IIOWidget::displayedNewData, this,
			[this](QString data, QString optionalData) { Q_EMIT samplingFrequencyUpdated(data.toInt()); });
		connect(m_samplingFreq, &IIOWidget::emitData, this, &BufferMenu::freqChangeStart);
		connect(m_samplingFreq, &IIOWidget::emitStatus, this, &BufferMenu::freqChangeEnd);
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

void BufferMenu::onDiagSamplingChange() {}

void BufferMenu::onRunBtnsPressed(bool en) { m_samplingFreq->setUIEnabled(!en); }

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
	IIOWidget *dacCode = IIOWidgetBuilder(this)
				     .connection(const_cast<Connection *>(m_connection))
				     .channel(const_cast<iio_channel *>(m_chnls[OUTPUT_CHNL]))
				     .attribute("raw")
				     .optionsValues("[0 1 8191]")
				     .buildSingle();
	dacCode->changeTitle("DAC_CODE");
	addMenuWidget(dacCode);

	QWidget *cnvtWidget = new QWidget(this);
	QHBoxLayout *cnvtLayout = new QHBoxLayout(cnvtWidget);
	cnvtWidget->setLayout(cnvtLayout);
	cnvtLayout->setMargin(0);

	m_cnvtLabel = new QLabel(cnvtWidget);
	cnvtLayout->addItem(new QSpacerItem(20, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	cnvtLayout->addWidget(m_cnvtLabel);
	addMenuWidget(cnvtWidget);

	connect(dacCode, &IIOWidget::emitData, this, &CurrentInLoopMenu::updateCnvtLabel);
	connect(dacCode, &IIOWidget::sendData, this, &CurrentInLoopMenu::updateCnvtLabel);
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
	m_threshold = IIOWidgetBuilder(this)
			      .connection(const_cast<Connection *>(m_connection))
			      .channel(const_cast<iio_channel *>(m_chnls[INPUT_CHNL]))
			      .attribute("threshold")
			      .optionsValues("[0 1 16000]")
			      .buildSingle();
	addMenuWidget(m_threshold);

	connect(m_threshold, &IIOWidget::emitData, this, &BufferMenu::thresholdChangeStart);
	connect(m_threshold, &IIOWidget::emitStatus, this, &DigitalInLoopMenu::onEmitStatus);
	// dac code - output channel
	IIOWidget *dacCode = IIOWidgetBuilder(this)
				     .connection(const_cast<Connection *>(m_connection))
				     .channel(const_cast<iio_channel *>(m_chnls[OUTPUT_CHNL]))
				     .attribute("raw")
				     .optionsValues("[0 1 8191]")
				     .buildSingle();
	dacCode->changeTitle("DAC_CODE");
	addMenuWidget(dacCode);

	QWidget *cnvtWidget = new QWidget(this);
	QHBoxLayout *cnvtLayout = new QHBoxLayout(cnvtWidget);
	cnvtWidget->setLayout(cnvtLayout);
	cnvtLayout->setMargin(0);

	m_cnvtLabel = new QLabel(cnvtWidget);
	cnvtLayout->addItem(new QSpacerItem(20, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	cnvtLayout->addWidget(m_cnvtLabel);
	addMenuWidget(cnvtWidget);

	connect(dacCode, &IIOWidget::emitData, this, &DigitalInLoopMenu::updateCnvtLabel);
	connect(dacCode, &IIOWidget::sendData, this, &DigitalInLoopMenu::updateCnvtLabel);
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

void DigitalInLoopMenu::onBroadcastThreshold() { m_threshold->readAsync(); }

void DigitalInLoopMenu::onRunBtnsPressed(bool en)
{
	BufferMenu::onRunBtnsPressed(en);
	m_threshold->setUIEnabled(!en);
}

void DigitalInLoopMenu::onEmitStatus(QDateTime timestamp, QString oldData, QString newData, int retCode, bool readOp)
{
	if(readOp) {
		return;
	}
	if(retCode < 0) {
		qWarning(CAT_SWIOT_AD74413R)
			<< "[" << m_chnlFunction << "]" << newData << "treshold value cannot be written!";
	}
	Q_EMIT thresholdChangeEnd();
}

VoltageOutMenu::VoltageOutMenu(QWidget *parent, QString chnlFunction, Connection *conn,
			       QMap<QString, iio_channel *> chnls)
	: BufferMenu(parent, chnlFunction, conn, chnls)
{
	// dac code - output channel
	IIOWidget *dacCode = IIOWidgetBuilder(this)
				     .connection(const_cast<Connection *>(m_connection))
				     .channel(const_cast<iio_channel *>(m_chnls[OUTPUT_CHNL]))
				     .attribute("raw")
				     .optionsValues("[0 1 8191]")
				     .buildSingle();
	dacCode->changeTitle("DAC_CODE");
	addMenuWidget(dacCode);

	QWidget *cnvtWidget = new QWidget(this);
	QHBoxLayout *cnvtLayout = new QHBoxLayout(cnvtWidget);
	cnvtWidget->setLayout(cnvtLayout);
	cnvtLayout->setMargin(0);

	m_cnvtLabel = new QLabel(cnvtWidget);
	cnvtLayout->addItem(new QSpacerItem(20, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	cnvtLayout->addWidget(m_cnvtLabel);
	addMenuWidget(cnvtWidget);

	connect(dacCode, &IIOWidget::emitData, this, &VoltageOutMenu::updateCnvtLabel);
	connect(dacCode, &IIOWidget::sendData, this, &VoltageOutMenu::updateCnvtLabel);

	// slew - output channel
	IIOWidget *slewOptions = IIOWidgetBuilder(this)
					 .connection(const_cast<Connection *>(m_connection))
					 .channel(const_cast<iio_channel *>(m_chnls[OUTPUT_CHNL]))
					 .attribute("slew_en")
					 .optionsValues("0 1")
					 .buildSingle();
	addMenuWidget(slewOptions);

	// slew step - output channel
	IIOWidget *slewStep = IIOWidgetBuilder(this)
				      .connection(const_cast<Connection *>(m_connection))
				      .channel(const_cast<iio_channel *>(m_chnls[OUTPUT_CHNL]))
				      .attribute("slew_step")
				      .optionsAttribute("slew_step_available")
				      .uiStrategy(IIOWidgetBuilder::UIS::ComboUi)
				      .buildSingle();
	addMenuWidget(slewStep);

	// slew rate - output channel
	IIOWidget *slewRate = IIOWidgetBuilder(this)
				      .connection(const_cast<Connection *>(m_connection))
				      .channel(const_cast<iio_channel *>(m_chnls[OUTPUT_CHNL]))
				      .attribute("slew_rate")
				      .optionsAttribute("slew_rate_available")
				      .uiStrategy(IIOWidgetBuilder::UIS::ComboUi)
				      .buildSingle();
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
	IIOWidget *dacCode = IIOWidgetBuilder(this)
				     .connection(const_cast<Connection *>(m_connection))
				     .channel(const_cast<iio_channel *>(m_chnls[OUTPUT_CHNL]))
				     .attribute("raw")
				     .optionsValues("[0 1 8191]")
				     .buildSingle();
	dacCode->changeTitle("DAC_CODE");
	addMenuWidget(dacCode);

	QWidget *cnvtWidget = new QWidget(this);
	QHBoxLayout *cnvtLayout = new QHBoxLayout(cnvtWidget);
	cnvtWidget->setLayout(cnvtLayout);
	cnvtLayout->setMargin(0);

	m_cnvtLabel = new QLabel(cnvtWidget);
	cnvtLayout->addItem(new QSpacerItem(20, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	cnvtLayout->addWidget(m_cnvtLabel);
	addMenuWidget(cnvtWidget);

	connect(dacCode, &IIOWidget::emitData, this, &CurrentOutMenu::updateCnvtLabel);
	connect(dacCode, &IIOWidget::sendData, this, &CurrentOutMenu::updateCnvtLabel);

	// slew - output channel
	IIOWidget *slewOptions = IIOWidgetBuilder(this)
					 .connection(const_cast<Connection *>(m_connection))
					 .channel(const_cast<iio_channel *>(m_chnls[OUTPUT_CHNL]))
					 .attribute("slew_en")
					 .optionsValues("0 1")
					 .buildSingle();
	addMenuWidget(slewOptions);

	// slew step - output channel
	IIOWidget *slewStep = IIOWidgetBuilder(this)
				      .connection(const_cast<Connection *>(m_connection))
				      .channel(const_cast<iio_channel *>(m_chnls[OUTPUT_CHNL]))
				      .attribute("slew_step")
				      .optionsAttribute("slew_step_available")
				      .uiStrategy(IIOWidgetBuilder::UIS::ComboUi)
				      .buildSingle();
	addMenuWidget(slewStep);

	// slew rate - output channel
	IIOWidget *slewRate = IIOWidgetBuilder(this)
				      .connection(const_cast<Connection *>(m_connection))
				      .channel(const_cast<iio_channel *>(m_chnls[OUTPUT_CHNL]))
				      .attribute("slew_rate")
				      .optionsAttribute("slew_rate_available")
				      .uiStrategy(IIOWidgetBuilder::UIS::ComboUi)
				      .buildSingle();
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
	IIOWidget *diagOptions = IIOWidgetBuilder(this)
					 .connection(const_cast<Connection *>(m_connection))
					 .channel(const_cast<iio_channel *>(m_chnls[INPUT_CHNL]))
					 .attribute("diag_function")
					 .optionsAttribute("diag_function_available")
					 .uiStrategy(IIOWidgetBuilder::UIS::ComboUi)
					 .buildSingle();
	addMenuWidget(diagOptions);

	connect(diagOptions, &IIOWidget::displayedNewData, this,
		[=, this](QString data, QString dataOptions) { Q_EMIT diagnosticFunctionUpdated(); });
	connect(m_samplingFreq, &IIOWidget::emitStatus, this, &DiagnosticMenu::onSamplingFreqWrite);
}

DiagnosticMenu::~DiagnosticMenu() {}

void DiagnosticMenu::onDiagSamplingChange() { m_samplingFreq->readAsync(); }

void DiagnosticMenu::onSamplingFreqWrite(QDateTime timestamp, QString oldData, QString newData, int retCode,
					 bool readOp)
{
	if(retCode < 0 || readOp) {
		return;
	}
	Q_EMIT diagSamplingFreqChange();
}

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
	m_threshold = IIOWidgetBuilder(this)
			      .connection(const_cast<Connection *>(m_connection))
			      .channel(const_cast<iio_channel *>(m_chnls[INPUT_CHNL]))
			      .attribute("threshold")
			      .optionsValues("[0 1 16000]")
			      .buildSingle();
	addMenuWidget(m_threshold);
	connect(m_threshold, &IIOWidget::emitData, this, &BufferMenu::thresholdChangeStart);
	connect(m_threshold, &IIOWidget::emitStatus, this, &DigitalInMenu::onEmitStatus);
}

DigitalInMenu::~DigitalInMenu() {}

void DigitalInMenu::onBroadcastThreshold() { m_threshold->readAsync(); }

void DigitalInMenu::onRunBtnsPressed(bool en)
{
	BufferMenu::onRunBtnsPressed(en);
	m_threshold->setUIEnabled(!en);
}

void DigitalInMenu::onEmitStatus(QDateTime timestamp, QString oldData, QString newData, int retCode, bool readOp)
{
	if(readOp) {
		return;
	}
	if(retCode < 0) {
		qWarning(CAT_SWIOT_AD74413R) << "[" << m_chnlFunction << "] Treshold value cannot be written!";
	}
	Q_EMIT thresholdChangeEnd();
}

#include "moc_buffermenu.cpp"
