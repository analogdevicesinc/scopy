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

#include "fmcomms11tool.h"

#include <QLabel>
#include <QScrollArea>
#include <toolbuttons.h>
#include <style.h>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_FMCOMMS11TOOL, "Fmcomms11Tool");

using namespace scopy;
using namespace fmcomms11;

Fmcomms11Tool::Fmcomms11Tool(iio_context *ctx, IIOWidgetGroup *group, QWidget *parent)
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

	QWidget *controlsWidget = new QWidget(this);
	QVBoxLayout *controlsLayout = new QVBoxLayout(controlsWidget);
	controlsLayout->setMargin(0);
	controlsLayout->setContentsMargins(0, 0, 0, 0);
	controlsWidget->setLayout(controlsLayout);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(controlsWidget);

	m_tool->addWidgetToCentralContainerHelper(scrollArea);

	if(m_ctx != nullptr) {
		iio_device *adcDev = iio_context_find_device(m_ctx, "axi-ad9625-hpc");
		iio_device *dacDev = iio_context_find_device(m_ctx, "axi-ad9162-hpc");
		iio_device *vgaDev = iio_context_find_device(m_ctx, "adl5240");
		iio_device *attnDev = iio_context_find_device(m_ctx, "hmc1119");
		iio_device *pllDev = iio_context_find_device(m_ctx, "adf4355");

		controlsLayout->addWidget(generateRxSection(adcDev, vgaDev, controlsWidget));
		controlsLayout->addWidget(generateTxSection(dacDev, attnDev, controlsWidget));
		controlsLayout->addWidget(generateLoSection(pllDev, controlsWidget));

		controlsLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));
	}
}

Fmcomms11Tool::~Fmcomms11Tool() {}

QWidget *Fmcomms11Tool::generateRxSection(iio_device *adcDev, iio_device *vgaDev, QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *mainLayout = new QVBoxLayout(widget);
	widget->setLayout(mainLayout);

	QLabel *titleLabel = new QLabel("Receive Chain", widget);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	mainLayout->addWidget(titleLabel);

	QHBoxLayout *adcLayout = new QHBoxLayout();

	if(adcDev) {
		iio_channel *voltage0 = iio_device_find_channel(adcDev, "voltage0", false);

		if(voltage0) {
			// sampling_frequency (read-only)
			IIOWidget *samplingFreq = IIOWidgetBuilder(widget)
							  .channel(voltage0)
							  .attribute("sampling_frequency")
							  .title("ADC Sampling Freq (MHz)")
							  .group(m_group)
							  .buildSingle();
			samplingFreq->setEnabled(false);
			samplingFreq->setDataToUIConversion(
				[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
			adcLayout->addWidget(samplingFreq);
			connect(this, &Fmcomms11Tool::readRequested, samplingFreq, &IIOWidget::readAsync);

			// scale (read-only)
			IIOWidget *scale = IIOWidgetBuilder(widget)
						   .channel(voltage0)
						   .attribute("scale")
						   .title("ADC Scale")
						   .group(m_group)
						   .buildSingle();
			scale->setEnabled(false);
			adcLayout->addWidget(scale);
			connect(this, &Fmcomms11Tool::readRequested, scale, &IIOWidget::readAsync);

			// test_mode
			IIOWidget *testMode = IIOWidgetBuilder(widget)
						      .channel(voltage0)
						      .attribute("test_mode")
						      .title("Test Mode")
						      .group(m_group)
						      .buildSingle();
			adcLayout->addWidget(testMode);
			connect(this, &Fmcomms11Tool::readRequested, testMode, &IIOWidget::readAsync);
		}
	}

	mainLayout->addLayout(adcLayout);

	// VGA: adl5240
	if(vgaDev) {
		iio_channel *voltage0 = iio_device_find_channel(vgaDev, "voltage0", true);

		if(voltage0) {
			IIOWidget *hardwaregain = IIOWidgetBuilder(widget)
							  .channel(voltage0)
							  .attribute("hardwaregain")
							  .title("VGA Gain (dB)")
							  .group(m_group)
							  .buildSingle();
			hardwaregain->setDataToUIConversion([](QString data) {
				auto result = data.split(" ");
				return result.first();
			});
			mainLayout->addWidget(hardwaregain);
			connect(this, &Fmcomms11Tool::readRequested, hardwaregain, &IIOWidget::readAsync);
		}
	}

	mainLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	return widget;
}

QWidget *Fmcomms11Tool::generateTxSection(iio_device *dacDev, iio_device *attnDev, QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(widget);
	widget->setLayout(layout);

	QLabel *titleLabel = new QLabel("Transmit Chain", widget);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	layout->addWidget(titleLabel);

	// DAC settings
	if(dacDev) {
		QHBoxLayout *dacLayout = new QHBoxLayout();

		// voltage0_i
		iio_channel *voltage0_i = iio_device_find_channel(dacDev, "voltage0_i", true);
		if(voltage0_i) {
			IIOWidget *fir85I = IIOWidgetBuilder(widget)
						    .channel(voltage0_i)
						    .attribute("fir85_enable")
						    .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						    .title("FIR85 Enable (I)")
						    .group(m_group)
						    .buildSingle();
			fir85I->showProgressBar(false);
			dacLayout->addWidget(fir85I);
			connect(this, &Fmcomms11Tool::readRequested, fir85I, &IIOWidget::readAsync);

			IIOWidget *dacSampFreq = IIOWidgetBuilder(widget)
							 .channel(voltage0_i)
							 .attribute("sampling_frequency")
							 .title("DAC Sampling Freq (MHz)")
							 .group(m_group)
							 .buildSingle();
			dacSampFreq->setEnabled(false);
			dacSampFreq->setDataToUIConversion(
				[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
			dacLayout->addWidget(dacSampFreq);
			connect(this, &Fmcomms11Tool::readRequested, dacSampFreq, &IIOWidget::readAsync);
		}

		// voltage0_q
		iio_channel *voltage0_q = iio_device_find_channel(dacDev, "voltage0_q", true);
		if(voltage0_q) {
			IIOWidget *fir85Q = IIOWidgetBuilder(widget)
						    .channel(voltage0_q)
						    .attribute("fir85_enable")
						    .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
						    .title("FIR85 Enable (Q)")
						    .group(m_group)
						    .buildSingle();
			fir85Q->showProgressBar(false);
			dacLayout->addWidget(fir85Q);
			connect(this, &Fmcomms11Tool::readRequested, fir85Q, &IIOWidget::readAsync);
		}

		layout->addLayout(dacLayout);

		// DDS Tones
		QLabel *ddsTitle = new QLabel("DDS Tones", widget);
		Style::setStyle(ddsTitle, style::properties::label::menuBig);
		layout->addWidget(ddsTitle);

		const char *toneNames[] = {"TX1_I_F1", "TX1_I_F2", "TX1_Q_F1", "TX1_Q_F2"};
		for(int i = 0; i < 4; i++) {
			QString chnId = QString("altvoltage%1").arg(i);
			iio_channel *chn = iio_device_find_channel(dacDev, chnId.toStdString().c_str(), true);
			if(chn) {
				layout->addWidget(generateDdsToneWidget(chn, toneNames[i], widget));
			}
		}

		// NCO (altvoltage4)
		iio_channel *altvoltage4 = iio_device_find_channel(dacDev, "altvoltage4", true);
		if(altvoltage4) {
			QWidget *ncoWidget = new QWidget(widget);
			Style::setStyle(ncoWidget, style::properties::widget::border_interactive);
			QHBoxLayout *ncoLayout = new QHBoxLayout(ncoWidget);

			QLabel *ncoLabel = new QLabel("NCO", ncoWidget);
			Style::setStyle(ncoLabel, style::properties::label::menuBig);
			ncoLayout->addWidget(ncoLabel);

			IIOWidget *ncoFreq = IIOWidgetBuilder(ncoWidget)
						     .channel(altvoltage4)
						     .attribute("frequency_nco")
						     .title("NCO Frequency (MHz)")
						     .group(m_group)
						     .buildSingle();
			ncoFreq->setDataToUIConversion(
				[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
			ncoFreq->setUItoDataConversion(
				[](QString data) { return QString::number(data.toDouble() * 1e6, 'f', 0); });
			ncoLayout->addWidget(ncoFreq);
			connect(this, &Fmcomms11Tool::readRequested, ncoFreq, &IIOWidget::readAsync);

			layout->addWidget(ncoWidget);
		}
	}

	// Attenuator: hmc1119
	if(attnDev) {
		iio_channel *voltage0 = iio_device_find_channel(attnDev, "voltage0", true);
		if(voltage0) {
			IIOWidget *attenuation = IIOWidgetBuilder(widget)
							 .channel(voltage0)
							 .attribute("hardwaregain")
							 .title("Attenuation (dB)")
							 .group(m_group)
							 .buildSingle();
			attenuation->setDataToUIConversion([](QString data) {
				auto result = data.split(" ");
				return result.first();
			});
			layout->addWidget(attenuation);
			connect(this, &Fmcomms11Tool::readRequested, attenuation, &IIOWidget::readAsync);
		}
	}

	layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	return widget;
}

QWidget *Fmcomms11Tool::generateDdsToneWidget(iio_channel *chn, const QString &title, QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QHBoxLayout *layout = new QHBoxLayout(widget);
	widget->setLayout(layout);

	QLabel *titleLabel = new QLabel(title, widget);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	layout->addWidget(titleLabel);

	// Enable (raw)
	IIOWidget *enable = IIOWidgetBuilder(widget)
				    .channel(chn)
				    .attribute("raw")
				    .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
				    .title("Enable")
				    .group(m_group)
				    .buildSingle();
	enable->showProgressBar(false);
	layout->addWidget(enable);
	connect(this, &Fmcomms11Tool::readRequested, enable, &IIOWidget::readAsync);

	// Frequency
	IIOWidget *frequency = IIOWidgetBuilder(widget)
				       .channel(chn)
				       .attribute("frequency")
				       .title("Frequency (MHz)")
				       .group(m_group)
				       .buildSingle();
	frequency->setDataToUIConversion(
		[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
	frequency->setUItoDataConversion(
		[](QString data) { return QString::number(data.toDouble() * 1e6, 'f', 0); });
	layout->addWidget(frequency);
	connect(this, &Fmcomms11Tool::readRequested, frequency, &IIOWidget::readAsync);

	// Phase
	IIOWidget *phase = IIOWidgetBuilder(widget)
				   .channel(chn)
				   .attribute("phase")
				   .title("Phase (mdeg)")
				   .group(m_group)
				   .buildSingle();
	layout->addWidget(phase);
	connect(this, &Fmcomms11Tool::readRequested, phase, &IIOWidget::readAsync);

	// Scale
	IIOWidget *scale = IIOWidgetBuilder(widget)
				   .channel(chn)
				   .attribute("scale")
				   .title("Scale")
				   .group(m_group)
				   .buildSingle();
	layout->addWidget(scale);
	connect(this, &Fmcomms11Tool::readRequested, scale, &IIOWidget::readAsync);

	return widget;
}

QWidget *Fmcomms11Tool::generateLoSection(iio_device *pllDev, QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(widget);
	widget->setLayout(layout);

	QLabel *titleLabel = new QLabel("LO / PLL", widget);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	layout->addWidget(titleLabel);

	if(pllDev) {
		layout->addWidget(generateLoChannelWidget(pllDev, "altvoltage0", "LO 1", widget));
		layout->addWidget(generateLoChannelWidget(pllDev, "altvoltage1", "LO 2", widget));
	}

	layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	return widget;
}

QWidget *Fmcomms11Tool::generateLoChannelWidget(iio_device *dev, const QString &channelId, const QString &title,
						 QWidget *parent)
{
	QWidget *widget = new QWidget(parent);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QHBoxLayout *layout = new QHBoxLayout(widget);
	widget->setLayout(layout);

	QLabel *titleLabel = new QLabel(title, widget);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	layout->addWidget(titleLabel);

	iio_channel *chn = iio_device_find_channel(dev, channelId.toStdString().c_str(), true);
	if(!chn) {
		return widget;
	}

	// Frequency
	IIOWidget *frequency = IIOWidgetBuilder(widget)
				       .channel(chn)
				       .attribute("frequency")
				       .title("Frequency (MHz)")
				       .group(m_group)
				       .buildSingle();
	frequency->setDataToUIConversion(
		[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
	frequency->setUItoDataConversion(
		[](QString data) { return QString::number(data.toDouble() * 1e6, 'f', 0); });
	layout->addWidget(frequency);
	connect(this, &Fmcomms11Tool::readRequested, frequency, &IIOWidget::readAsync);

	// Power Down
	IIOWidget *powerdown = IIOWidgetBuilder(widget)
				       .channel(chn)
				       .attribute("powerdown")
				       .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
				       .title("Power Down")
				       .group(m_group)
				       .buildSingle();
	powerdown->showProgressBar(false);
	layout->addWidget(powerdown);
	connect(this, &Fmcomms11Tool::readRequested, powerdown, &IIOWidget::readAsync);

	// Ref Input Frequency (read-only)
	IIOWidget *refinFreq = IIOWidgetBuilder(widget)
				       .channel(chn)
				       .attribute("refin_frequency")
				       .title("Ref Input (MHz)")
				       .group(m_group)
				       .buildSingle();
	refinFreq->setEnabled(false);
	refinFreq->setDataToUIConversion(
		[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
	layout->addWidget(refinFreq);
	connect(this, &Fmcomms11Tool::readRequested, refinFreq, &IIOWidget::readAsync);

	return widget;
}
