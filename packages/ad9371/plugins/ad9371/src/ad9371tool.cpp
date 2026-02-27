/*
 * Copyright (c) 2025 Analog Devices Inc.
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
 *
 */

#include "ad9371tool.h"

#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <style.h>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_AD9371TOOL, "AD9371Tool");

using namespace scopy;
using namespace ad9371;

AD9371::AD9371(iio_context *ctx, IIOWidgetGroup *group, QWidget *parent)
	: QWidget(parent)
	, m_ctx(ctx)
	, m_group(group)
{
	m_mainLayout = new QVBoxLayout(this);
	m_mainLayout->setMargin(0);
	m_mainLayout->setContentsMargins(0, 0, 0, 0);

	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->topContainerMenuControl()->setVisible(false);

	m_mainLayout->addWidget(m_tool);

	m_refreshButton = new AnimatedRefreshBtn(false, this);
	m_tool->addWidgetToTopContainerHelper(m_refreshButton, TTA_RIGHT);

	connect(m_refreshButton, &QPushButton::clicked, this, [this]() {
		m_refreshButton->startAnimation();

		QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
		connect(
			watcher, &QFutureWatcher<void>::finished, this,
			[this, watcher]() {
				m_refreshButton->stopAnimation();
				watcher->deleteLater();
			},
			Qt::QueuedConnection);

		QFuture<void> future = QtConcurrent::run([this]() { Q_EMIT readRequested(); });

		watcher->setFuture(future);
	});

	// --- Controls tab ---
	m_controlsWidget = new QWidget(this);
	QVBoxLayout *controlsLayout = new QVBoxLayout(m_controlsWidget);
	controlsLayout->setMargin(0);
	controlsLayout->setContentsMargins(0, 0, 0, 0);
	m_controlsWidget->setLayout(controlsLayout);

	QWidget *controlsWidget = new QWidget(this);
	QVBoxLayout *controlWidgetLayout = new QVBoxLayout(controlsWidget);
	controlWidgetLayout->setMargin(0);
	controlWidgetLayout->setContentsMargins(0, 0, 0, 0);
	controlsWidget->setLayout(controlWidgetLayout);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(controlsWidget);

	controlsLayout->addWidget(scrollArea);

	if(m_ctx != nullptr) {
		// Find required devices
		iio_device *phyDev = iio_context_find_device(m_ctx, "ad9371-phy");
		iio_device *ddsDev = iio_context_find_device(m_ctx, "axi-ad9371-tx-hpc");
		iio_device *capDev = iio_context_find_device(m_ctx, "axi-ad9371-rx-hpc");
		iio_device *capObsDev = iio_context_find_device(m_ctx, "axi-ad9371-rx-obs-hpc");

		if(!phyDev) {
			qWarning(CAT_AD9371TOOL) << "ad9371-phy device not found";
		}
		if(!ddsDev) {
			qWarning(CAT_AD9371TOOL) << "axi-ad9371-tx-hpc device not found";
		}
		if(!capDev) {
			qWarning(CAT_AD9371TOOL) << "axi-ad9371-rx-hpc device not found";
		}
		if(!capObsDev) {
			qWarning(CAT_AD9371TOOL) << "axi-ad9371-rx-obs-hpc device not found";
		}

		// Detect 2RX/2TX configuration
		bool is_2rx_2tx = false;
		if(phyDev) {
			iio_channel *voltage1 = iio_device_find_channel(phyDev, "voltage1", false);
			is_2rx_2tx = voltage1 && iio_channel_find_attr(voltage1, "hardwaregain");
		}
		qInfo(CAT_AD9371TOOL) << "is_2rx_2tx:" << is_2rx_2tx;

		// Detect DPD support
		bool has_dpd = false;
		if(phyDev) {
			iio_channel *tx0 = iio_device_find_channel(phyDev, "voltage0", true);
			has_dpd = tx0 && iio_channel_find_attr(tx0, "dpd_tracking_en");
		}
		qInfo(CAT_AD9371TOOL) << "has_dpd:" << has_dpd;

		m_helper = new AD9371Helper(m_group, this);
		connect(this, &AD9371::readRequested, m_helper, &AD9371Helper::readRequested);

		if(phyDev) {
			// Global Settings section
			controlWidgetLayout->addWidget(m_helper->generateGlobalSettingsWidget(
				phyDev, has_dpd, "AD9371 Global Settings", controlsWidget));

			// Receive Chain section
			controlWidgetLayout->addWidget(m_helper->generateRxChainWidget(
				phyDev, capDev, is_2rx_2tx, "AD9371 Receive Chain", controlsWidget));

			// Observation RX section
			controlWidgetLayout->addWidget(m_helper->generateObsRxChainWidget(
				phyDev, "AD9371 Observation Receive", controlsWidget));

			// Transmit Chain section
			controlWidgetLayout->addWidget(m_helper->generateTxChainWidget(
				phyDev, has_dpd, is_2rx_2tx, "AD9371 Transmit Chain", controlsWidget));
		}

		if(ddsDev || capDev) {
			// FPGA section
			controlWidgetLayout->addWidget(m_helper->generateFpgaWidget(
				ddsDev, capDev, is_2rx_2tx, "AD9371 FPGA Settings", controlsWidget));
		}

		controlWidgetLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));
	}

	m_tool->addWidgetToCentralContainerHelper(m_controlsWidget);
}

AD9371::~AD9371() {}
