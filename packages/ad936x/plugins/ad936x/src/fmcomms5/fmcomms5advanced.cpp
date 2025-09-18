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

#include "fmcomms5/fmcomms5advanced.h"

#include <QFutureWatcher>
#include <QtConcurrent>
#include <iiowidgetbuilder.h>
#include <menuonoffswitch.h>
#include <style.h>
#include <toolbuttons.h>
#include <QLoggingCategory>
#include <pluginbase/preferences.h>

#include <ad9361.h>

Q_LOGGING_CATEGORY(CAT_FMCOMMS5_ADVANCED, "FMCOMMS5_ADVANCED")

using namespace scopy;
using namespace ad936x;

Fmcomms5Advanced::Fmcomms5Advanced(iio_context *ctx, QWidget *parent)
	: m_ctx(ctx)
	, QWidget{parent}
{
	m_mainLayout = new QVBoxLayout(this);
	m_mainLayout->setMargin(0);
	m_mainLayout->setContentsMargins(0, 0, 0, 0);

	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->bottomContainer()->setVisible(true);
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

	m_isToolInitialized = false;

	QStackedWidget *centralWidget = new QStackedWidget(this);
	m_tool->addWidgetToCentralContainerHelper(centralWidget);

	QButtonGroup *navigationButtons = new QButtonGroup(this);
	navigationButtons->setExclusive(true);

	if(m_ctx != nullptr) {
		iio_device *mainDevice = nullptr;
		iio_device *secondDevice = nullptr;
		int foundDevices = 0;
		int device_count = iio_context_get_devices_count(m_ctx);
		for(int i = 0; i < device_count; ++i) {
			iio_device *dev = iio_context_get_device(m_ctx, i);
			const char *dev_name = iio_device_get_name(dev);
			if(dev_name && QString(dev_name).compare("ad9361-phy", Qt::CaseInsensitive) == 0) {
				mainDevice = dev;
				foundDevices++;
			}

			if(dev_name && QString(dev_name).compare("ad9361-phy-B", Qt::CaseInsensitive) == 0) {
				secondDevice = dev;
				foundDevices++;
			}

			if(foundDevices == 2) {
				break;
			}
		}
		if(mainDevice == nullptr) {
			qWarning(CAT_FMCOMMS5_ADVANCED) << "No ad9361-phy device found in context!";
			return;
		}

		if(secondDevice == nullptr) {
			qWarning(CAT_FMCOMMS5_ADVANCED) << "No ad9361-phy-B device found in context!";
			return;
		}

		m_mainDevice = mainDevice;
		m_secondDevice = secondDevice;
		m_centralWidget = centralWidget;

		// Create buttons
		m_ensmModeClocksBtn = new QPushButton("ENSM/Mode/Clocks", this);
		Style::setStyle(m_ensmModeClocksBtn, style::properties::button::blueGrayButton);
		m_ensmModeClocksBtn->setCheckable(true);
		m_ensmModeClocksBtn->setChecked(true);
		m_eLnaBtn = new QPushButton("eLNA", this);
		Style::setStyle(m_eLnaBtn, style::properties::button::blueGrayButton);
		m_eLnaBtn->setCheckable(true);
		m_rssiBtn = new QPushButton("RSSI", this);
		Style::setStyle(m_rssiBtn, style::properties::button::blueGrayButton);
		m_rssiBtn->setCheckable(true);
		m_gainBtn = new QPushButton("GAIN", this);
		Style::setStyle(m_gainBtn, style::properties::button::blueGrayButton);
		m_gainBtn->setCheckable(true);
		m_txMonitorBtn = new QPushButton("TX MONITOR", this);
		Style::setStyle(m_txMonitorBtn, style::properties::button::blueGrayButton);
		m_txMonitorBtn->setCheckable(true);
		m_auxAdcDacIioBtn = new QPushButton("Aux ADC/DAC/IIO", this);
		Style::setStyle(m_auxAdcDacIioBtn, style::properties::button::blueGrayButton);
		m_auxAdcDacIioBtn->setCheckable(true);
		m_miscBtn = new QPushButton("MISC", this);
		Style::setStyle(m_miscBtn, style::properties::button::blueGrayButton);
		m_miscBtn->setCheckable(true);
		m_bistBtn = new QPushButton("BIST", this);
		Style::setStyle(m_bistBtn, style::properties::button::blueGrayButton);
		m_bistBtn->setCheckable(true);
		m_fmcomms5Btn = new QPushButton("FMCOMMS5", this);
		Style::setStyle(m_fmcomms5Btn, style::properties::button::blueGrayButton);
		m_fmcomms5Btn->setCheckable(true);

		navigationButtons->addButton(m_ensmModeClocksBtn);
		navigationButtons->addButton(m_eLnaBtn);
		navigationButtons->addButton(m_rssiBtn);
		navigationButtons->addButton(m_gainBtn);
		navigationButtons->addButton(m_txMonitorBtn);
		navigationButtons->addButton(m_auxAdcDacIioBtn);
		navigationButtons->addButton(m_miscBtn);
		navigationButtons->addButton(m_bistBtn);
		navigationButtons->addButton(m_fmcomms5Btn);

		m_tool->addWidgetToTopContainerHelper(m_ensmModeClocksBtn, TTA_LEFT);
		m_tool->addWidgetToTopContainerHelper(m_eLnaBtn, TTA_LEFT);
		m_tool->addWidgetToTopContainerHelper(m_rssiBtn, TTA_LEFT);
		m_tool->addWidgetToTopContainerHelper(m_gainBtn, TTA_LEFT);
		m_tool->addWidgetToTopContainerHelper(m_txMonitorBtn, TTA_LEFT);
		m_tool->addWidgetToTopContainerHelper(m_auxAdcDacIioBtn, TTA_LEFT);
		m_tool->addWidgetToTopContainerHelper(m_miscBtn, TTA_LEFT);
		m_tool->addWidgetToTopContainerHelper(m_bistBtn, TTA_LEFT);
		m_tool->addWidgetToTopContainerHelper(m_fmcomms5Btn, TTA_LEFT);

		m_syncBtn = new QPushButton("MSC Sync", this);
		Style::setStyle(m_syncBtn, style::properties::button::basicButton);
		connect(m_syncBtn, &QPushButton::clicked, this, [=]() {
			// call to lib ad9361
			ad9361_multichip_sync(m_mainDevice, &m_secondDevice, 1,
					      FIXUP_INTERFACE_TIMING | CHECK_SAMPLE_RATES);
		});

		m_tool->addWidgetToBottomContainerHelper(m_syncBtn, TTA_LEFT);

		bool useLazyLoading = scopy::Preferences::get("iiowidgets_use_lazy_loading").toBool();
		if(!useLazyLoading) {
			init();
		}
	}
}

Fmcomms5Advanced::~Fmcomms5Advanced() {}

void Fmcomms5Advanced::showEvent(QShowEvent *event)
{

	if(!m_isToolInitialized) {
		bool useLazyLoading = scopy::Preferences::get("iiowidgets_use_lazy_loading").toBool();
		if(useLazyLoading) {
			init();
		}
	}
	QWidget::showEvent(event);
}

void Fmcomms5Advanced::init()
{

	// ENSM Mode Clocks
	m_ensmModeClocks = new EnsmModeClocksWidget(m_mainDevice, m_centralWidget);
	m_centralWidget->addWidget(m_ensmModeClocks);
	connect(this, &Fmcomms5Advanced::readRequested, m_ensmModeClocks, &EnsmModeClocksWidget::readRequested);
	connect(m_ensmModeClocksBtn, &QPushButton::clicked, this,
		[=, this]() { m_centralWidget->setCurrentWidget(m_ensmModeClocks); });
	// eLNA
	m_elna = new ElnaWidget(m_mainDevice, m_centralWidget);
	connect(this, &Fmcomms5Advanced::readRequested, m_elna, &ElnaWidget::readRequested);
	m_centralWidget->addWidget(m_elna);
	connect(m_eLnaBtn, &QPushButton::clicked, this, [=, this]() { m_centralWidget->setCurrentWidget(m_elna); });
	// RSSI
	m_rssi = new RssiWidget(m_mainDevice, m_centralWidget);
	connect(this, &Fmcomms5Advanced::readRequested, m_rssi, &RssiWidget::readRequested);
	m_centralWidget->addWidget(m_rssi);
	connect(m_rssiBtn, &QPushButton::clicked, this, [=, this]() { m_centralWidget->setCurrentWidget(m_rssi); });
	// GAIN
	m_gainWidget = new GainWidget(m_mainDevice, m_centralWidget);
	connect(this, &Fmcomms5Advanced::readRequested, m_gainWidget, &GainWidget::readRequested);
	m_centralWidget->addWidget(m_gainWidget);
	connect(m_gainBtn, &QPushButton::clicked, this,
		[=, this]() { m_centralWidget->setCurrentWidget(m_gainWidget); });
	// TX MONITOR
	m_txMonitor = new TxMonitorWidget(m_mainDevice, m_centralWidget);
	connect(this, &Fmcomms5Advanced::readRequested, m_txMonitor, &TxMonitorWidget::readRequested);
	m_centralWidget->addWidget(m_txMonitor);
	connect(m_txMonitorBtn, &QPushButton::clicked, this,
		[=, this]() { m_centralWidget->setCurrentWidget(m_txMonitor); });
	// AUX ADC/DAC/IIO
	m_auxAdcDacIo = new AuxAdcDacIoWidget(m_mainDevice, m_centralWidget);
	connect(this, &Fmcomms5Advanced::readRequested, m_auxAdcDacIo, &AuxAdcDacIoWidget::readRequested);
	m_centralWidget->addWidget(m_auxAdcDacIo);
	connect(m_auxAdcDacIioBtn, &QPushButton::clicked, this,
		[=, this]() { m_centralWidget->setCurrentWidget(m_auxAdcDacIo); });
	// MISC
	m_misc = new MiscWidget(m_mainDevice, m_centralWidget);
	connect(this, &Fmcomms5Advanced::readRequested, m_misc, &MiscWidget::readRequested);
	m_centralWidget->addWidget(m_misc);
	connect(m_miscBtn, &QPushButton::clicked, this, [=, this]() { m_centralWidget->setCurrentWidget(m_misc); });
	// BIST
	m_bist = new BistWidget(m_mainDevice, m_centralWidget);
	connect(this, &Fmcomms5Advanced::readRequested, m_bist, &BistWidget::readRequested);
	m_centralWidget->addWidget(m_bist);
	connect(m_bistBtn, &QPushButton::clicked, this, [=, this]() { m_centralWidget->setCurrentWidget(m_bist); });

	// FMCOMMS5
	m_fmcomms5 = new Fmcomms5Tab(m_ctx, m_centralWidget);
	connect(this, &Fmcomms5Advanced::readRequested, m_fmcomms5, &Fmcomms5Tab::readRequested);
	m_centralWidget->addWidget(m_fmcomms5);
	connect(m_fmcomms5Btn, &QPushButton::clicked, this,
		[=, this]() { m_centralWidget->setCurrentWidget(m_fmcomms5); });

	m_isToolInitialized = true;
}

void Fmcomms5Advanced::ad9361MultichipSync() {}
