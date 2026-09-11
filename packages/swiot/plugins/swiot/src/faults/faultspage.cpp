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

#include "faults/faultspage.h"
#include "swiot_logging_categories.h"
#include <QScrollArea>
#include <gui/stylehelper.h>
#include <component/device.h>

using namespace scopy::swiot;

FaultsPage::FaultsPage(QString uri, QWidget *parent)
	: QWidget(parent)
	, m_uri(uri)
{
	m_context = component::Controller::context(m_uri);
	setupDevices();

	StyleHelper::BackgroundWidget(this);
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	setLayout(layout);

	QScrollArea *scrollArea = new QScrollArea(this);
	QWidget *scrollWidget = new QWidget(scrollArea);
	QVBoxLayout *layScroll = new QVBoxLayout(scrollWidget);
	layScroll->setContentsMargins(0, 0, 0, 0);
	layScroll->setContentsMargins(0, 0, 0, 0);
	scrollWidget->setLayout(layScroll);

	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(scrollWidget);

	// needed for subsection separator resize
	if(m_ad74413rFaultsDevice != nullptr) {
		m_ad74413rFaultsDevice->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
		layScroll->addWidget(m_ad74413rFaultsDevice);
	}
	if(m_max14906FaultsDevice != nullptr) {
		m_max14906FaultsDevice->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
		layScroll->addWidget(m_max14906FaultsDevice);
	}
	layScroll->addItem(new QSpacerItem(0, 0, QSizePolicy::Preferred, QSizePolicy::Expanding));

	layout->addWidget(scrollArea);
}

FaultsPage::~FaultsPage() { m_context = {}; }

void FaultsPage::update()
{
	if(m_ad74413rFaultsDevice != nullptr) {
		m_ad74413rFaultsDevice->update();
	}
	if(m_max14906FaultsDevice != nullptr) {
		m_max14906FaultsDevice->update();
	}
}

void FaultsPage::setupDevices()
{
	if(!m_context) {
		qCritical(CAT_SWIOT_FAULTS) << "Error: no context available.";
		return;
	}
	component::Device *ad74413r = m_context->findChild<component::Device *>("ad74413r");
	component::Device *max14906 = m_context->findChild<component::Device *>("max14906");
	component::Device *swiot = m_context->findChild<component::Device *>("swiot");

	if(swiot) {
		if(ad74413r) {
			QVector<uint32_t> faultRegistersAddr = {0x02e};
			m_ad74413rFaultsDevice = new FaultsDevice("ad74413r", ":/swiot/swiot_faults.json", m_uri,
								  faultRegistersAddr, this);
		} else {
			qCritical(CAT_SWIOT_FAULTS) << "Error: did not find ad74413r device.";
		}

		if(max14906) {
			QVector<uint32_t> faultRegistersAddr = {0x04, 0x05, 0x06, 0x07};
			m_max14906FaultsDevice = new FaultsDevice("max14906", ":/swiot/swiot_faults.json", m_uri,
								  faultRegistersAddr, this);
		} else {
			qCritical(CAT_SWIOT_FAULTS) << "Error: did not find max14906 device.";
		}
	} else {
		qCritical(CAT_SWIOT_FAULTS) << "Error: did not find swiot device.";
	}
}

#include "moc_faultspage.cpp"
