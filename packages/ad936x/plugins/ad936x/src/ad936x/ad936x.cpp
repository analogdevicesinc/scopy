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

#include "ad936x/ad936x.h"

#include <QLabel>
#include <QTabWidget>
#include <menucombo.h>
#include <toolbuttons.h>
#include <utils.h>
#include <QList>
#include <style.h>
#include <menuonoffswitch.h>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QLoggingCategory>

#include <guistrategy/comboguistrategy.h>

Q_LOGGING_CATEGORY(CAT_AD936X, "AD936X");

using namespace scopy;
using namespace ad936x;

AD936X::AD936X(iio_context *ctx, QWidget *parent)
	: QWidget(parent)
	, m_ctx(ctx)
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

	QStackedWidget *centralWidget = new QStackedWidget(this);

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
		// this should work for any device from AD936x family
		iio_device *plutoDevice = nullptr;
		int device_count = iio_context_get_devices_count(m_ctx);
		for(int i = 0; i < device_count; ++i) {
			iio_device *dev = iio_context_get_device(m_ctx, i);
			const char *dev_name = iio_device_get_name(dev);
			if(dev_name && QString(dev_name).contains("ad936", Qt::CaseInsensitive)) {
				plutoDevice = dev;
				break;
			}
		}

		m_helper = new AD936xHelper();
		connect(this, &AD936X::readRequested, m_helper, &AD936xHelper::readRequested);

		///  first widget the global settings can be created with iiowigets only
		controlWidgetLayout->addWidget(m_helper->generateGlobalSettingsWidget(
			plutoDevice, "AD9361 / AD9364 Global Settings", controlsWidget));

		/// second is Rx ( receive chain)
		controlWidgetLayout->addWidget(
			generateRxChainWidget(plutoDevice, "AD9361 / AD9364 Receive Chain", controlsWidget));

		/// third is Tx (transmit chain)
		controlWidgetLayout->addWidget(
			generateTxChainWidget(plutoDevice, "AD9361 / AD9364 Transmit Chain", controlsWidget));

		controlWidgetLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));
	}

	m_blockDiagramWidget = new QWidget(this);
	Style::setBackgroundColor(m_blockDiagramWidget, json::theme::background_primary);
	QVBoxLayout *blockDiagramLayout = new QVBoxLayout(m_blockDiagramWidget);
	m_blockDiagramWidget->setLayout(blockDiagramLayout);

	QWidget *blockDiagramWidget = new QWidget(this);
	QVBoxLayout *blockDiagramWidgetLayout = new QVBoxLayout(blockDiagramWidget);
	blockDiagramWidget->setLayout(blockDiagramWidgetLayout);

	QScrollArea *blockDiagramWidgetScrollArea = new QScrollArea(this);
	blockDiagramWidgetScrollArea->setWidgetResizable(true);
	blockDiagramWidgetScrollArea->setWidget(blockDiagramWidget);

	blockDiagramLayout->addWidget(blockDiagramWidgetScrollArea);

	QLabel *blockDiagram = new QLabel(m_blockDiagramWidget);
	blockDiagramWidgetLayout->addWidget(blockDiagram);
	blockDiagram->setAlignment(Qt::AlignCenter);
	QPixmap pixmap(":/pluto/ad936x.svg");
	blockDiagram->setPixmap(pixmap);

	centralWidget->addWidget(m_controlsWidget);
	centralWidget->addWidget(m_blockDiagramWidget);

	m_tool->addWidgetToCentralContainerHelper(centralWidget);

	QButtonGroup *centralWidgetButtons = new QButtonGroup(this);
	centralWidgetButtons->setExclusive(true);

	QPushButton *ad963xBtn = new QPushButton("Controls", this);
	ad963xBtn->setCheckable(true);
	ad963xBtn->setChecked(true);
	Style::setStyle(ad963xBtn, style::properties::button::blueGrayButton);
	connect(ad963xBtn, &QPushButton::clicked, this,
		[=, this]() { centralWidget->setCurrentWidget(m_controlsWidget); });

	QPushButton *blockDiagramBtn = new QPushButton("Block Diagram", this);
	blockDiagramBtn->setCheckable(true);
	Style::setStyle(blockDiagramBtn, style::properties::button::blueGrayButton);
	connect(blockDiagramBtn, &QPushButton::clicked, this,
		[=, this]() { centralWidget->setCurrentWidget(m_blockDiagramWidget); });

	centralWidgetButtons->addButton(ad963xBtn);
	centralWidgetButtons->addButton(blockDiagramBtn);

	m_tool->addWidgetToTopContainerHelper(ad963xBtn, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(blockDiagramBtn, TTA_LEFT);
}

AD936X::~AD936X() {}

QWidget *AD936X::generateRxChainWidget(iio_device *dev, QString title, QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *mainLayout = new QVBoxLayout(widget);
	widget->setLayout(mainLayout);

	QLabel *titleLabel = new QLabel(title, widget);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	mainLayout->addWidget(titleLabel);

	if(dev == nullptr) {
		qWarning(CAT_AD936X) << "No AD936X device found";
	}

	QGridLayout *layout = new QGridLayout();

	bool isOutput = false;

	iio_channel *voltage0 = iio_device_find_channel(dev, "voltage0", isOutput);

	// voltage0: rf_bandwidth
	IIOWidget *rfBandwidth = IIOWidgetBuilder(widget)
					 .channel(voltage0)
					 .attribute("rf_bandwidth")
					 .optionsAttribute("rf_bandwidth_available")
					 .title("RF Bandwidth(MHz)")
					 .uiStrategy(IIOWidgetBuilder::RangeUi)
					 .buildSingle();
	layout->addWidget(rfBandwidth, 0, 0, 2, 1);
	connect(this, &AD936X::readRequested, rfBandwidth, &IIOWidget::readAsync);

	// voltage0:  sampling_frequency
	IIOWidget *samplingFrequency = IIOWidgetBuilder(widget)
					       .channel(voltage0)
					       .attribute("sampling_frequency")
					       .optionsAttribute("sampling_frequency_available")
					       .title("Sampling Rate(MSPS)")
					       .uiStrategy(IIOWidgetBuilder::RangeUi)
					       .buildSingle();
	layout->addWidget(samplingFrequency, 0, 1, 2, 1);
	connect(this, &AD936X::readRequested, samplingFrequency, &IIOWidget::readAsync);

	// voltage 0 : rf_port_select
	IIOWidget *rfPortSelect = IIOWidgetBuilder(widget)
					  .channel(voltage0)
					  .attribute("rf_port_select")
					  .optionsAttribute("rf_port_select_available")
					  .title("RF Port Select")
					  .uiStrategy(IIOWidgetBuilder::ComboUi)
					  .buildSingle();
	layout->addWidget(rfPortSelect, 0, 2, 2, 1);
	connect(this, &AD936X::readRequested, rfPortSelect, &IIOWidget::readAsync);

	// quadrature_tracking_en
	IIOWidget *quadratureTrackingEn = IIOWidgetBuilder(this)
						  .channel(voltage0)
						  .attribute("quadrature_tracking_en")
						  .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						  .title("Quadrature")
						  .buildSingle();
	layout->addWidget(quadratureTrackingEn, 0, 5);
	quadratureTrackingEn->showProgressBar(false);
	connect(this, &AD936X::readRequested, quadratureTrackingEn, &IIOWidget::readAsync);

	// rf_dc_offset_tracking_en
	IIOWidget *rcDcOffsetTrackingEn = IIOWidgetBuilder(widget)
						  .channel(voltage0)
						  .attribute("rf_dc_offset_tracking_en")
						  .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						  .title("RF DC")
						  .buildSingle();
	layout->addWidget(rcDcOffsetTrackingEn, 1, 5);
	rcDcOffsetTrackingEn->showProgressBar(false);
	connect(this, &AD936X::readRequested, rcDcOffsetTrackingEn, &IIOWidget::readAsync);

	// bb_dc_offset_tracking_en
	IIOWidget *bbDcOffsetTrackingEn = IIOWidgetBuilder(widget)
						  .channel(voltage0)
						  .attribute("bb_dc_offset_tracking_en")
						  .title("BB DC")
						  .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						  .buildSingle();
	layout->addWidget(bbDcOffsetTrackingEn, 2, 5);
	bbDcOffsetTrackingEn->showProgressBar(false);
	connect(this, &AD936X::readRequested, bbDcOffsetTrackingEn, &IIOWidget::readAsync);

	mainLayout->addLayout(layout);

	QHBoxLayout *rxDeviceLayout = new QHBoxLayout();
	rxDeviceLayout->setMargin(0);
	rxDeviceLayout->setSpacing(10);

	QWidget *rxDeviceWidget = m_helper->generateRxDeviceWidget(dev, "ad9361-phy", widget);

	rxDeviceLayout->addWidget(rxDeviceWidget);
	rxDeviceWidget->layout()->addWidget(m_helper->generateRxChannelWidget(voltage0, "RX 1", rxDeviceWidget));
	iio_channel *voltage1 = iio_device_find_channel(dev, "voltage1", isOutput);
	if(voltage1 && iio_channel_find_attr(voltage1, "hardwaregain")) {
		rxDeviceWidget->layout()->addWidget(
			m_helper->generateRxChannelWidget(voltage1, "RX 2", rxDeviceWidget));
	}

	rxDeviceLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
	mainLayout->addLayout(rxDeviceLayout);
	mainLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	return widget;
}

QWidget *AD936X::generateTxChainWidget(iio_device *dev, QString title, QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(widget);
	widget->setLayout(layout);

	QLabel *titleLabel = new QLabel(title, widget);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	layout->addWidget(titleLabel);

	QGridLayout *lay = new QGridLayout();

	bool isOutput = true;
	iio_channel *voltage0 = iio_device_find_channel(dev, "voltage0", isOutput);

	// voltage0: rf_bandwidth
	IIOWidget *rfBandwidth = IIOWidgetBuilder(widget)
					 .channel(voltage0)
					 .attribute("rf_bandwidth")
					 .optionsAttribute("rf_bandwidth_available")
					 .uiStrategy(IIOWidgetBuilder::RangeUi)
					 .title("RF Bandwidth(MHz)")
					 .buildSingle();
	lay->addWidget(rfBandwidth, 0, 0, 2, 1);
	connect(this, &AD936X::readRequested, rfBandwidth, &IIOWidget::readAsync);

	// voltage0:  sampling_frequency
	IIOWidget *samplingFrequency = IIOWidgetBuilder(widget)
					       .channel(voltage0)
					       .attribute("sampling_frequency")
					       .optionsAttribute("sampling_frequency_available")
					       .uiStrategy(IIOWidgetBuilder::RangeUi)
					       .title("Sampling Rate(MSPS)")
					       .buildSingle();
	lay->addWidget(samplingFrequency, 0, 1, 2, 1);
	connect(this, &AD936X::readRequested, samplingFrequency, &IIOWidget::readAsync);

	// voltage0:  rf_port_select
	IIOWidget *rfPortSelect = IIOWidgetBuilder(widget)
					  .channel(voltage0)
					  .attribute("rf_port_select")
					  .optionsAttribute("rf_port_select_available")
					  .uiStrategy(IIOWidgetBuilder::ComboUi)
					  .title("RF Port Select")
					  .buildSingle();
	lay->addWidget(rfPortSelect, 0, 2, 2, 1);
	connect(this, &AD936X::readRequested, rfPortSelect, &IIOWidget::readAsync);

	layout->addLayout(lay);

	QHBoxLayout *txWidgetsLayout = new QHBoxLayout();
	txWidgetsLayout->setMargin(0);
	txWidgetsLayout->setSpacing(10);

	QWidget *txDeviceWidget = m_helper->generateTxDeviceWidget(dev, "ad9361-phy", widget);

	txWidgetsLayout->addWidget(txDeviceWidget);
	txDeviceWidget->layout()->addWidget(m_helper->generateTxChannelWidget(voltage0, "TX 1", txDeviceWidget));
	iio_channel *voltage1 = iio_device_find_channel(dev, "voltage1", isOutput);
	if(voltage1 && iio_channel_find_attr(voltage1, "hardwaregain")) {
		txDeviceWidget->layout()->addWidget(
			m_helper->generateTxChannelWidget(voltage1, "TX 2", txDeviceWidget));
	}

	txWidgetsLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Preferred));
	layout->addLayout(txWidgetsLayout);
	layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	return widget;
}
