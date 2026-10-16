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

#include "cn0511.h"

#include <QLabel>
#include <QScrollArea>
#include <QGridLayout>
#include <QLoggingCategory>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <style.h>
#include <iio-widgets/iiowidgetgroup.h>

#include <component/attribute.h>
#include <component/attributereader.h>
#include <component/channel.h>
#include <component/context.h>
#include <component/device.h>
#include <component/navigation.h>
#include <component/backends/iio/iiochannel.h>
#include <component/backends/iio/iiocontext.h>
#include <component/backends/iio/iiodevice.h>

#include <iio.h>
#include <ad9166.h>

Q_LOGGING_CATEGORY(CAT_CN0511, "CN0511");

using namespace scopy;
using namespace cn0511;

static const double MHZ_SCALE = 1000000.0;

CN0511::CN0511(component::Context *ctx, IIOWidgetGroup *group, QWidget *parent)
	: QWidget(parent)
	, m_ctx(ctx)
	, m_group(group)
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->topContainerMenuControl()->setVisible(false);

	mainLayout->addWidget(m_tool);

	m_refreshButton = new AnimatedRefreshBtn(false, this);
	m_tool->addWidgetToTopContainerHelper(m_refreshButton, TTA_RIGHT);

	connect(m_refreshButton, &QPushButton::clicked, this, &CN0511::refresh);

	// Central scroll area
	QWidget *controlsContainer = new QWidget(this);
	QVBoxLayout *controlsLayout = new QVBoxLayout(controlsContainer);
	controlsLayout->setContentsMargins(0, 0, 0, 0);

	QWidget *controlsWidget = new QWidget(this);
	QVBoxLayout *controlWidgetLayout = new QVBoxLayout(controlsWidget);
	controlWidgetLayout->setContentsMargins(0, 0, 0, 0);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(controlsWidget);

	controlsLayout->addWidget(scrollArea);

	if(m_ctx != nullptr) {
		m_dac = m_ctx->findChild<component::Device *>("ad9166", Qt::FindDirectChildrenOnly);
		if(m_dac) {
			// An input and an output channel can share the id "altvoltage0", so the
			// direction has to be part of the lookup.
			m_dacCh = component::channelById(m_dac, "altvoltage0", true);
		}

		controlWidgetLayout->addWidget(generateSingleToneSection(controlsWidget));
		controlWidgetLayout->addWidget(generateDacAmpSection(controlsWidget));
		controlWidgetLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));
	}

	m_tool->addWidgetToCentralContainerHelper(controlsContainer);
}

CN0511::~CN0511()
{
	// A refresh in flight holds "this"; let it settle before the members go away.
	if(!m_refreshTask.isReady()) {
		QCoro::waitFor(m_refreshTask);
	}
	m_refreshTask = {};

	if(m_calibFuture.isRunning()) {
		m_calibFuture.waitForFinished();
	}
}

QWidget *CN0511::generateSingleToneSection(QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(widget);
	widget->setLayout(layout);
	layout->setContentsMargins(10, 10, 10, 10);
	layout->setSpacing(10);

	QLabel *titleLabel = new QLabel("Single Tone Mode", widget);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	layout->addWidget(titleLabel);

	QHBoxLayout *controlsLayout = new QHBoxLayout();
	controlsLayout->setContentsMargins(0, 0, 0, 0);

	QVBoxLayout *configLayout = new QVBoxLayout();
	configLayout->setContentsMargins(0, 0, 0, 0);
	configLayout->setSpacing(10);

	// Frequency (MHz) - MenuSpinbox: 0-10000, default 4500
	m_freqSpinBox = new gui::MenuSpinbox("Frequency (MHz)", 0.0, "", 0.0, 10000.0, false, false, true, widget);
	m_freqSpinBox->setScalingEnabled(false);
	m_freqSpinBox->setIncrementMode(gui::MenuSpinbox::IS_FIXED);
	m_freqSpinBox->incrementStrategy()->setScale(1.0);
	m_freqSpinBox->setValue(4500);
	configLayout->addWidget(m_freqSpinBox);

	connect(m_freqSpinBox, &gui::MenuSpinbox::valueChanged, this,
		[this](double val) { m_calibFreq = val * MHZ_SCALE; });

	// Amplitude (dBm) - MenuSpinbox: -91 to 0, default 0
	m_ampSpinBox = new gui::MenuSpinbox("Amplitude (dBm)", 0.0, "", -91.0, 0.0, false, false, true, widget);
	m_ampSpinBox->setScaleRange(1.0, 1.0); // Force combobox to have unit scale (empty otherwise since max=0)
	m_ampSpinBox->setScalingEnabled(false);
	m_ampSpinBox->setIncrementMode(gui::MenuSpinbox::IS_FIXED);
	configLayout->addWidget(m_ampSpinBox);

	connect(m_ampSpinBox, &gui::MenuSpinbox::valueChanged, this, [this](double val) { m_amplitude = (int)val; });

	// Enter button - triggers calibration with loading animation
	m_enterBtn = new AnimatedLoadingButton("Enter", widget);
	connect(m_enterBtn, &QPushButton::clicked, this, &CN0511::applyCalibration);

	controlsLayout->addLayout(configLayout, 1);
	controlsLayout->addWidget(m_enterBtn, 1);

	layout->addLayout(controlsLayout);

	return widget;
}

QWidget *CN0511::generateDacAmpSection(QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(widget);
	widget->setLayout(layout);
	layout->setContentsMargins(10, 10, 10, 10);
	layout->setSpacing(10);

	QLabel *titleLabel = new QLabel("DAC Amplifier", widget);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	layout->addWidget(titleLabel);

	component::Device *dacAmp = m_ctx->findChild<component::Device *>("ad9166-amp", Qt::FindDirectChildrenOnly);
	if(dacAmp) {
		m_dacAmpEnWidget = IIOWidgetBuilder(widget)
					   .attribute(component::attributeByName(dacAmp, "en"))
					   .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					   .title("Enable")
					   .group(m_group)
					   .buildSingle();

		if(m_dacAmpEnWidget) {
			m_dacAmpEnWidget->showProgressBar(false);
			layout->addWidget(m_dacAmpEnWidget);
		}
	}

	return widget;
}

void CN0511::applyCalibration()
{
	if(!m_ctx || !m_dac || !m_dacCh) {
		qWarning(CAT_CN0511) << "Cannot apply calibration: device not available";
		return;
	}

	// libad9166 needs raw libiio pointers; recover them from the device-controller
	// nodes' backend handles (pragmatic hybrid).
	auto *iioCtx = dynamic_cast<component::iio::IIOContext *>(m_ctx);
	auto *iioDac = dynamic_cast<component::iio::IIODevice *>(m_dac);
	auto *iioDacCh = dynamic_cast<component::iio::IIOChannel *>(m_dacCh);
	if(!iioCtx || !iioDac || !iioDacCh) {
		qWarning(CAT_CN0511) << "Cannot apply calibration: not an IIO context";
		return;
	}

	iio_context *rawCtx = static_cast<iio_context *>(iioCtx->handle().ptr);
	iio_device *rawDac = static_cast<iio_device *>(iioDac->handle().ptr);
	iio_channel *rawDacCh = static_cast<iio_channel *>(iioDacCh->handle().ptr);
	if(!rawCtx || !rawDac || !rawDacCh) {
		qWarning(CAT_CN0511) << "Cannot apply calibration: no raw libiio handles available";
		return;
	}

	m_enterBtn->startAnimation();
	m_refreshButton->setEnabled(false);

	double calibFreq = m_calibFreq;
	int amplitude = m_amplitude;

	QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
	connect(
		watcher, &QFutureWatcher<void>::finished, this,
		[this, watcher]() {
			m_enterBtn->stopAnimation();
			m_refreshButton->setEnabled(true);
			watcher->deleteLater();
		},
		Qt::QueuedConnection);

	m_calibFuture = QtConcurrent::run([rawCtx, rawDac, rawDacCh, calibFreq, amplitude]() {
		qDebug(CAT_CN0511) << "Applying calibration: freq=" << calibFreq << " amplitude=" << amplitude;

		struct ad9166_calibration_data *calibData = nullptr;
		int ret = ad9166_context_find_calibration_data(rawCtx, "cn0511", &calibData);
		if(ret < 0) {
			qWarning(CAT_CN0511) << "Failed to find calibration data. Error:" << ret;
			return;
		}

		ret = ad9166_channel_set_freq(rawDacCh, calibFreq);
		if(ret < 0) {
			qWarning(CAT_CN0511) << "Failed to set frequency. Error:" << ret;
		}

		ret = ad9166_device_set_amplitude(rawDac, amplitude);
		if(ret < 0) {
			qWarning(CAT_CN0511) << "Failed to set amplitude. Error:" << ret;
		}

		ret = ad9166_device_set_iofs(rawDac, calibData, calibFreq);
		if(ret < 0) {
			qWarning(CAT_CN0511) << "Failed to set IOFS. Error:" << ret;
		}
	});

	watcher->setFuture(m_calibFuture);
}

void CN0511::refresh()
{
	// Guard against overlapping refreshes: skip if the previous one is still in flight.
	if(!m_refreshTask.isReady()) {
		return;
	}

	if(m_dacAmpEnWidget) {
		m_dacAmpEnWidget->readAsync();
	}

	m_enterBtn->setEnabled(false);
	m_refreshButton->startAnimation();
	m_refreshTask = readFrequencyFromDevice();
}

QCoro::Task<void> CN0511::readFrequencyFromDevice()
{
	if(!m_dacCh || !m_freqSpinBox) {
		m_refreshButton->stopAnimation();
		m_enterBtn->setEnabled(true);
		co_return;
	}

	component::Attribute *attr = component::attributeByName(m_dacCh, "nco_frequency");
	if(!attr || !attr->readCapability()) {
		qWarning(CAT_CN0511) << "nco_frequency is not readable";
		m_refreshButton->stopAnimation();
		m_enterBtn->setEnabled(true);
		co_return;
	}

	auto response = co_await attr->readCapability()->readAsync();
	if(!response) {
		qWarning(CAT_CN0511) << "Failed to read nco_frequency. Error:" << response.error().message;
		m_refreshButton->stopAnimation();
		m_enterBtn->setEnabled(true);
		co_return;
	}

	bool ok = false;
	double val = QString::fromUtf8(response.value()).trimmed().toDouble(&ok);
	if(!ok) {
		qWarning(CAT_CN0511) << "Could not parse nco_frequency:" << response.value();
		m_refreshButton->stopAnimation();
		m_enterBtn->setEnabled(true);
		co_return;
	}

	m_freqSpinBox->setValue(val / MHZ_SCALE);
	m_refreshButton->stopAnimation();
	m_enterBtn->setEnabled(true);
}
