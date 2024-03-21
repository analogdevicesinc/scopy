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

#include "faults/faultsdevice.h"
#include "faults/faultsgroup.h"
#include "swiot_logging_categories.h"

#include <iioutil/connectionprovider.h>
#include <iioutil/iiocommand/iiochannelattributeread.h>
#include <iioutil/iiocommand/iiodeviceattributeread.h>
#include <iioutil/iiocommand/iioregisterread.h>

#include <gui/stylehelper.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/dynamicWidget.h>
#include <utility>

#define FAULT_CHANNEL_NAME "voltage"
#define SWIOT_NB_CHANNELS 4

using namespace scopy::swiotrefactor;

FaultsDevice::FaultsDevice(const QString &name, QString path, QString uri, QVector<uint32_t> &registers,
			   QWidget *parent)
	: QWidget(parent)
	, m_uri(uri)
	, m_name(name.toUpper())
	, m_registers(registers)
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	setLayout(layout);

	establishConnection(name);
	initSpecialFaults();

	if(m_device == nullptr) {
		qCritical(CAT_SWIOT_FAULTS) << "No device was found";
	}
	m_faultsGroup = new FaultsGroup(name, path, this);

	QWidget *topWidget = createTopWidget(this);
	QWidget *faultsWidget = new QWidget(this);
	faultsWidget->setLayout(new QHBoxLayout(faultsWidget));
	faultsWidget->layout()->setContentsMargins(0, 0, 0, 0);
	faultsWidget->layout()->addWidget(m_faultsGroup);
	QWidget *explanationSection = createExplanationSection(this);

	layout->addWidget(topWidget);
	layout->addWidget(faultsWidget);
	layout->addWidget(explanationSection);
	layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	// initialize components that might be used for the Faults tutorial
	if(m_name == "AD74413R") {
		initTutorialProperties();
	}

	connect(this, &FaultsDevice::specialFaultsUpdated, m_faultsGroup, &FaultsGroup::specialFaultsUpdated);
	connect(m_faultsGroup, &FaultsGroup::specialFaultExplanationChanged, this, &FaultsDevice::updateExplanation);
	connect(this, &FaultsDevice::faultNumericUpdated, this, &FaultsDevice::onFaultNumericUpdated,
		Qt::QueuedConnection);
	connect(this, &FaultsDevice::faultRegisterRead, this, &FaultsDevice::onFaultRegisterRead, Qt::QueuedConnection);

	connectSignalsAndSlots();

	m_faultsExplanation->ensurePolished();
	onFaultNumericUpdated();
}

FaultsDevice::~FaultsDevice() { ConnectionProvider::close(m_uri); }

void FaultsDevice::resetStored()
{
	for(auto fault : m_faultsGroup->getFaults()) {
		fault->setStored(false);
	}
	updateExplanations();
}

void FaultsDevice::update() { readRegister(); }

void FaultsDevice::updateExplanation(int index)
{
	QLabel *lbl = dynamic_cast<QLabel *>(m_faultExplanationWidgets[index]);
	if(lbl) {
		QString updatedText = m_faultsGroup->getExplanation(index);
		lbl->setText(updatedText);
	}
	m_faultsExplanation->ensurePolished();
}

void FaultsDevice::updateExplanations()
{
	std::set<unsigned int> selected = m_faultsGroup->getSelectedIndexes();
	std::set<unsigned int> actives = m_faultsGroup->getActiveIndexes();
	if(selected.empty()) {
		for(int i = 0; i < m_faultExplanationWidgets.size(); ++i) {
			m_faultExplanationWidgets[i]->show();

			if(actives.contains(i)) {
				setDynamicProperty(m_faultExplanationWidgets[i], "highlighted", true);
			} else {
				setDynamicProperty(m_faultExplanationWidgets[i], "highlighted", false);
			}
		}
	} else {
		for(int i = 0; i < m_faultExplanationWidgets.size(); ++i) {
			if(selected.contains(i)) {
				m_faultExplanationWidgets[i]->show();
			} else {
				m_faultExplanationWidgets[i]->hide();
			}

			if(actives.contains(i)) {
				setDynamicProperty(m_faultExplanationWidgets[i], "highlighted", true);
			} else {
				setDynamicProperty(m_faultExplanationWidgets[i], "highlighted", false);
			}
		}
	}

	m_faultsExplanation->ensurePolished();
}

void FaultsDevice::updateMinimumHeight()
{
	ensurePolished();
	m_faultsExplanation->ensurePolished();
	m_faultsGroup->ensurePolished();
}

void FaultsDevice::connectSignalsAndSlots()
{
	connect(m_clearBtn, &QPushButton::clicked, m_faultsGroup, &FaultsGroup::clearSelection);
	connect(m_resetBtn, &QPushButton::clicked, this, &FaultsDevice::resetStored);
	connect(m_faultsGroup, &FaultsGroup::selectionUpdated, this, &FaultsDevice::updateExplanations);
	connect(m_faultsGroup, &FaultsGroup::minimumSizeChanged, this, &FaultsDevice::updateMinimumHeight);
}

void FaultsDevice::onFaultNumericUpdated()
{
	m_registerNoLineEdit->setText(QString("0x%1").arg(m_faultNumeric, 8, 16, QLatin1Char('0')));
	m_faultsGroup->update(m_faultNumeric);
	updateExplanations();
}

void FaultsDevice::onFaultRegisterRead(int iReg, uint32_t value)
{
	m_registerValues.insert(iReg, value);
	if(m_registerValues.size() == m_registers.size()) {
		uint32_t faultRegisterValue = 0;
		for(int i = 0; i < m_registerValues.size(); i++) {
			faultRegisterValue |= (m_registerValues.at(i) << (i * 8));
		}
		m_faultNumeric = faultRegisterValue;
		Q_EMIT faultNumericUpdated();
		m_registerValues.clear();
	}
}

void FaultsDevice::readRegister()
{
	for(int i = 0; i < m_registers.size(); i++) {
		uint32_t reg_val;
		uint32_t address = m_registers.at(i);
		Command *readRegisterCommand = new IioRegisterRead(m_device, address, nullptr);
		connect(
			readRegisterCommand, &scopy::Command::finished, this,
			[=, this](scopy::Command *cmd) {
				IioRegisterRead *tcmd = dynamic_cast<IioRegisterRead *>(cmd);
				if(!tcmd) {
					qCritical(CAT_SWIOT_FAULTS) << m_name << "faults register could not be read.";
					return;
				}
				uint32_t reg = tcmd->getResult();

				if(tcmd->getReturnCode() < 0) {
					qCritical(CAT_SWIOT_FAULTS) << m_name << "faults register could not be read.";
				} else {
					qDebug(CAT_SWIOT_FAULTS) << m_name << "faults register read val:" << reg;
					try {
						Q_EMIT faultRegisterRead(i, reg);
					} catch(std::invalid_argument &exception) {
						qCritical(CAT_SWIOT_FAULTS)
							<< m_name << "faults register could not be read.";
					}
				}
			},
			Qt::QueuedConnection);
		m_cmdQueue->enqueue(readRegisterCommand);
	}
}

void FaultsDevice::functionConfigCmdFinished(scopy::Command *cmd)
{
	IioDeviceAttributeRead *tcmd = dynamic_cast<IioDeviceAttributeRead *>(cmd);
	if(!tcmd) {
		qCritical(CAT_SWIOT_FAULTS) << "Error: cannot read swiot special fault property";
		return;
	}

	if(tcmd->getReturnCode() < 0) {
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
	IioDeviceAttributeRead *tcmd = dynamic_cast<IioDeviceAttributeRead *>(cmd);
	if(!tcmd) {
		qCritical(CAT_SWIOT_FAULTS) << "Error: cannot read swiot special fault config property";
		return;
	}

	if(tcmd->getReturnCode() < 0) {
		qCritical(CAT_SWIOT_FAULTS) << "Error: cannot read swiot special fault config property";
	} else {
		char *readDevice = tcmd->getResult();
		int cmdIndex = m_deviceConfigCmds.indexOf(cmd);
		if(std::string(readDevice) == "ad74413r") {
			QString function = "ch" + QString::number(cmdIndex) + "_function";

			m_functionConfigCmds[cmdIndex] =
				new IioDeviceAttributeRead(m_swiot, function.toStdString().c_str(), m_cmdQueue);
			connect(m_functionConfigCmds.at(cmdIndex), &scopy::Command::finished, this,
				&FaultsDevice::functionConfigCmdFinished, Qt::QueuedConnection);
			disconnect(cmd, &scopy::Command::finished, this, &FaultsDevice::deviceConfigCmdFinished);
			m_cmdQueue->enqueue(m_functionConfigCmds.at(cmdIndex));
		}
	}
}

void FaultsDevice::establishConnection(QString name)
{
	Connection *conn = ConnectionProvider::open(m_uri);
	m_context = conn->context();
	m_cmdQueue = conn->commandQueue();
	m_device = iio_context_find_device(m_context, name.toStdString().c_str());
	m_swiot = iio_context_find_device(m_context, "swiot");
}

void FaultsDevice::initSpecialFaults()
{
	if(!m_name.contains("MAX")) {
		for(int i = 0; i < SWIOT_NB_CHANNELS; ++i) {
			std::string device = "ch" + std::to_string(i) + "_device";
			m_deviceConfigCmds.push_back(new IioDeviceAttributeRead(m_swiot, device.c_str(), m_cmdQueue));
			connect(m_deviceConfigCmds.at(i), &scopy::Command::finished, this,
				&FaultsDevice::deviceConfigCmdFinished, Qt::QueuedConnection);
			m_functionConfigCmds.push_back(nullptr);
			m_cmdQueue->enqueue(m_deviceConfigCmds.at(i));
		}
	}
}

QWidget *FaultsDevice::createTopWidget(QWidget *parent)
{
	QWidget *w = new QWidget(parent);
	w->setMinimumSize(700, 60);
	QHBoxLayout *layout = new QHBoxLayout(w);
	layout->setContentsMargins(0, 0, 0, 0);

	QLabel *deviceNameLabel = new QLabel(w);
	StyleHelper::MenuLargeLabel(deviceNameLabel);
	deviceNameLabel->setText(m_name);

	m_registerNoLineEdit = new QLineEdit(w);
	StyleHelper::MenuLineEdit(m_registerNoLineEdit);
	m_registerNoLineEdit->setFixedWidth(140);
	m_registerNoLineEdit->setPlaceholderText("0x0");
	m_registerNoLineEdit->setFocusPolicy(Qt::NoFocus);
	m_registerNoLineEdit->setReadOnly(true);

	m_resetBtn = new QPushButton(w);
	StyleHelper::BlueButton(m_resetBtn);
	m_resetBtn->setFixedWidth(110);
	m_resetBtn->setCheckable(false);
	m_resetBtn->setText("RESET\nSTORED");

	m_clearBtn = new QPushButton(w);
	StyleHelper::BlueButton(m_clearBtn);
	m_clearBtn->setFixedWidth(110);
	m_clearBtn->setCheckable(false);
	m_clearBtn->setText("CLEAR\nSELECTION");

	layout->addWidget(deviceNameLabel);
	layout->addWidget(m_registerNoLineEdit);
	layout->addWidget(m_resetBtn);
	layout->addWidget(m_clearBtn);
	layout->addItem(new QSpacerItem(40, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));

	return w;
}

QWidget *FaultsDevice::createExplanationSection(QWidget *parent)
{
	m_faultsExplanation = new QWidget(this);
	m_faultsExplanation->setLayout(new QVBoxLayout(m_faultsExplanation));
	m_faultsExplanation->layout()->setContentsMargins(0, 0, 0, 0);
	m_faultsExplanation->layout()->setSpacing(0);
	m_faultsExplanation->layout()->setSizeConstraint(QLayout::SetMinimumSize);

	QStringList l = m_faultsGroup->getExplanations();
	for(const auto &item : l) {
		auto widget = new QLabel(item, m_faultsExplanation);
		widget->setTextFormat(Qt::PlainText);
		widget->setWordWrap(true);
		StyleHelper::FaultsExplanation(widget);

		m_faultExplanationWidgets.push_back(widget);
		m_faultsExplanation->layout()->addWidget(widget);
	}
	m_faultsExplanation->ensurePolished();

	MenuCollapseSection *explanationSection =
		new MenuCollapseSection("Faults explanation", MenuCollapseSection::MHCW_ONOFF, this);
	explanationSection->setLayout(new QVBoxLayout());
	explanationSection->contentLayout()->setSpacing(10);
	explanationSection->contentLayout()->setMargin(0);

	explanationSection->contentLayout()->addWidget(m_faultsExplanation);

	return explanationSection;
}

void FaultsDevice::initTutorialProperties()
{
	m_registerNoLineEdit->setProperty("tutorial_name", "AD74413R_NUMERIC");
	m_resetBtn->setProperty("tutorial_name", "AD74413R_RESET_STORED");
	m_clearBtn->setProperty("tutorial_name", "AD74413R_CLEAR_SELECTION");
	m_faultsGroup->setProperty("tutorial_name", "AD74413R_FAULTS_GROUP");
	m_faultsExplanation->setProperty("tutorial_name", "AD74413R_EXPLANATIONS");
}
