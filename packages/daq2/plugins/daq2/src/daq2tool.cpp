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

#include "daq2tool.h"

#include <QFutureWatcher>
#include <QHBoxLayout>
#include <QLabel>
#include <QLoggingCategory>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QtConcurrent>
#include <iio-widgets/iiowidget.h>
#include <iio-widgets/iiowidgetbuilder.h>
#include <style.h>
#include <qcorotask.h>

#include <component/context.h>
#include <component/device.h>
#include <component/channel.h>
#include <component/attribute.h>
#include <component/navigation.h>

Q_LOGGING_CATEGORY(CAT_DAQ2TOOL, "DAQ2Tool")
using namespace scopy;
using namespace scopy::daq2;

DAQ2::DAQ2(component::Context *ctx, IIOWidgetGroup *group, QWidget *parent)
	: QWidget(parent)
	, m_ctx(ctx)
	, m_group(group)
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
	controlsWidget->setLayout(controlsLayout);

	controlsLayout->addWidget(buildAdcSection());
	controlsLayout->addWidget(buildDacSection());
	controlsLayout->addStretch();

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(controlsWidget);

	m_tool->addWidgetToCentralContainerHelper(scrollArea);
}

DAQ2::~DAQ2() {}

QWidget *DAQ2::buildAdcSection()
{
	component::Device *adc =
		m_ctx ? m_ctx->findChild<component::Device *>("axi-ad9680-hpc", Qt::FindDirectChildrenOnly) : nullptr;
	component::Channel *ch0 = adc ? component::channelById(adc, "voltage0", false) : nullptr;
	component::Channel *ch1 = adc ? component::channelById(adc, "voltage1", false) : nullptr;

	QWidget *section = new QWidget(this);
	Style::setBackgroundColor(section, json::theme::background_primary);
	Style::setStyle(section, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(section);
	layout->setSpacing(10);

	QLabel *title = new QLabel("ADC", section);
	Style::setStyle(title, style::properties::label::menuBig);
	layout->addWidget(title);

	// Sampling frequency (read-once at connect, displayed as MHz)
	QWidget *freqRow = new QWidget(section);
	QHBoxLayout *freqLayout = new QHBoxLayout(freqRow);
	freqLayout->setContentsMargins(0, 0, 0, 0);
	freqLayout->addWidget(new QLabel("Sampling frequency:", freqRow));
	QLabel *adcFreqLabel = new QLabel("N/A", freqRow);
	if(ch0) {
		component::Attribute *freqAttr = component::attributeByName(ch0, "sampling_frequency");
		if(freqAttr && freqAttr->readCapability()) {
			QCoro::waitFor(freqAttr->readCapability()->readAsync());
			adcFreqLabel->setText(QString::number(freqAttr->cachedValue().toLongLong() / 1e6, 'f', 3) +
					      " MHz");
		}
	}
	freqLayout->addWidget(adcFreqLabel);
	freqLayout->addStretch();
	layout->addWidget(freqRow);

	// Ch0 test_mode combo
	if(ch0) {
		IIOWidget *ch0TestMode = IIOWidgetBuilder(section)
						 .attribute(component::attributeByName(ch0, "test_mode"))
						 .title("Ch0 Test Mode")
						 .uiStrategy(IIOWidgetBuilder::ComboUi)
						 .group(m_group)
						 .buildSingle();
		layout->addWidget(ch0TestMode);
		connect(this, &DAQ2::readRequested, ch0TestMode, &IIOWidget::readAsync);
	}

	// Ch1 test_mode combo
	if(ch1) {
		IIOWidget *ch1TestMode = IIOWidgetBuilder(section)
						 .attribute(component::attributeByName(ch1, "test_mode"))
						 .title("Ch1 Test Mode")
						 .uiStrategy(IIOWidgetBuilder::ComboUi)
						 .group(m_group)
						 .buildSingle();
		layout->addWidget(ch1TestMode);
		connect(this, &DAQ2::readRequested, ch1TestMode, &IIOWidget::readAsync);
	}

	return section;
}

QWidget *DAQ2::buildDacSection()
{
	component::Device *dac =
		m_ctx ? m_ctx->findChild<component::Device *>("axi-ad9144-hpc", Qt::FindDirectChildrenOnly) : nullptr;
	component::Channel *alt0 = dac ? component::channelById(dac, "altvoltage0", true) : nullptr;

	QWidget *section = new QWidget(this);
	Style::setBackgroundColor(section, json::theme::background_primary);
	Style::setStyle(section, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(section);
	layout->setSpacing(10);

	QLabel *title = new QLabel("DAC", section);
	Style::setStyle(title, style::properties::label::menuBig);
	layout->addWidget(title);

	// Sampling frequency (read-once at connect, displayed as MHz)
	QWidget *freqRow = new QWidget(section);
	QHBoxLayout *freqLayout = new QHBoxLayout(freqRow);
	freqLayout->setContentsMargins(0, 0, 0, 0);
	freqLayout->addWidget(new QLabel("Sampling frequency:", freqRow));
	QLabel *dacFreqLabel = new QLabel("N/A", freqRow);
	if(alt0) {
		component::Attribute *freqAttr = component::attributeByName(alt0, "sampling_frequency");
		if(freqAttr && freqAttr->readCapability()) {
			QCoro::waitFor(freqAttr->readCapability()->readAsync());
			dacFreqLabel->setText(QString::number(freqAttr->cachedValue().toLongLong() / 1e6, 'f', 3) +
					      " MHz");
		}
	}
	freqLayout->addWidget(dacFreqLabel);
	freqLayout->addStretch();
	layout->addWidget(freqRow);

	return section;
}
