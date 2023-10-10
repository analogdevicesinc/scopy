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

#include "faultspage.h"

#include "src/swiot_logging_categories.h"

#include "ui_faultspage.h"

using namespace scopy::swiot;

FaultsPage::FaultsPage(struct iio_context *context, QWidget *parent)
	: QWidget(parent)
	, m_context(context)
	, ui(new Ui::FaultsPage)
	, m_ad74413rFaultsDevice(nullptr)
	, m_max14906FaultsDevice(nullptr)
{
	ui->setupUi(this);
	this->setupDevices();

	// needed for subsection separator resize
	this->m_ad74413rFaultsDevice->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	this->m_max14906FaultsDevice->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	this->ui->mainVerticalLayout->addWidget(this->m_ad74413rFaultsDevice);
	this->ui->mainVerticalLayout->addWidget(this->m_max14906FaultsDevice);
	this->ui->mainVerticalLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Preferred, QSizePolicy::Expanding));

	this->ui->frame->setStyleSheet("QFrame#frame{background-color:#1C1C20;}");
}

FaultsPage::~FaultsPage() { delete ui; }

void FaultsPage::update()
{
	this->m_ad74413rFaultsDevice->update();
	this->m_max14906FaultsDevice->update();
}

void FaultsPage::setupDevices()
{
	struct iio_device *ad74413r = iio_context_find_device(m_context, "ad74413r");
	struct iio_device *max14906 = iio_context_find_device(m_context, "max14906");
	struct iio_device *swiot = iio_context_find_device(m_context, "swiot");

	if(swiot) {
		if(ad74413r) {
			QVector<uint32_t> faultRegistersAddr = {0x02e};
			m_ad74413rFaultsDevice = new FaultsDevice("ad74413r", ":/swiot/swiot_faults.json", ad74413r,
								  swiot, m_context, faultRegistersAddr, this);
		} else {
			qCritical(CAT_SWIOT_FAULTS) << "Error: did not find ad74413r device.";
		}

		if(max14906) {
			QVector<uint32_t> faultRegistersAddr = {0x04, 0x05, 0x06, 0x07};
			m_max14906FaultsDevice = new FaultsDevice("max14906", ":/swiot/swiot_faults.json", max14906,
								  swiot, m_context, faultRegistersAddr, this);
		} else {
			qCritical(CAT_SWIOT_FAULTS) << "Error: did not find max14906 device.";
		}
	} else {
		qCritical(CAT_SWIOT_FAULTS) << "Error: did not find swiot device.";
	}
}
