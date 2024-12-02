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


#include "faultsdevice.h"
#include <utility>
#include "faultsgroup.h"
#include "src/swiot_logging_categories.h"
#include <iioutil/iiocommand/iiochannelattributeread.h>
#include <iioutil/iiocommand/iiodeviceattributeread.h>
#include <iioutil/commandqueueprovider.h>

#define FAULT_CHANNEL_NAME "voltage"
#define SWIOT_NB_CHANNELS 4

using namespace scopy::swiot;

FaultsDevice::FaultsDevice(const QString& name, QString path, struct iio_device* device,
			   struct iio_device* swiot, struct iio_context *context, QWidget *parent)
	: ui(new Ui::FaultsDevice),
	  QWidget(parent),
	  m_faults_explanation(new QWidget(this)),
	  m_subsectionSeparator(new scopy::gui::SubsectionSeparator("Faults Explanation", true, this)),
	  m_name(name.toUpper()),
	  m_path(std::move(path)),
	  m_device(device),
	  m_swiot(swiot),
	  m_context(context),
	  m_readFaultCommand(nullptr),
	  m_faultsChannel(nullptr),
	  m_cmdQueue(nullptr)
{
	ui->setupUi(this);
	ui->reset_button->setProperty("blue_button", QVariant(true));
	ui->clear_selection_button->setProperty("blue_button", QVariant(true));
	m_cmdQueue = CommandQueueProvider::GetInstance()->open(m_context);
	initSpecialFaults();
	m_faultsGroup = new FaultsGroup(name, m_path, this);
	connect(this, &FaultsDevice::specialFaultsUpdated, m_faultsGroup, &FaultsGroup::specialFaultsUpdated);
	connect(m_faultsGroup, &FaultsGroup::specialFaultExplanationChanged, this, &FaultsDevice::updateExplanation);


	if (m_device == nullptr) {
		qCritical(CAT_SWIOT_FAULTS) << "No device was found";
	}
	m_faultsChannel = iio_device_find_channel(m_device, FAULT_CHANNEL_NAME, false);
	if (m_faultsChannel == nullptr) {
		qCritical(CAT_SWIOT_FAULTS) << "Channel" << FAULT_CHANNEL_NAME << "was not found";
	}

	this->connectSignalsAndSlots();
	this->initFaultExplanations();

	this->ui->lineEdit_numeric->setPlaceholderText("0x0");
	this->ui->lineEdit_numeric->setFocusPolicy(Qt::NoFocus);

	setDynamicProperty(m_subsectionSeparator->getButton(), "subsection_arrow_button", true);
	m_subsectionSeparator->setContent(m_faults_explanation);

	this->ui->label_name->setText(m_name);
	this->ui->lineEdit_numeric->setReadOnly(true);
	this->ui->faults_layout->addWidget(this->m_faultsGroup);
	this->ui->faults_explanation->layout()->addWidget(m_subsectionSeparator);
	m_faults_explanation->ensurePolished();
}

FaultsDevice::~FaultsDevice() {
	if (m_cmdQueue) {
		CommandQueueProvider::GetInstance()->close(m_context);
	}
	delete ui;
}

void FaultsDevice::resetStored() {
	for (auto fault: this->m_faultsGroup->getFaults()) {
		fault->setStored(false);
	}
	this->updateExplanations();
}

void FaultsDevice::update() {
	this->readFaults();
	this->ui->lineEdit_numeric->setText(QString("0x%1").arg(m_faultNumeric, 8, 16, QLatin1Char('0')));
	this->m_faultsGroup->update(m_faultNumeric);
	this->updateExplanations();
}

void FaultsDevice::updateExplanation(int index)
{
	QLabel *lbl = dynamic_cast<QLabel*>(m_faultExplanationWidgets[index]);
	if (lbl) {
		QString updatedText = m_faultsGroup->getExplanation(index);
		lbl->setText(updatedText);
	}
	m_faults_explanation->ensurePolished();
}

void FaultsDevice::updateExplanations() {
	std::set<unsigned int> selected = m_faultsGroup->getSelectedIndexes();
	std::set<unsigned int> actives = m_faultsGroup->getActiveIndexes();
	if (selected.empty()) {
		for (int i = 0; i < m_faultExplanationWidgets.size(); ++i) {
			m_faultExplanationWidgets[i]->show();

			if (actives.contains(i)) {
				setDynamicProperty(m_faultExplanationWidgets[i], "highlighted", true);
			} else {
				setDynamicProperty(m_faultExplanationWidgets[i], "highlighted", false);
			}
		}
	} else {
		for (int i = 0; i < m_faultExplanationWidgets.size(); ++i) {
			if (selected.contains(i)) {
				m_faultExplanationWidgets[i]->show();
			} else {
				m_faultExplanationWidgets[i]->hide();
			}

			if (actives.contains(i)) {
				setDynamicProperty(m_faultExplanationWidgets[i], "highlighted", true);
			} else {
				setDynamicProperty(m_faultExplanationWidgets[i], "highlighted", false);
			}
		}
	}

	m_faults_explanation->ensurePolished();
}

void FaultsDevice::updateMinimumHeight() {
	this->ensurePolished();
	this->m_faults_explanation->ensurePolished();
	this->m_faultsGroup->ensurePolished();
}

void FaultsDevice::initFaultExplanations() {
	m_faults_explanation->setLayout(new QVBoxLayout(m_faults_explanation));
	m_faults_explanation->layout()->setContentsMargins(0, 0, 0, 0);
	m_faults_explanation->layout()->setSpacing(0);
	m_faults_explanation->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
	m_faults_explanation->layout()->setSizeConstraint(QLayout::SetMinimumSize);

	QStringList l = this->m_faultsGroup->getExplanations();
	for (const auto &item: l) {
		auto widget = new QLabel(item, m_faults_explanation);
		widget->setTextFormat(Qt::PlainText);
		widget->setStyleSheet("QWidget[highlighted=true]{color:white;} QWidget{color:#5c5c5c;}");

		widget->setWordWrap(true);
		m_faultExplanationWidgets.push_back(widget);
		m_faults_explanation->layout()->addWidget(widget);
	}
	m_faults_explanation->ensurePolished();
}

void FaultsDevice::connectSignalsAndSlots() {
	connect(this->ui->clear_selection_button, &QPushButton::clicked, this->m_faultsGroup,
		&FaultsGroup::clearSelection);
	connect(this->ui->reset_button, &QPushButton::clicked, this, &FaultsDevice::resetStored);
	connect(m_faultsGroup, &FaultsGroup::selectionUpdated, this, &FaultsDevice::updateExplanations);
	connect(m_faultsGroup, &FaultsGroup::minimumSizeChanged, this, &FaultsDevice::updateMinimumHeight);
}

void FaultsDevice::readFaults()
{
	if (!m_readFaultCommand) {
		m_readFaultCommand = new IioChannelAttributeRead(m_faultsChannel, "raw", m_cmdQueue, true);
		connect(m_readFaultCommand, &scopy::Command::finished,
			this, [=, this](scopy::Command* cmd) {
			IioChannelAttributeRead *tcmd = dynamic_cast<IioChannelAttributeRead*>(cmd);
			if (!tcmd) {
				qCritical(CAT_SWIOT_FAULTS) << m_name << "faults value could not be read.";
				return;
			}
			char *fau = tcmd->getResult();

			if (tcmd->getReturnCode() < 0) {
				qCritical(CAT_SWIOT_FAULTS) << m_name << "faults value could not be read.";
			} else {
				qDebug(CAT_SWIOT_FAULTS) << m_name << "faults read the value:" << fau;
				try {
					m_faultNumeric = std::stoi(fau);
					qDebug(CAT_SWIOT_FAULTS) << m_name << "faults read the value INT:" << m_faultNumeric;
				} catch (std::invalid_argument& exception) {
					qCritical(CAT_SWIOT_FAULTS) << m_name << "faults value could not be converted from string to int, read" << fau << "; exception message:" << exception.what();
				}
			}
		}, Qt::QueuedConnection);
	}
	m_cmdQueue->enqueue(m_readFaultCommand);
}

void FaultsDevice::functionConfigCmdFinished(scopy::Command *cmd)
{
	IioDeviceAttributeRead *tcmd = dynamic_cast<IioDeviceAttributeRead*>(cmd);
	if (!tcmd) {
		qCritical(CAT_SWIOT_FAULTS) << "Error: cannot read swiot special fault property";
		return;
	}

	if (tcmd->getReturnCode() < 0) {
		qCritical(CAT_SWIOT_FAULTS) << "Error: cannot read swiot special fault property";
	} else {
		int cmdIndex = m_functionConfigCmds.indexOf(cmd);
		char *readFunction = tcmd->getResult();
		Q_EMIT specialFaultsUpdated(cmdIndex, QString(readFunction));
		disconnect(cmd, &scopy::Command::finished, this, &FaultsDevice::deviceConfigCmdFinished);
	}
}

void FaultsDevice::deviceConfigCmdFinished(scopy::Command *cmd)
{
	IioDeviceAttributeRead *tcmd = dynamic_cast<IioDeviceAttributeRead*>(cmd);
	if (!tcmd) {
		qCritical(CAT_SWIOT_FAULTS) << "Error: cannot read swiot special fault config property";
		return;
	}

	if (tcmd->getReturnCode() < 0) {
		qCritical(CAT_SWIOT_FAULTS) << "Error: cannot read swiot special fault config property";
	} else {
		char *readDevice = tcmd->getResult();
		int cmdIndex = m_deviceConfigCmds.indexOf(cmd);
		if (std::string(readDevice) == "ad74413r") {
			QString function = "ch" + QString::number(cmdIndex) + "_function";

			m_functionConfigCmds[cmdIndex] = new IioDeviceAttributeRead(m_swiot, function.toStdString().c_str(), m_cmdQueue);
			connect(m_functionConfigCmds.at(cmdIndex), &scopy::Command::finished, this,
				&FaultsDevice::functionConfigCmdFinished, Qt::QueuedConnection);
			disconnect(cmd, &scopy::Command::finished, this, &FaultsDevice::deviceConfigCmdFinished);
			m_cmdQueue->enqueue(m_functionConfigCmds.at(cmdIndex));
		}
	}
}

void FaultsDevice::initSpecialFaults()
{
	if (!m_name.contains("MAX")) {
		for (int i = 0; i < SWIOT_NB_CHANNELS; ++i) {
			std::string device = "ch" + std::to_string(i) + "_device";
			m_deviceConfigCmds.push_back(new IioDeviceAttributeRead(m_swiot, device.c_str(), m_cmdQueue));
			connect(m_deviceConfigCmds.at(i), &scopy::Command::finished, this,
				&FaultsDevice::deviceConfigCmdFinished, Qt::QueuedConnection);
			m_functionConfigCmds.push_back(nullptr);
			m_cmdQueue->enqueue(m_deviceConfigCmds.at(i));
		}
	}
}
