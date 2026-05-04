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

#include "fmcomms11.h"

#include <QLabel>
#include <QScrollArea>
#include <QLoggingCategory>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <style.h>
#include <iio-widgets/iiowidgetgroup.h>

#include <iio.h>

Q_LOGGING_CATEGORY(CAT_FMCOMMS11, "FMCOMMS11");

using namespace scopy;
using namespace fmcomms11;

FMCOMMS11::FMCOMMS11(iio_context *ctx, IIOWidgetGroup *group, QWidget *parent)
	: QWidget(parent)
	, m_ctx(ctx)
	, m_group(group)
{
	m_adc = iio_context_find_device(m_ctx, "axi-ad9625-hpc");
	m_dac = iio_context_find_device(m_ctx, "axi-ad9162-hpc");
	m_attn = iio_context_find_device(m_ctx, "hmc1119");
	m_vga = iio_context_find_device(m_ctx, "adl5240");

	setupUi();
}

FMCOMMS11::~FMCOMMS11() {}

void FMCOMMS11::setupUi()
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

	QWidget *controlsWidget = new QWidget(this);
	QVBoxLayout *controlsLayout = new QVBoxLayout(controlsWidget);
	controlsLayout->setMargin(0);
	controlsLayout->setContentsMargins(0, 0, 0, 0);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(controlsWidget);

	controlsLayout->addWidget(generateAdcWidget(controlsWidget));
	controlsLayout->addWidget(generateInputAttenuatorWidget(controlsWidget));
	controlsLayout->addWidget(generateDacWidget(controlsWidget));
	controlsLayout->addWidget(generateOutputVgaWidget(controlsWidget));
	controlsLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	m_tool->addWidgetToCentralContainerHelper(scrollArea);
}

QWidget *FMCOMMS11::generateAdcWidget(QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(widget);

	QLabel *titleLabel = new QLabel("ADC", widget);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	layout->addWidget(titleLabel);

	if(m_adc) {
		iio_channel *voltage0In = iio_device_find_channel(m_adc, "voltage0", false);

		if(voltage0In) {
			QGridLayout *gridLayout = new QGridLayout();
			layout->addLayout(gridLayout);

			// Sampling Frequency (read-only, MHz) — left column, row 0
			IIOWidget *samplingFreq = IIOWidgetBuilder(widget)
							  .device(m_adc)
							  .channel(voltage0In)
							  .attribute("sampling_frequency")
							  .title("Sampling Frequency (MHz)")
							  .group(m_group)
							  .buildSingle();
			if(samplingFreq) {
				samplingFreq->setEnabled(false);
				samplingFreq->showProgressBar(false);
				samplingFreq->setDataToUIConversion(
					[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 2); });
				connect(this, &FMCOMMS11::readRequested, samplingFreq, &IIOWidget::readAsync);
				gridLayout->addWidget(samplingFreq, 0, 0);
			}

			// Scale — left column, row 1
			IIOWidget *scale = IIOWidgetBuilder(widget)
						   .device(m_adc)
						   .channel(voltage0In)
						   .attribute("scale")
						   .uiStrategy(IIOWidgetBuilder::ComboUi)
						   .optionsAttribute("scale_available")
						   .title("Scale")
						   .group(m_group)
						   .buildSingle();
			if(scale) {
				scale->showProgressBar(false);
				connect(this, &FMCOMMS11::readRequested, scale, &IIOWidget::readAsync);
				gridLayout->addWidget(scale, 1, 0);
			}

			// Test Mode — right column, row 0
			IIOWidget *testMode = IIOWidgetBuilder(widget)
						      .device(m_adc)
						      .channel(voltage0In)
						      .attribute("test_mode")
						      .uiStrategy(IIOWidgetBuilder::ComboUi)
						      .optionsAttribute("test_mode_available")
						      .title("Test Mode")
						      .group(m_group)
						      .buildSingle();
			if(testMode) {
				testMode->showProgressBar(false);
				connect(this, &FMCOMMS11::readRequested, testMode, &IIOWidget::readAsync);
				gridLayout->addWidget(testMode, 0, 1);
			}
		}
	}

	return widget;
}

QWidget *FMCOMMS11::generateInputAttenuatorWidget(QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(widget);

	QLabel *titleLabel = new QLabel("Input Attenuator", widget);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	layout->addWidget(titleLabel);

	if(m_attn) {
		iio_channel *voltage0Out = iio_device_find_channel(m_attn, "voltage0", true);

		if(voltage0Out) {
			IIOWidget *hardwaregain = IIOWidgetBuilder(widget)
							  .device(m_attn)
							  .channel(voltage0Out)
							  .attribute("hardwaregain")
							  .uiStrategy(IIOWidgetBuilder::RangeUi)
							  .optionsValues("[-31.75 0.25 0]")
							  .title("Hardwaregain (dB)")
							  .group(m_group)
							  .buildSingle();
			if(hardwaregain) {
				hardwaregain->showProgressBar(false);
				hardwaregain->setDataToUIConversion(
					[](QString data) { return data.split(" ").first(); });
				connect(this, &FMCOMMS11::readRequested, hardwaregain, &IIOWidget::readAsync);
				layout->addWidget(hardwaregain);
			}
		}
	}

	return widget;
}

QWidget *FMCOMMS11::generateDacWidget(QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(widget);

	QLabel *titleLabel = new QLabel("DAC", widget);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	layout->addWidget(titleLabel);

	if(m_dac) {
		// Sampling Frequency (read-only, MHz) - on altvoltage0 output channel
		iio_channel *altvoltage0Out = iio_device_find_channel(m_dac, "altvoltage0", true);

		if(altvoltage0Out) {
			IIOWidget *samplingFreq = IIOWidgetBuilder(widget)
							  .device(m_dac)
							  .channel(altvoltage0Out)
							  .attribute("sampling_frequency")
							  .title("Sampling Frequency (MHz)")
							  .group(m_group)
							  .buildSingle();
			if(samplingFreq) {
				samplingFreq->setEnabled(false);
				samplingFreq->showProgressBar(false);
				samplingFreq->setDataToUIConversion(
					[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 2); });
				connect(this, &FMCOMMS11::readRequested, samplingFreq, &IIOWidget::readAsync);
				layout->addWidget(samplingFreq);
			}
		}

		// NCO Frequency (MHz) - try altvoltage4 first, fallback to altvoltage2
		iio_channel *ncoCh = iio_device_find_channel(m_dac, "altvoltage4", true);
		if(!ncoCh) {
			ncoCh = iio_device_find_channel(m_dac, "altvoltage2", true);
		}

		if(ncoCh) {
			IIOWidget *ncoFreq = IIOWidgetBuilder(widget)
						     .device(m_dac)
						     .channel(ncoCh)
						     .attribute("frequency_nco")
						     .uiStrategy(IIOWidgetBuilder::RangeUi)
						     .optionsValues("[1000000 1 5999000000]")
						     .title("NCO Frequency (MHz)")
						     .group(m_group)
						     .buildSingle();
			if(ncoFreq) {
				ncoFreq->showProgressBar(false);
				ncoFreq->setDataToUIConversion(
					[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
				ncoFreq->setRangeToUIConversion(
					[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
				ncoFreq->setUItoDataConversion(
					[](QString data) { return QString::number(data.toDouble() * 1e6, 'f', 0); });
				connect(this, &FMCOMMS11::readRequested, ncoFreq, &IIOWidget::readAsync);
				layout->addWidget(ncoFreq);
			}
		}

		// FIR85 Enable
		iio_channel *voltage0Out = iio_device_find_channel(m_dac, "voltage0_i", true);
		const char *fir85Attr = voltage0Out ? iio_channel_find_attr(voltage0Out, "fir85_enable") : nullptr;

		IIOWidget *fir85Enable = IIOWidgetBuilder(widget)
						 .device(m_dac)
						 .channel(voltage0Out)
						 .attribute("fir85_enable")
						 .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						 .title("FIR85 Enable")
						 .group(m_group)
						 .buildSingle();

		if(fir85Enable) {
			fir85Enable->showProgressBar(false);
			fir85Enable->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
			if(!voltage0Out || !fir85Attr) {
				fir85Enable->setEnabled(false);
				fir85Enable->getUiStrategy()->setInfoMessage(
					"The attribute for this option is missing");
			} else {
				connect(this, &FMCOMMS11::readRequested, fir85Enable, &IIOWidget::readAsync);
			}
			layout->addWidget(fir85Enable);
		}
	}

	return widget;
}

QWidget *FMCOMMS11::generateOutputVgaWidget(QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(widget);

	QLabel *titleLabel = new QLabel("Output VGA", widget);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	layout->addWidget(titleLabel);

	if(m_vga) {
		iio_channel *voltage0Out = iio_device_find_channel(m_vga, "voltage0", true);

		if(voltage0Out) {
			IIOWidget *hardwaregain = IIOWidgetBuilder(widget)
							  .device(m_vga)
							  .channel(voltage0Out)
							  .attribute("hardwaregain")
							  .uiStrategy(IIOWidgetBuilder::RangeUi)
							  .optionsValues("[-11.5 0.5 20]")
							  .title("Hardwaregain (dB)")
							  .group(m_group)
							  .buildSingle();
			if(hardwaregain) {
				hardwaregain->showProgressBar(false);
				hardwaregain->setDataToUIConversion(
					[](QString data) { return data.split(" ").first(); });
				connect(this, &FMCOMMS11::readRequested, hardwaregain, &IIOWidget::readAsync);
				layout->addWidget(hardwaregain);
			}
		}
	}

	return widget;
}

#include "moc_fmcomms11.cpp"
