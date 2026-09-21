/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#include <component/context.h>
#include <component/device.h>
#include <component/channel.h>
#include <component/attribute.h>
#include <component/navigation.h>

Q_LOGGING_CATEGORY(CAT_FMCOMMS11, "FMCOMMS11");

using namespace scopy;
using namespace fmcomms11;

FMCOMMS11::FMCOMMS11(component::Context *ctx, IIOWidgetGroup *group, QWidget *parent)
	: QWidget(parent)
	, m_ctx(ctx)
	, m_group(group)
{
	if(m_ctx) {
		m_adc = m_ctx->findChild<component::Device *>("axi-ad9625-hpc", Qt::FindDirectChildrenOnly);
		m_dac = m_ctx->findChild<component::Device *>("axi-ad9162-hpc", Qt::FindDirectChildrenOnly);
		m_attn = m_ctx->findChild<component::Device *>("hmc1119", Qt::FindDirectChildrenOnly);
		m_vga = m_ctx->findChild<component::Device *>("adl5240", Qt::FindDirectChildrenOnly);
	}

	setupUi();
}

FMCOMMS11::~FMCOMMS11() {}

void FMCOMMS11::setupUi()
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
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
	controlsLayout->setContentsMargins(0, 0, 0, 0);
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
		component::Channel *voltage0In = component::channelById(m_adc, "voltage0", false);

		if(voltage0In) {
			QGridLayout *gridLayout = new QGridLayout();
			layout->addLayout(gridLayout);

			// Sampling Frequency (read-only, MHz) — left column, row 0
			IIOWidget *samplingFreq =
				IIOWidgetBuilder(widget)
					.attribute(component::attributeByName(voltage0In, "sampling_frequency"))
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
						   .attribute(component::attributeByName(voltage0In, "scale"))
						   .uiStrategy(IIOWidgetBuilder::ComboUi)
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
						      .attribute(component::attributeByName(voltage0In, "test_mode"))
						      .uiStrategy(IIOWidgetBuilder::ComboUi)
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
		component::Channel *voltage0Out = component::channelById(m_attn, "voltage0", true);

		if(voltage0Out) {
			IIOWidget *hardwaregain =
				IIOWidgetBuilder(widget)
					.attribute(component::attributeByName(voltage0Out, "hardwaregain"))
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
		component::Channel *altvoltage0Out = component::channelById(m_dac, "altvoltage0", true);

		if(altvoltage0Out) {
			IIOWidget *samplingFreq =
				IIOWidgetBuilder(widget)
					.attribute(component::attributeByName(altvoltage0Out, "sampling_frequency"))
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
		component::Channel *ncoCh = component::channelById(m_dac, "altvoltage4", true);
		if(!ncoCh) {
			ncoCh = component::channelById(m_dac, "altvoltage2", true);
		}

		if(ncoCh) {
			IIOWidget *ncoFreq = IIOWidgetBuilder(widget)
						     .attribute(component::attributeByName(ncoCh, "frequency_nco"))
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
		component::Channel *voltage0Out = component::channelById(m_dac, "voltage0_i", true);
		component::Attribute *fir85Attr =
			voltage0Out ? component::attributeByName(voltage0Out, "fir85_enable") : nullptr;

		if(fir85Attr) {
			IIOWidget *fir85Enable = IIOWidgetBuilder(widget)
							 .attribute(fir85Attr)
							 .uiStrategy(IIOWidgetBuilder::CheckBoxUi)
							 .title("FIR85 Enable")
							 .group(m_group)
							 .buildSingle();
			if(fir85Enable) {
				fir85Enable->showProgressBar(false);
				fir85Enable->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
				connect(this, &FMCOMMS11::readRequested, fir85Enable, &IIOWidget::readAsync);
				layout->addWidget(fir85Enable);
			}
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
		component::Channel *voltage0Out = component::channelById(m_vga, "voltage0", true);

		if(voltage0Out) {
			IIOWidget *hardwaregain =
				IIOWidgetBuilder(widget)
					.attribute(component::attributeByName(voltage0Out, "hardwaregain"))
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
