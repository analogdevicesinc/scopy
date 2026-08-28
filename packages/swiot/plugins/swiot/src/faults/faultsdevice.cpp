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

#include <component/device.h>
#include <component/attribute.h>
#include <component/attributereader.h>
#include <component/backends/iio/iioregisterreader.h>

#include <qcorotask.h>

#include <style.h>
#include <gui/stylehelper.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/dynamicWidget.h>
#include "style_properties.h"

#define FAULT_CHANNEL_NAME "voltage"
#define SWIOT_NB_CHANNELS 4

using namespace scopy::swiot;

FaultsDevice::FaultsDevice(const QString &name, QString path, QString uri, QVector<uint32_t> &registers,
			   QWidget *parent)
	: QWidget(parent)
	, m_uri(uri)
	, m_name(name.toUpper())
	, m_device(nullptr)
	, m_swiot(nullptr)
	, m_registerReader(nullptr)
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
	connect(this, &FaultsDevice::faultNumericUpdated, this, &FaultsDevice::onFaultNumericUpdated);
	connect(this, &FaultsDevice::faultRegisterRead, this, &FaultsDevice::onFaultRegisterRead);

	connectSignalsAndSlots();

	m_faultsExplanation->ensurePolished();
	onFaultNumericUpdated();
}

FaultsDevice::~FaultsDevice()
{
	if(!m_readRegTask.isReady()) {
		QCoro::waitFor(m_readRegTask);
	}
	m_readRegTask = {};
	m_context = {};
}

void FaultsDevice::resetStored()
{
	for(auto fault : m_faultsGroup->getFaults()) {
		fault->setStored(false);
	}
	updateExplanations();
}

void FaultsDevice::update() { readRegister(); }

void FaultsDevice::readRegister()
{

	if(!m_readRegTask.isReady()) {
		return;
	}
	m_readRegTask = readRegisters();
}

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
		}
	} else {
		for(int i = 0; i < m_faultExplanationWidgets.size(); ++i) {
			if(selected.contains(i)) {
				m_faultExplanationWidgets[i]->show();
			} else {
				m_faultExplanationWidgets[i]->hide();
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
			faultRegisterValue |= (m_registerValues[i] << (i * 8));
		}
		m_faultNumeric = faultRegisterValue;
		Q_EMIT faultNumericUpdated();
		m_registerValues.clear();
	}
}

QCoro::Task<void> FaultsDevice::readRegisters()
{
	if(!m_registerReader) {
		qCritical(CAT_SWIOT_FAULTS) << m_name << "has no register-read capability.";
		co_return;
	}
	for(int i = 0; i < m_registers.size(); i++) {
		uint32_t address = m_registers.at(i);
		auto r = co_await m_registerReader->readAsync(address);
		if(!r) {
			qCritical(CAT_SWIOT_FAULTS) << m_name << "faults register could not be read.";
			continue;
		}
		uint32_t reg = r.value();
		qDebug(CAT_SWIOT_FAULTS) << m_name << "faults register read val:" << reg;
		Q_EMIT faultRegisterRead(i, reg);
	}
}

void FaultsDevice::establishConnection(QString name)
{
	m_context = component::Controller::context(m_uri);
	if(!m_context) {
		return;
	}
	m_device = m_context->findChild<component::Device *>(name);
	m_swiot = m_context->findChild<component::Device *>("swiot");
	if(m_device) {
		m_registerReader = m_device->findChild<component::iio::IIORegisterReader *>();
	}
}

QCoro::Task<void> FaultsDevice::initSpecialFaults()
{
	if(m_name.contains("MAX") || !m_swiot) {
		co_return;
	}
	for(int i = 0; i < SWIOT_NB_CHANNELS; ++i) {
		QString deviceAttrName = "ch" + QString::number(i) + "_device";
		component::Attribute *deviceAttr =
			m_swiot->findChild<component::Attribute *>(deviceAttrName, Qt::FindDirectChildrenOnly);
		if(!deviceAttr || !deviceAttr->readCapability()) {
			continue;
		}
		auto rd = co_await deviceAttr->readCapability()->readAsync();
		if(!rd || deviceAttr->cachedValue() != "ad74413r") {
			continue;
		}
		QString functionAttrName = "ch" + QString::number(i) + "_function";
		component::Attribute *functionAttr =
			m_swiot->findChild<component::Attribute *>(functionAttrName, Qt::FindDirectChildrenOnly);
		if(!functionAttr || !functionAttr->readCapability()) {
			continue;
		}
		auto rf = co_await functionAttr->readCapability()->readAsync();
		if(!rf) {
			continue;
		}
		Q_EMIT specialFaultsUpdated(i, functionAttr->cachedValue());
	}
}

QWidget *FaultsDevice::createTopWidget(QWidget *parent)
{
	QWidget *w = new QWidget(parent);
	w->setMinimumSize(700, 60);
	QHBoxLayout *layout = new QHBoxLayout(w);
	layout->setContentsMargins(0, 0, 0, 0);

	QLabel *deviceNameLabel = new QLabel(w);
	Style::setStyle(deviceNameLabel, style::properties::label::menuBig);
	deviceNameLabel->setText(m_name);

	m_registerNoLineEdit = new QLineEdit(w);
	Style::setStyle(m_registerNoLineEdit, style::properties::widget::solidBorder);
	m_registerNoLineEdit->setFixedWidth(140);
	m_registerNoLineEdit->setPlaceholderText("0x0");
	m_registerNoLineEdit->setFocusPolicy(Qt::NoFocus);
	m_registerNoLineEdit->setReadOnly(true);

	m_resetBtn = new QPushButton(w);
	StyleHelper::BasicButton(m_resetBtn);
	m_resetBtn->setCheckable(false);
	m_resetBtn->setText("Reset stored");

	m_clearBtn = new QPushButton(w);
	StyleHelper::BasicButton(m_clearBtn);
	m_clearBtn->setCheckable(false);
	m_clearBtn->setText("Clear selection");

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
		Style::setStyle(widget, style::properties::label::subtle);

		m_faultExplanationWidgets.push_back(widget);
		m_faultsExplanation->layout()->addWidget(widget);
	}
	m_faultsExplanation->ensurePolished();

	m_explanationSection = new MenuCollapseSection("Faults explanation", MenuCollapseSection::MHCW_ONOFF,
						       MenuCollapseSection::MHW_BASEWIDGET, this);
	m_explanationSection->setLayout(new QVBoxLayout());
	m_explanationSection->contentLayout()->setSpacing(10);
	m_explanationSection->contentLayout()->setContentsMargins(0, 0, 0, 0);

	m_explanationSection->contentLayout()->addWidget(m_faultsExplanation);

	return m_explanationSection;
}

void FaultsDevice::initTutorialProperties()
{
	m_registerNoLineEdit->setProperty("tutorial_name", "AD74413R_NUMERIC");
	m_resetBtn->setProperty("tutorial_name", "AD74413R_RESET_STORED");
	m_clearBtn->setProperty("tutorial_name", "AD74413R_CLEAR_SELECTION");
	m_faultsGroup->setProperty("tutorial_name", "AD74413R_FAULTS_GROUP");
	m_faultsExplanation->setProperty("tutorial_name", "AD74413R_EXPLANATIONS");
}

#include "moc_faultsdevice.cpp"
