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

#include <iio.h>
#include <ad9166.h>

Q_LOGGING_CATEGORY(CAT_CN0511, "CN0511");

using namespace scopy;
using namespace cn0511;

static const double MHZ_SCALE = 1000000.0;

CN0511::CN0511(iio_context *ctx, IIOWidgetGroup *group, QWidget *parent)
	: QWidget(parent)
	, m_ctx(ctx)
	, m_group(group)
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(0);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->topContainerMenuControl()->setVisible(false);

	mainLayout->addWidget(m_tool);

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

	// Central scroll area
	QWidget *controlsContainer = new QWidget(this);
	QVBoxLayout *controlsLayout = new QVBoxLayout(controlsContainer);
	controlsLayout->setMargin(0);
	controlsLayout->setContentsMargins(0, 0, 0, 0);

	QWidget *controlsWidget = new QWidget(this);
	QVBoxLayout *controlWidgetLayout = new QVBoxLayout(controlsWidget);
	controlWidgetLayout->setMargin(0);
	controlWidgetLayout->setContentsMargins(0, 0, 0, 0);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(controlsWidget);

	controlsLayout->addWidget(scrollArea);

	if(m_ctx != nullptr) {
		m_dac = iio_context_find_device(m_ctx, "ad9166");
		if(m_dac) {
			m_dacCh = iio_device_find_channel(m_dac, "altvoltage0", true);
		}

		controlWidgetLayout->addWidget(generateSingleToneSection(controlsWidget));
		controlWidgetLayout->addWidget(generateDacAmpSection(controlsWidget));
		controlWidgetLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

		connect(this, &CN0511::readRequested, this, &CN0511::readFrequencyFromDevice);
	}

	m_tool->addWidgetToCentralContainerHelper(controlsContainer);
}

CN0511::~CN0511() {}

QWidget *CN0511::generateSingleToneSection(QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(widget);
	widget->setLayout(layout);
	layout->setMargin(0);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(10);

	QLabel *titleLabel = new QLabel("Single Tone Mode", widget);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	layout->addWidget(titleLabel);

	QHBoxLayout *controlsLayout = new QHBoxLayout();
	controlsLayout->setMargin(0);
	controlsLayout->setContentsMargins(0, 0, 0, 0);

	QVBoxLayout *configLayout = new QVBoxLayout();
	configLayout->setMargin(0);
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

	QLabel *titleLabel = new QLabel("DAC Amplifier", widget);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	layout->addWidget(titleLabel);

	iio_device *dacAmp = iio_context_find_device(m_ctx, "ad9166-amp");
	if(dacAmp) {
		IIOWidget *enWidget = IIOWidgetBuilder(widget)
					      .device(dacAmp)
					      .attribute("en")
					      .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
					      .title("Enable")
					      .group(m_group)
					      .buildSingle();

		enWidget->showProgressBar(false);
		layout->addWidget(enWidget);
		connect(this, &CN0511::readRequested, enWidget, &IIOWidget::readAsync);
	}

	return widget;
}

void CN0511::applyCalibration()
{
	if(!m_ctx || !m_dac || !m_dacCh) {
		qWarning(CAT_CN0511) << "Cannot apply calibration: device not available";
		return;
	}

	m_enterBtn->startAnimation();

	double calibFreq = m_calibFreq;
	int amplitude = m_amplitude;

	QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
	connect(
		watcher, &QFutureWatcher<void>::finished, this,
		[this, watcher]() {
			m_enterBtn->stopAnimation();
			watcher->deleteLater();
		},
		Qt::QueuedConnection);

	QFuture<void> future = QtConcurrent::run([this, calibFreq, amplitude]() {
		qDebug(CAT_CN0511) << "Applying calibration: freq=" << calibFreq << " amplitude=" << amplitude;

		struct ad9166_calibration_data *calibData = nullptr;
		int ret = ad9166_context_find_calibration_data(m_ctx, "cn0511", &calibData);
		if(ret < 0) {
			qWarning(CAT_CN0511) << "Failed to find calibration data. Error:" << ret;
			return;
		}

		ret = ad9166_channel_set_freq(m_dacCh, calibFreq);
		if(ret < 0) {
			qWarning(CAT_CN0511) << "Failed to set frequency. Error:" << ret;
		}

		ret = ad9166_device_set_amplitude(m_dac, amplitude);
		if(ret < 0) {
			qWarning(CAT_CN0511) << "Failed to set amplitude. Error:" << ret;
		}

		ret = ad9166_device_set_iofs(m_dac, calibData, calibFreq);
		if(ret < 0) {
			qWarning(CAT_CN0511) << "Failed to set IOFS. Error:" << ret;
		}
	});

	watcher->setFuture(future);
}

void CN0511::readFrequencyFromDevice()
{
	if(!m_dacCh || !m_freqSpinBox) {
		return;
	}

	long long val = 0;
	int ret = iio_channel_attr_read_longlong(m_dacCh, "nco_frequency", &val);
	if(ret < 0) {
		qWarning(CAT_CN0511) << "Failed to read nco_frequency. Error:" << ret;
		return;
	}

	m_freqSpinBox->setValue(val / MHZ_SCALE);
}
