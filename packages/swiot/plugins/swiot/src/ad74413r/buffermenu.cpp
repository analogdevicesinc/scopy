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
#include <style.h>
#include <titlespinbox.h>
#include <component/channel.h>
#include <component/attribute.h>
#include <iio-widgets/iiowidgetbuilder.h>
#include <iio-widgets/datastrategy/componentattrdatastrategy.h>
#include <iio-widgets/guistrategy/editableguistrategy.h>
#include <guistrategy/comboguistrategy.h>
#include <guistrategy/rangeguistrategy.h>

using namespace scopy::swiot;
using namespace scopy;

component::Attribute *BufferMenu::getAttr(const QString &chnlKey, const QString &attrName)
{
	if(!m_chnls.contains(chnlKey) || !m_chnls[chnlKey]) {
		return nullptr;
	}
	return m_chnls[chnlKey]->findChild<component::Attribute *>(attrName, Qt::FindDirectChildrenOnly);
}

BufferMenu::BufferMenu(QWidget *parent, QString chnlFunction, QMap<QString, component::Channel *> chnls,
		       IIOWidgetGroup *widgetGroup)
	: QWidget(parent)
	, m_chnlFunction(chnlFunction)
	, m_chnls(chnls)
	, m_widgetGroup(widgetGroup)
{
	component::Attribute *samplingFreqAttr = getAttr(INPUT_CHNL, "sampling_frequency");
	if(samplingFreqAttr) {
		m_samplingFreq = IIOWidgetBuilder(this)
					 .group(m_widgetGroup)
					 .attribute(samplingFreqAttr)
					 .uiStrategy(IIOWidgetBuilder::UIS::ComboUi)
					 .buildSingle();
		addMenuWidget(m_samplingFreq);
		connect(dynamic_cast<ComboAttrUi *>(m_samplingFreq->getUiStrategy()), &ComboAttrUi::displayedNewData,
			this,
			[this](QString data, QString optionalData) { Q_EMIT samplingFrequencyUpdated(data.toInt()); });
		connect(dynamic_cast<ComboAttrUi *>(m_samplingFreq->getUiStrategy()), &ComboAttrUi::emitData, this,
			&BufferMenu::freqChangeStart);
		connect(dynamic_cast<ComponentAttrDataStrategy *>(m_samplingFreq->getDataStrategy()),
			&ComponentAttrDataStrategy::emitStatus, this, &BufferMenu::freqChangeEnd);
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

CurrentInLoopMenu::CurrentInLoopMenu(QWidget *parent, QString chnlFunction, QMap<QString, component::Channel *> chnls,
				     IIOWidgetGroup *widgetGroup)
	: BufferMenu(parent, chnlFunction, chnls, widgetGroup)
{
	// dac code - output channel
	IIOWidget *dacCode = IIOWidgetBuilder(this)
				     .group(m_widgetGroup)
				     .attribute(getAttr(OUTPUT_CHNL, "raw"))
				     .optionsValues("[0 1 8191]")
				     .uiStrategy(IIOWidgetBuilder::UIS::RangeUi)
				     .buildSingle();
	QLayoutItem *item = dacCode->getUiStrategy()->ui()->layout()->itemAt(0);
	TitleSpinBox *dacSpin = nullptr;
	if(item) {
		dacSpin = dynamic_cast<TitleSpinBox *>(item->widget());
	}
	if(dacSpin) {
		dacSpin->setTitle("DAC_CODE");
	}
	addMenuWidget(dacCode);

	QWidget *cnvtWidget = new QWidget(this);
	QHBoxLayout *cnvtLayout = new QHBoxLayout(cnvtWidget);
	cnvtWidget->setLayout(cnvtLayout);
	cnvtLayout->setContentsMargins(0, 0, 0, 0);

	m_cnvtLabel = new QLabel(cnvtWidget);
	cnvtLayout->addItem(new QSpacerItem(20, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	cnvtLayout->addWidget(m_cnvtLabel);
	addMenuWidget(cnvtWidget);

	connect(dynamic_cast<RangeAttrUi *>(dacCode->getUiStrategy()), &RangeAttrUi::emitData, this,
		&CurrentInLoopMenu::updateCnvtLabel);
	connect(dynamic_cast<ComponentAttrDataStrategy *>(dacCode->getDataStrategy()),
		&ComponentAttrDataStrategy::sendData, this, &CurrentInLoopMenu::updateCnvtLabel);
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

DigitalInLoopMenu::DigitalInLoopMenu(QWidget *parent, QString chnlFunction, QMap<QString, component::Channel *> chnls,
				     IIOWidgetGroup *widgetGroup)
	: BufferMenu(parent, chnlFunction, chnls, widgetGroup)
{
	// threshold - input channel
	m_threshold = IIOWidgetBuilder(this)
			      .group(m_widgetGroup)
			      .attribute(getAttr(INPUT_CHNL, "threshold"))
			      .optionsValues("[0 1 16000]")
			      .uiStrategy(IIOWidgetBuilder::UIS::RangeUi)
			      .buildSingle();
	addMenuWidget(m_threshold);

	ComponentAttrDataStrategy *dataStrategy =
		dynamic_cast<ComponentAttrDataStrategy *>(m_threshold->getDataStrategy());
	connect(dynamic_cast<EditableGuiStrategy *>(m_threshold->getUiStrategy()), &EditableGuiStrategy::emitData, this,
		&BufferMenu::thresholdChangeStart);
	connect(dataStrategy, &ComponentAttrDataStrategy::emitStatus, this, &DigitalInLoopMenu::onEmitStatus);
	// dac code - output channel
	IIOWidget *dacCode = IIOWidgetBuilder(this)
				     .group(m_widgetGroup)
				     .attribute(getAttr(OUTPUT_CHNL, "raw"))
				     .optionsValues("[0 1 8191]")
				     .uiStrategy(IIOWidgetBuilder::UIS::RangeUi)
				     .buildSingle();
	QLayoutItem *item = dacCode->getUiStrategy()->ui()->layout()->itemAt(0);
	TitleSpinBox *dacSpin = nullptr;
	if(item) {
		dacSpin = dynamic_cast<TitleSpinBox *>(item->widget());
	}
	if(dacSpin) {
		dacSpin->setTitle("DAC_CODE");
	}
	addMenuWidget(dacCode);

	QWidget *cnvtWidget = new QWidget(this);
	QHBoxLayout *cnvtLayout = new QHBoxLayout(cnvtWidget);
	cnvtWidget->setLayout(cnvtLayout);
	cnvtLayout->setContentsMargins(0, 0, 0, 0);

	m_cnvtLabel = new QLabel(cnvtWidget);
	cnvtLayout->addItem(new QSpacerItem(20, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	cnvtLayout->addWidget(m_cnvtLabel);
	addMenuWidget(cnvtWidget);

	connect(dynamic_cast<RangeAttrUi *>(dacCode->getUiStrategy()), &RangeAttrUi::emitData, this,
		&DigitalInLoopMenu::updateCnvtLabel);
	connect(dynamic_cast<ComponentAttrDataStrategy *>(dacCode->getDataStrategy()),
		&ComponentAttrDataStrategy::sendData, this, &DigitalInLoopMenu::updateCnvtLabel);
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
	ComponentAttrDataStrategy *dataStrategy =
		dynamic_cast<ComponentAttrDataStrategy *>(m_threshold->getDataStrategy());
	dataStrategy->readAsync();
}

void DigitalInLoopMenu::onRunBtnsPressed(bool en)
{
	BufferMenu::onRunBtnsPressed(en);
	m_threshold->getUiStrategy()->ui()->setEnabled(!en);
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

VoltageOutMenu::VoltageOutMenu(QWidget *parent, QString chnlFunction, QMap<QString, component::Channel *> chnls,
			       IIOWidgetGroup *widgetGroup)
	: BufferMenu(parent, chnlFunction, chnls, widgetGroup)
{
	// dac code - output channel
	IIOWidget *dacCode = IIOWidgetBuilder(this)
				     .group(m_widgetGroup)
				     .attribute(getAttr(OUTPUT_CHNL, "raw"))
				     .optionsValues("[0 1 8191]")
				     .uiStrategy(IIOWidgetBuilder::UIS::RangeUi)
				     .buildSingle();
	QLayoutItem *item = dacCode->getUiStrategy()->ui()->layout()->itemAt(0);
	TitleSpinBox *dacSpin = nullptr;
	if(item) {
		dacSpin = dynamic_cast<TitleSpinBox *>(item->widget());
	}
	if(dacSpin) {
		dacSpin->setTitle("DAC_CODE");
	}
	addMenuWidget(dacCode);

	QWidget *cnvtWidget = new QWidget(this);
	QHBoxLayout *cnvtLayout = new QHBoxLayout(cnvtWidget);
	cnvtWidget->setLayout(cnvtLayout);
	cnvtLayout->setContentsMargins(0, 0, 0, 0);

	m_cnvtLabel = new QLabel(cnvtWidget);
	cnvtLayout->addItem(new QSpacerItem(20, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	cnvtLayout->addWidget(m_cnvtLabel);
	addMenuWidget(cnvtWidget);

	connect(dynamic_cast<RangeAttrUi *>(dacCode->getUiStrategy()), &RangeAttrUi::emitData, this,
		&VoltageOutMenu::updateCnvtLabel);
	connect(dynamic_cast<ComponentAttrDataStrategy *>(dacCode->getDataStrategy()),
		&ComponentAttrDataStrategy::sendData, this, &VoltageOutMenu::updateCnvtLabel);

	// slew - output channel
	IIOWidget *slewOptions = IIOWidgetBuilder(this)
					 .group(m_widgetGroup)
					 .attribute(getAttr(OUTPUT_CHNL, "slew_en"))
					 .optionsValues("0 1")
					 .uiStrategy(IIOWidgetBuilder::UIS::ComboUi)
					 .buildSingle();
	addMenuWidget(slewOptions);

	// slew step - output channel
	IIOWidget *slewStep = IIOWidgetBuilder(this)
				      .group(m_widgetGroup)
				      .attribute(getAttr(OUTPUT_CHNL, "slew_step"))
				      .uiStrategy(IIOWidgetBuilder::UIS::ComboUi)
				      .buildSingle();
	addMenuWidget(slewStep);

	// slew rate - output channel
	IIOWidget *slewRate = IIOWidgetBuilder(this)
				      .group(m_widgetGroup)
				      .attribute(getAttr(OUTPUT_CHNL, "slew_rate"))
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

CurrentOutMenu::CurrentOutMenu(QWidget *parent, QString chnlFunction, QMap<QString, component::Channel *> chnls,
			       IIOWidgetGroup *widgetGroup)
	: BufferMenu(parent, chnlFunction, chnls, widgetGroup)
{
	// dac code - output channel
	IIOWidget *dacCode = IIOWidgetBuilder(this)
				     .group(m_widgetGroup)
				     .attribute(getAttr(OUTPUT_CHNL, "raw"))
				     .optionsValues("[0 1 8191]")
				     .uiStrategy(IIOWidgetBuilder::UIS::RangeUi)
				     .buildSingle();
	QLayoutItem *item = dacCode->getUiStrategy()->ui()->layout()->itemAt(0);
	TitleSpinBox *dacSpin = nullptr;
	if(item) {
		dacSpin = dynamic_cast<TitleSpinBox *>(item->widget());
	}
	if(dacSpin) {
		dacSpin->setTitle("DAC_CODE");
	}
	addMenuWidget(dacCode);

	QWidget *cnvtWidget = new QWidget(this);
	QHBoxLayout *cnvtLayout = new QHBoxLayout(cnvtWidget);
	cnvtWidget->setLayout(cnvtLayout);
	cnvtLayout->setContentsMargins(0, 0, 0, 0);

	m_cnvtLabel = new QLabel(cnvtWidget);
	cnvtLayout->addItem(new QSpacerItem(20, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	cnvtLayout->addWidget(m_cnvtLabel);
	addMenuWidget(cnvtWidget);

	connect(dynamic_cast<RangeAttrUi *>(dacCode->getUiStrategy()), &RangeAttrUi::emitData, this,
		&CurrentOutMenu::updateCnvtLabel);
	connect(dynamic_cast<ComponentAttrDataStrategy *>(dacCode->getDataStrategy()),
		&ComponentAttrDataStrategy::sendData, this, &CurrentOutMenu::updateCnvtLabel);

	// slew - output channel
	IIOWidget *slewOptions = IIOWidgetBuilder(this)
					 .group(m_widgetGroup)
					 .attribute(getAttr(OUTPUT_CHNL, "slew_en"))
					 .optionsValues("0 1")
					 .uiStrategy(IIOWidgetBuilder::UIS::ComboUi)
					 .buildSingle();
	addMenuWidget(slewOptions);

	// slew step - output channel
	IIOWidget *slewStep = IIOWidgetBuilder(this)
				      .group(m_widgetGroup)
				      .attribute(getAttr(OUTPUT_CHNL, "slew_step"))
				      .uiStrategy(IIOWidgetBuilder::UIS::ComboUi)
				      .buildSingle();
	addMenuWidget(slewStep);

	// slew rate - output channel
	IIOWidget *slewRate = IIOWidgetBuilder(this)
				      .group(m_widgetGroup)
				      .attribute(getAttr(OUTPUT_CHNL, "slew_rate"))
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

DiagnosticMenu::DiagnosticMenu(QWidget *parent, QString chnlFunction, QMap<QString, component::Channel *> chnls,
			       IIOWidgetGroup *widgetGroup)
	: BufferMenu(parent, chnlFunction, chnls, widgetGroup)
{
	// diag options - input channel
	IIOWidget *diagOptions = IIOWidgetBuilder(this)
					 .group(m_widgetGroup)
					 .attribute(getAttr(INPUT_CHNL, "diag_function"))
					 .uiStrategy(IIOWidgetBuilder::UIS::ComboUi)
					 .buildSingle();
	addMenuWidget(diagOptions);

	connect(dynamic_cast<ComboAttrUi *>(diagOptions->getUiStrategy()), &ComboAttrUi::displayedNewData, this,
		[=, this](QString data, QString dataOptions) { Q_EMIT diagnosticFunctionUpdated(); });
	connect(dynamic_cast<ComponentAttrDataStrategy *>(m_samplingFreq->getDataStrategy()),
		&ComponentAttrDataStrategy::emitStatus, this, &DiagnosticMenu::onSamplingFreqWrite);
}

DiagnosticMenu::~DiagnosticMenu() {}

void DiagnosticMenu::onDiagSamplingChange() { m_samplingFreq->getDataStrategy()->readAsync(); }

void DiagnosticMenu::onSamplingFreqWrite(QDateTime timestamp, QString oldData, QString newData, int retCode,
					 bool readOp)
{
	if(retCode < 0 || readOp) {
		return;
	}
	Q_EMIT diagSamplingFreqChange();
}

WithoutAdvSettings::WithoutAdvSettings(QWidget *parent, QString chnlFunction, QMap<QString, component::Channel *> chnls,
				       IIOWidgetGroup *widgetGroup)
	: BufferMenu(parent, chnlFunction, chnls, widgetGroup)
{
	QLabel *msgLabel = new QLabel("No advanced settings available", this);
	Style::setStyle(msgLabel, style::properties::label::menuSmall);
	addMenuWidget(msgLabel);
}

WithoutAdvSettings::~WithoutAdvSettings() {}

DigitalInMenu::DigitalInMenu(QWidget *parent, QString chnlFunction, QMap<QString, component::Channel *> chnls,
			     IIOWidgetGroup *widgetGroup)
	: BufferMenu(parent, chnlFunction, chnls, widgetGroup)
{
	// threshold - input channel
	m_threshold = IIOWidgetBuilder(this)
			      .group(m_widgetGroup)
			      .attribute(getAttr(INPUT_CHNL, "threshold"))
			      .optionsValues("[0 1 16000]")
			      .uiStrategy(IIOWidgetBuilder::UIS::RangeUi)
			      .buildSingle();
	addMenuWidget(m_threshold);
	ComponentAttrDataStrategy *dataStrategy =
		dynamic_cast<ComponentAttrDataStrategy *>(m_threshold->getDataStrategy());
	connect(dynamic_cast<EditableGuiStrategy *>(m_threshold->getUiStrategy()), &EditableGuiStrategy::emitData, this,
		&BufferMenu::thresholdChangeStart);
	connect(dataStrategy, &ComponentAttrDataStrategy::emitStatus, this, &DigitalInMenu::onEmitStatus);
}

DigitalInMenu::~DigitalInMenu() {}

void DigitalInMenu::onBroadcastThreshold()
{
	ComponentAttrDataStrategy *dataStrategy =
		dynamic_cast<ComponentAttrDataStrategy *>(m_threshold->getDataStrategy());
	dataStrategy->readAsync();
}

void DigitalInMenu::onRunBtnsPressed(bool en)
{
	BufferMenu::onRunBtnsPressed(en);
	m_threshold->getUiStrategy()->ui()->setEnabled(!en);
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
