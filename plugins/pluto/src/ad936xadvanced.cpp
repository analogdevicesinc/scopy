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

#include "ad963xadvanced.h"

#include <QFutureWatcher>
#include <QtConcurrent>
#include <iiowidgetbuilder.h>
#include <menuonoffswitch.h>
#include <style.h>
#include <toolbuttons.h>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_AD936x_ADVANCED, "AD936x_ADVANCED")

using namespace scopy;
using namespace pluto;

AD936XAdvanced::AD936XAdvanced(iio_context *ctx, QWidget *parent)
	: m_ctx(ctx)
	, QWidget{parent}
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

	// main widget body

	QStackedWidget *centralWidget = new QStackedWidget(this);
	m_tool->addWidgetToCentralContainerHelper(centralWidget);

	QButtonGroup *navigationButtons = new QButtonGroup(this);
	navigationButtons->setExclusive(true);

	QPushButton *ensmModeClocksBtn = new QPushButton("ENSM/Mode/Clocks", this);
	Style::setStyle(ensmModeClocksBtn, style::properties::button::blueGrayButton);
	ensmModeClocksBtn->setCheckable(true);
	ensmModeClocksBtn->setChecked(true);

	if(m_ctx != nullptr) {
		iio_device *plutoDevice = nullptr;
		int device_count = iio_context_get_devices_count(ctx);
		for(int i = 0; i < device_count; ++i) {
			iio_device *dev = iio_context_get_device(ctx, i);
			const char *dev_name = iio_device_get_name(dev);
			if(dev_name && QString(dev_name).contains("ad936", Qt::CaseInsensitive)) {
				plutoDevice = dev;
				break;
			}
		}
		if(plutoDevice == nullptr) {
			qWarning(CAT_AD936x_ADVANCED) << "No AD936x device found in context!";
			return;
		}

		m_ensmModeClocks = new EnsmModeClocksWidget(plutoDevice, centralWidget);
		centralWidget->addWidget(m_ensmModeClocks);

		connect(this, &AD936XAdvanced::readRequested, m_ensmModeClocks, &EnsmModeClocksWidget::readRequested);
		connect(ensmModeClocksBtn, &QPushButton::clicked, this,
			[=, this]() { centralWidget->setCurrentWidget(m_ensmModeClocks); });

		QPushButton *eLnaBtn = new QPushButton("eLNA", this);
		Style::setStyle(eLnaBtn, style::properties::button::blueGrayButton);
		eLnaBtn->setCheckable(true);

		m_elna = nullptr;

		connect(eLnaBtn, &QPushButton::clicked, this, [=, this]() {
			if(m_elna == nullptr) {
				m_elna = new ElnaWidget(plutoDevice, centralWidget);
				connect(this, &AD936XAdvanced::readRequested, m_elna, &ElnaWidget::readRequested);
				centralWidget->addWidget(m_elna);
			}

			centralWidget->setCurrentWidget(m_elna);
		});

		QPushButton *rssiBtn = new QPushButton("RSSI", this);
		Style::setStyle(rssiBtn, style::properties::button::blueGrayButton);
		rssiBtn->setCheckable(true);

		m_rssi = nullptr;

		connect(rssiBtn, &QPushButton::clicked, this, [=, this]() {
			if(m_rssi == nullptr) {
				m_rssi = new RssiWidget(plutoDevice, centralWidget);
				connect(this, &AD936XAdvanced::readRequested, m_rssi, &RssiWidget::readRequested);
				centralWidget->addWidget(m_rssi);
			}

			centralWidget->setCurrentWidget(m_rssi);
		});

		QPushButton *gainBtn = new QPushButton("GAIN", this);
		Style::setStyle(gainBtn, style::properties::button::blueGrayButton);
		gainBtn->setCheckable(true);

		m_gainWidget = nullptr;

		connect(gainBtn, &QPushButton::clicked, this, [=, this]() {
			if(m_gainWidget == nullptr) {
				m_gainWidget = new GainWidget(plutoDevice, centralWidget);
				connect(this, &AD936XAdvanced::readRequested, m_gainWidget, &GainWidget::readRequested);
				centralWidget->addWidget(m_gainWidget);
			}

			centralWidget->setCurrentWidget(m_gainWidget);
		});

		QPushButton *txMonitorBtn = new QPushButton("TX MONITOR", this);
		Style::setStyle(txMonitorBtn, style::properties::button::blueGrayButton);
		txMonitorBtn->setCheckable(true);

		m_txMonitor = nullptr;

		connect(txMonitorBtn, &QPushButton::clicked, this, [=, this]() {
			if(m_txMonitor == nullptr) {
				m_txMonitor = new TxMonitorWidget(plutoDevice, centralWidget);
				connect(this, &AD936XAdvanced::readRequested, m_txMonitor,
					&TxMonitorWidget::readRequested);
				centralWidget->addWidget(m_txMonitor);
			}

			centralWidget->setCurrentWidget(m_txMonitor);
		});

		QPushButton *auxAdcDacIioBtn = new QPushButton("Aux ADC/DAC/IIO", this);
		Style::setStyle(auxAdcDacIioBtn, style::properties::button::blueGrayButton);
		auxAdcDacIioBtn->setCheckable(true);

		m_auxAdcDacIo = nullptr;

		connect(auxAdcDacIioBtn, &QPushButton::clicked, this, [=, this]() {
			if(m_auxAdcDacIo == nullptr) {
				m_auxAdcDacIo = new AuxAdcDacIoWidget(plutoDevice, centralWidget);
				connect(this, &AD936XAdvanced::readRequested, m_auxAdcDacIo,
					&AuxAdcDacIoWidget::readRequested);
				centralWidget->addWidget(m_auxAdcDacIo);
			}

			centralWidget->setCurrentWidget(m_auxAdcDacIo);
		});

		QPushButton *miscBtn = new QPushButton("MISC", this);
		Style::setStyle(miscBtn, style::properties::button::blueGrayButton);
		miscBtn->setCheckable(true);

		m_misc = nullptr;

		connect(miscBtn, &QPushButton::clicked, this, [=, this]() {
			if(m_misc == nullptr) {
				m_misc = new MiscWidget(plutoDevice, centralWidget);
				connect(this, &AD936XAdvanced::readRequested, m_misc, &MiscWidget::readRequested);
				centralWidget->addWidget(m_misc);
			}

			centralWidget->setCurrentWidget(m_misc);
		});

		QPushButton *bistBtn = new QPushButton("BIST", this);
		Style::setStyle(bistBtn, style::properties::button::blueGrayButton);
		bistBtn->setCheckable(true);

		m_bist = nullptr;

		connect(bistBtn, &QPushButton::clicked, this, [=, this]() {
			if(m_bist == nullptr) {
				m_bist = new BistWidget(plutoDevice, centralWidget);
				connect(this, &AD936XAdvanced::readRequested, m_bist, &BistWidget::readRequested);
				centralWidget->addWidget(m_bist);
			}

			centralWidget->setCurrentWidget(m_bist);
		});

		navigationButtons->addButton(ensmModeClocksBtn);
		navigationButtons->addButton(eLnaBtn);
		navigationButtons->addButton(rssiBtn);
		navigationButtons->addButton(gainBtn);
		navigationButtons->addButton(txMonitorBtn);
		navigationButtons->addButton(auxAdcDacIioBtn);
		navigationButtons->addButton(miscBtn);
		navigationButtons->addButton(bistBtn);

		m_tool->addWidgetToTopContainerHelper(ensmModeClocksBtn, TTA_LEFT);
		m_tool->addWidgetToTopContainerHelper(eLnaBtn, TTA_LEFT);
		m_tool->addWidgetToTopContainerHelper(rssiBtn, TTA_LEFT);
		m_tool->addWidgetToTopContainerHelper(gainBtn, TTA_LEFT);
		m_tool->addWidgetToTopContainerHelper(txMonitorBtn, TTA_LEFT);
		m_tool->addWidgetToTopContainerHelper(auxAdcDacIioBtn, TTA_LEFT);
		m_tool->addWidgetToTopContainerHelper(miscBtn, TTA_LEFT);
		m_tool->addWidgetToTopContainerHelper(bistBtn, TTA_LEFT);
	}
}

AD936XAdvanced::~AD936XAdvanced() {}
