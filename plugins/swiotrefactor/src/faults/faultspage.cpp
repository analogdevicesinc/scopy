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
#include <iioutil/connectionprovider.h>

using namespace scopy::swiotrefactor;

FaultsPage::FaultsPage(QString uri, QWidget *parent)
	: QWidget(parent)
	, m_uri(uri)
{
	Connection *conn = ConnectionProvider::open(m_uri);
	m_context = conn->context();
	setupDevices();

	StyleHelper::BackgroundWidget(this);
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	setLayout(layout);

	QScrollArea *scrollArea = new QScrollArea(this);
	QWidget *scrollWidget = new QWidget(scrollArea);
	QVBoxLayout *layScroll = new QVBoxLayout(scrollWidget);
	layScroll->setMargin(0);
	layScroll->setContentsMargins(0, 0, 0, 0);
	scrollWidget->setLayout(layScroll);

	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(scrollWidget);

	// needed for subsection separator resize
	m_ad74413rFaultsDevice->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_max14906FaultsDevice->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	layScroll->addWidget(m_ad74413rFaultsDevice);
	layScroll->addWidget(m_max14906FaultsDevice);
	layScroll->addItem(new QSpacerItem(0, 0, QSizePolicy::Preferred, QSizePolicy::Expanding));

	layout->addWidget(scrollArea);
}

FaultsPage::~FaultsPage() { ConnectionProvider::close(m_uri); }

void FaultsPage::update()
{
	m_ad74413rFaultsDevice->update();
	m_max14906FaultsDevice->update();
}

void FaultsPage::setupDevices()
{
	struct iio_device *ad74413r = iio_context_find_device(m_context, "ad74413r");
	struct iio_device *max14906 = iio_context_find_device(m_context, "max14906");
	struct iio_device *swiot = iio_context_find_device(m_context, "swiot");

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
