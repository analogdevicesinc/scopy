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

#include "fmcomms5/fmcomms5.h"

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

Q_LOGGING_CATEGORY(CAT_FMCOMMS5, "FMCOMMS5");

using namespace scopy;
using namespace ad936x;

FMCOMMS5::FMCOMMS5(iio_context *ctx, QWidget *parent)
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

		iio_device *mainDevice = nullptr;
		int device_count = iio_context_get_devices_count(m_ctx);
		for(int i = 0; i < device_count; ++i) {
			iio_device *dev = iio_context_get_device(m_ctx, i);
			const char *dev_name = iio_device_get_name(dev);
			if(dev_name && QString(dev_name).compare("ad9361-phy", Qt::CaseInsensitive) == 0) {
				mainDevice = dev;
				break;
			}
		}

		m_helper = new AD936xHelper();
		connect(this, &FMCOMMS5::readRequested, m_helper, &AD936xHelper::readRequested);

		///  first widget the global settings can be created with iiowigets only
		controlWidgetLayout->addWidget(
			m_helper->generateGlobalSettingsWidget(mainDevice, "FMCOMMS5 Global Settings", controlsWidget));

		/// second is Rx ( receive chain)
		controlWidgetLayout->addWidget(
			generateRxChainWidget(mainDevice, "FMCOMMS5 Receive Chain", controlsWidget));

		/// third is Tx (transmit chain)
		controlWidgetLayout->addWidget(
			generateTxChainWidget(mainDevice, "FMCOMMS5 Transmit Chain", controlsWidget));

		controlWidgetLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));
	}

	// Block Diagram Stack Widget with Next/Prev buttons
	QWidget *blockDiagramStackWidget = new QWidget(this);
	Style::setBackgroundColor(blockDiagramStackWidget, json::theme::background_primary);
	QVBoxLayout *blockDiagramStackLayout = new QVBoxLayout(blockDiagramStackWidget);
	blockDiagramStackWidget->setLayout(blockDiagramStackLayout);

	QStackedWidget *imageStack = new QStackedWidget(blockDiagramStackWidget);

	// Add images as QLabel widgets
	QLabel *imageLabel1 = new QLabel(blockDiagramStackWidget);
	imageLabel1->setAlignment(Qt::AlignCenter);
	imageLabel1->setPixmap(QPixmap(":/pluto/ad936x.svg"));
	imageStack->addWidget(imageLabel1);

	QLabel *imageLabel2 = new QLabel(blockDiagramStackWidget);
	imageLabel2->setAlignment(Qt::AlignCenter);
	imageLabel2->setPixmap(QPixmap(":/pluto/AD_FMCOMMS5_EBZ.jpg"));
	imageStack->addWidget(imageLabel2);

	blockDiagramStackLayout->addWidget(imageStack);

	QHBoxLayout *buttonLayout = new QHBoxLayout();
	QPushButton *prevBtn = new QPushButton("Prev", blockDiagramStackWidget);
	Style::setStyle(prevBtn, style::properties::button::basicButton);
	QPushButton *nextBtn = new QPushButton("Next", blockDiagramStackWidget);
	Style::setStyle(nextBtn, style::properties::button::basicButton);
	buttonLayout->addWidget(prevBtn);
	buttonLayout->addWidget(nextBtn);
	blockDiagramStackLayout->addLayout(buttonLayout);

	// Navigation logic
	connect(prevBtn, &QPushButton::clicked, this, [imageStack]() {
		int idx = imageStack->currentIndex();
		if(idx > 0)
			imageStack->setCurrentIndex(idx - 1);
	});
	connect(nextBtn, &QPushButton::clicked, this, [imageStack]() {
		int idx = imageStack->currentIndex();
		if(idx < imageStack->count() - 1)
			imageStack->setCurrentIndex(idx + 1);
	});

	centralWidget->addWidget(m_controlsWidget);
	centralWidget->addWidget(blockDiagramStackWidget);

	m_tool->addWidgetToCentralContainerHelper(centralWidget);

	QButtonGroup *centralWidgetButtons = new QButtonGroup(this);
	centralWidgetButtons->setExclusive(true);

	QPushButton *controlsBtn = new QPushButton("Controls", this);
	controlsBtn->setCheckable(true);
	controlsBtn->setChecked(true);
	Style::setStyle(controlsBtn, style::properties::button::blueGrayButton);
	connect(controlsBtn, &QPushButton::clicked, this,
		[=, this]() { centralWidget->setCurrentWidget(m_controlsWidget); });

	QPushButton *blockDiagramBtn = new QPushButton("Block Diagram", this);
	blockDiagramBtn->setCheckable(true);
	Style::setStyle(blockDiagramBtn, style::properties::button::blueGrayButton);
	connect(blockDiagramBtn, &QPushButton::clicked, this,
		[=, this]() { centralWidget->setCurrentWidget(blockDiagramStackWidget); });

	centralWidgetButtons->addButton(controlsBtn);
	centralWidgetButtons->addButton(blockDiagramBtn);

	m_tool->addWidgetToTopContainerHelper(controlsBtn, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(blockDiagramBtn, TTA_LEFT);
}

FMCOMMS5::~FMCOMMS5() {}

QWidget *FMCOMMS5::generateRxChainWidget(iio_device *dev, QString title, QWidget *parent)
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
		qWarning(CAT_FMCOMMS5) << "No FMCOMMS5 device found";
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
	connect(this, &FMCOMMS5::readRequested, rfBandwidth, &IIOWidget::readAsync);

	// voltage0:  sampling_frequency
	IIOWidget *samplingFrequency = IIOWidgetBuilder(widget)
					       .channel(voltage0)
					       .attribute("sampling_frequency")
					       .optionsAttribute("sampling_frequency_available")
					       .title("Sampling Rate(MSPS)")
					       .uiStrategy(IIOWidgetBuilder::RangeUi)
					       .buildSingle();
	layout->addWidget(samplingFrequency, 0, 1, 2, 1);
	connect(this, &FMCOMMS5::readRequested, samplingFrequency, &IIOWidget::readAsync);

	// voltage 0 : rf_port_select
	IIOWidget *rfPortSelect = IIOWidgetBuilder(widget)
					  .channel(voltage0)
					  .attribute("rf_port_select")
					  .optionsAttribute("rf_port_select_available")
					  .title("RF Port Select")
					  .uiStrategy(IIOWidgetBuilder::ComboUi)
					  .buildSingle();
	layout->addWidget(rfPortSelect, 0, 2, 2, 1);
	connect(this, &FMCOMMS5::readRequested, rfPortSelect, &IIOWidget::readAsync);

	// quadrature_tracking_en
	IIOWidget *quadratureTrackingEn = IIOWidgetBuilder(this)
						  .channel(voltage0)
						  .attribute("quadrature_tracking_en")
						  .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						  .title("Quadrature")
						  .buildSingle();
	layout->addWidget(quadratureTrackingEn, 0, 5);
	quadratureTrackingEn->showProgressBar(false);
	connect(this, &FMCOMMS5::readRequested, quadratureTrackingEn, &IIOWidget::readAsync);

	// rf_dc_offset_tracking_en
	IIOWidget *rcDcOffsetTrackingEn = IIOWidgetBuilder(widget)
						  .channel(voltage0)
						  .attribute("rf_dc_offset_tracking_en")
						  .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						  .title("RF DC")
						  .buildSingle();
	layout->addWidget(rcDcOffsetTrackingEn, 1, 5);
	rcDcOffsetTrackingEn->showProgressBar(false);
	connect(this, &FMCOMMS5::readRequested, rcDcOffsetTrackingEn, &IIOWidget::readAsync);

	// bb_dc_offset_tracking_en
	IIOWidget *bbDcOffsetTrackingEn = IIOWidgetBuilder(widget)
						  .channel(voltage0)
						  .attribute("bb_dc_offset_tracking_en")
						  .title("BB DC")
						  .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						  .buildSingle();
	layout->addWidget(bbDcOffsetTrackingEn, 2, 5);
	bbDcOffsetTrackingEn->showProgressBar(false);
	connect(this, &FMCOMMS5::readRequested, bbDcOffsetTrackingEn, &IIOWidget::readAsync);

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

	//////// second device for fmcomms5 RX

	iio_device *dev2 = nullptr;
	int device_count = iio_context_get_devices_count(m_ctx);
	for(int i = 0; i < device_count; ++i) {
		iio_device *aux = iio_context_get_device(m_ctx, i);
		const char *dev_name = iio_device_get_name(aux);
		if(dev_name && QString(dev_name).contains("ad9361-phy-B", Qt::CaseInsensitive)) {
			dev2 = aux;
			break;
		}
	}

	QWidget *rxDevice2Widget = m_helper->generateRxDeviceWidget(dev2, "ad9361-phy-B", widget);

	rxDeviceLayout->addWidget(rxDevice2Widget);

	iio_channel *voltage0B = iio_device_find_channel(dev2, "voltage0", isOutput);

	rxDevice2Widget->layout()->addWidget(m_helper->generateRxChannelWidget(voltage0B, "RX 3", rxDevice2Widget));
	iio_channel *voltage1B = iio_device_find_channel(dev2, "voltage1", isOutput);
	if(voltage1B && iio_channel_find_attr(voltage1B, "hardwaregain")) {
		rxDevice2Widget->layout()->addWidget(
			m_helper->generateRxChannelWidget(voltage1B, "RX 4", rxDevice2Widget));
	}

	/////////////////////////

	mainLayout->addLayout(rxDeviceLayout);
	mainLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	return widget;
}

QWidget *FMCOMMS5::generateTxChainWidget(iio_device *dev, QString title, QWidget *parent)
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
	connect(this, &FMCOMMS5::readRequested, rfBandwidth, &IIOWidget::readAsync);

	// voltage0:  sampling_frequency
	IIOWidget *samplingFrequency = IIOWidgetBuilder(widget)
					       .channel(voltage0)
					       .attribute("sampling_frequency")
					       .optionsAttribute("sampling_frequency_available")
					       .uiStrategy(IIOWidgetBuilder::RangeUi)
					       .title("Sampling Rate(MSPS)")
					       .buildSingle();
	lay->addWidget(samplingFrequency, 0, 1, 2, 1);
	connect(this, &FMCOMMS5::readRequested, samplingFrequency, &IIOWidget::readAsync);

	// voltage0:  rf_port_select
	IIOWidget *rfPortSelect = IIOWidgetBuilder(widget)
					  .channel(voltage0)
					  .attribute("rf_port_select")
					  .optionsAttribute("rf_port_select_available")
					  .uiStrategy(IIOWidgetBuilder::ComboUi)
					  .title("RF Port Select")
					  .buildSingle();
	lay->addWidget(rfPortSelect, 0, 2, 2, 1);
	connect(this, &FMCOMMS5::readRequested, rfPortSelect, &IIOWidget::readAsync);

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

	//////// second device for fmcomms5 TX

	iio_device *dev2 = nullptr;
	int device_count = iio_context_get_devices_count(m_ctx);
	for(int i = 0; i < device_count; ++i) {
		iio_device *aux = iio_context_get_device(m_ctx, i);
		const char *dev_name = iio_device_get_name(aux);
		if(dev_name && QString(dev_name).contains("ad9361-phy-b", Qt::CaseInsensitive)) {
			dev2 = aux;
			break;
		}
	}

	QWidget *txDevice2Widget = m_helper->generateTxDeviceWidget(dev2, "ad9361-phy-B", widget);

	txWidgetsLayout->addWidget(txDevice2Widget);

	iio_channel *voltage0B = iio_device_find_channel(dev2, "voltage0", isOutput);

	txDevice2Widget->layout()->addWidget(m_helper->generateTxChannelWidget(voltage0B, "TX 3", txDevice2Widget));
	iio_channel *voltage1B = iio_device_find_channel(dev2, "voltage1", isOutput);
	if(voltage1 && iio_channel_find_attr(voltage1B, "hardwaregain")) {
		txDevice2Widget->layout()->addWidget(
			m_helper->generateTxChannelWidget(voltage1B, "TX 4", txDevice2Widget));
	}

	/////////////////////////

	layout->addLayout(txWidgetsLayout);
	layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	return widget;
}
