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
 */

#include <channelconfigwidget.h>
#include <profilegeneratorwidget.h>
#include <profilegeneratorconstants.h>

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_CHANNELCONFIGWIDGET, "ChannelConfigWidget")

using namespace scopy::adrv9002;
using namespace scopy;

ChannelConfigWidget::ChannelConfigWidget(const QString &title, ChannelMode mode, const QStringList &rfInputOptions,
					 QWidget *parent)
	: QWidget(parent)
	, m_title(title)
	, m_mode(mode)
	, m_rfInputOptions(rfInputOptions)
	, m_layout(nullptr)
	, m_titleLabel(nullptr)
	, m_enabledCb(nullptr)
	, m_freqOffsetCb(nullptr)
	, m_bandwidthCombo(nullptr)
	, m_sampleRateCombo(nullptr)
	, m_rfInputCombo(nullptr)
	, m_updatingData(false)
	, m_isLTEMode(true) // Default to LTE mode
{
	setupUI();
	connectSignals();
}

ChannelConfigWidget::~ChannelConfigWidget() {}

void ChannelConfigWidget::setupUI()
{
	// Create main layout for ChannelConfigWidget (following AD936X RSSI pattern)
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(0);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	setLayout(mainLayout);

	// Create internal styled container widget
	QWidget *container = new QWidget(this);
	Style::setBackgroundColor(container, json::theme::background_primary);
	Style::setStyle(container, style::properties::widget::border_interactive);
	mainLayout->addWidget(container);

	// Create grid layout for the container (existing logic preserved)
	m_layout = new QGridLayout(container);
	container->setLayout(m_layout);
	m_layout->setContentsMargins(15, 15, 15, 15); // Standard margins from prompt
	m_layout->setSpacing(10);		      // Standard spacing from prompt

	// Title
	m_titleLabel = new QLabel(m_title);
	Style::setStyle(m_titleLabel, style::properties::label::menuBig);
	m_layout->addWidget(m_titleLabel, 0, 0, 1, 2);

	// Enabled checkbox
	m_enabledCb = new QCheckBox("Enabled");
	m_enabledCb->setChecked(true); // Default enabled
	m_layout->addWidget(m_enabledCb, 1, 0, 1, 2);

	// Frequency Offset Correction
	m_freqOffsetCb = new QCheckBox("Frequency Offset Correction");
	m_layout->addWidget(m_freqOffsetCb, 2, 0, 1, 2);

	// Bandwidth (Hz) - matching iio-oscilloscope exactly
	m_layout->addWidget(new QLabel("Bandwidth (Hz):"), 3, 0);
	m_bandwidthCombo = new QComboBox();
	m_bandwidthCombo->addItems(FrequencyTable::BANDWIDTHS_HZ);
	m_bandwidthCombo->setEditable(true);
	m_bandwidthCombo->setEnabled(false); // Default disabled for LTE mode
	m_layout->addWidget(m_bandwidthCombo, 3, 1);

	// Interface Sample Rate (Hz) - matching iio-oscilloscope exactly
	m_layout->addWidget(new QLabel("Interface Sample Rate (Hz):"), 4, 0);
	m_sampleRateCombo = new QComboBox();
	m_sampleRateCombo->addItems(FrequencyTable::SAMPLE_RATES_HZ);
	m_layout->addWidget(m_sampleRateCombo, 4, 1);

	// RX RF Input (only for RX channels) - using descriptive names
	if(m_mode == RX_MODE && !m_rfInputOptions.isEmpty()) {
		m_layout->addWidget(new QLabel("RX RF Input:"), 5, 0);
		m_rfInputCombo = new QComboBox();
		m_rfInputCombo->addItems(m_rfInputOptions);
		m_layout->addWidget(m_rfInputCombo, 5, 1);
	}

	updateControlsVisibility(m_isLTEMode);
}

void ChannelConfigWidget::connectSignals()
{
	connect(m_enabledCb, &QCheckBox::toggled, this, &ChannelConfigWidget::onEnabledChanged);
	connect(m_freqOffsetCb, &QCheckBox::toggled, this, &ChannelConfigWidget::onFreqOffsetChanged);
	connect(m_bandwidthCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged), this,
		&ChannelConfigWidget::onBandwidthChanged);
	connect(m_sampleRateCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged), this,
		&ChannelConfigWidget::onSampleRateChanged);
}

void ChannelConfigWidget::setChannelData(const ChannelData &data)
{
	if(m_updatingData)
		return;

	m_updatingData = true;

	m_enabledCb->setChecked(data.enabled);
	m_freqOffsetCb->setChecked(data.freqOffsetCorrection);
	m_bandwidthCombo->setCurrentText(data.bandwidth);
	m_sampleRateCombo->setCurrentText(data.sampleRate);

	if(m_rfInputCombo && m_mode == RX_MODE) {
		m_rfInputCombo->setCurrentText(data.rfInput);
	}

	m_updatingData = false;

	updateControlsVisibility(m_isLTEMode);
}

ChannelConfigWidget::ChannelData ChannelConfigWidget::getChannelData() const
{
	ChannelData data;
	data.enabled = m_enabledCb->isChecked();
	data.freqOffsetCorrection = m_freqOffsetCb->isChecked();
	data.bandwidth = m_bandwidthCombo->currentText();
	data.sampleRate = m_sampleRateCombo->currentText();

	if(m_rfInputCombo && m_mode == RX_MODE) {
		data.rfInput = m_rfInputCombo->currentText();
	}

	return data;
}

void ChannelConfigWidget::updateControlsVisibility(bool lteMode)
{
	m_isLTEMode = lteMode; // Store mode state for mode-aware control logic
	bool channelEnabled = m_enabledCb->isChecked();

	// Apply mode-specific control settings
	if(m_isLTEMode) {
		// LTE Mode: Sample rate from predefined list (combo), bandwidth read-only
		m_bandwidthCombo->setEnabled(false);

		m_sampleRateCombo->setEditable(false);
		m_sampleRateCombo->setEnabled(channelEnabled);

	} else {
		// Live Device Mode: Both sample rate and bandwidth can be edited
		m_bandwidthCombo->setEnabled(channelEnabled);

		m_sampleRateCombo->setEditable(true);
		m_sampleRateCombo->setEnabled(channelEnabled);
	}

	// These always follow channel state regardless of mode
	m_freqOffsetCb->setEnabled(channelEnabled);
	if(m_rfInputCombo) {
		m_rfInputCombo->setEnabled(channelEnabled);
	}
}

void ChannelConfigWidget::setBandwidthReadOnly(bool readOnly) { m_bandwidthCombo->setEnabled(!readOnly); }

void ChannelConfigWidget::setSampleRateOptions(const QStringList &options, bool forceValue)
{
	m_updatingData = true;
	QString currentText = m_sampleRateCombo->currentText();

	m_sampleRateCombo->clear();
	m_sampleRateCombo->addItems(options);

	if(forceValue && !options.isEmpty()) {
		// Force first option (device value) like iio-oscilloscope populate_combo_box
		m_sampleRateCombo->setCurrentText(options.first());
	} else if(options.contains(currentText)) {
		// Restore selection if still valid (existing behavior)
		m_sampleRateCombo->setCurrentText(currentText);
	} else if(!options.isEmpty()) {
		m_sampleRateCombo->setCurrentText(options.first());
	}

	m_updatingData = false;
}

void ChannelConfigWidget::setBandwidthOptions(const QStringList &options, bool forceValue)
{
	m_updatingData = true;
	QString currentText = m_bandwidthCombo->currentText();

	m_bandwidthCombo->clear();
	m_bandwidthCombo->addItems(options);

	if(forceValue && !options.isEmpty()) {
		// Force first option (device value) like iio-oscilloscope populate_combo_box
		m_bandwidthCombo->setCurrentText(options.first());
	} else if(options.contains(currentText)) {
		// Restore selection if still valid (existing behavior)
		m_bandwidthCombo->setCurrentText(currentText);
	} else if(!options.isEmpty()) {
		m_bandwidthCombo->setCurrentText(options.first());
	}

	m_updatingData = false;
}

void ChannelConfigWidget::onEnabledChanged()
{
	if(m_updatingData)
		return;

	updateControlsVisibility(m_isLTEMode);
	Q_EMIT enabledChanged(m_enabledCb->isChecked());
	Q_EMIT channelDataChanged();
}

void ChannelConfigWidget::onSampleRateChanged()
{
	if(m_updatingData)
		return;

	Q_EMIT sampleRateChanged(m_sampleRateCombo->currentText());
	Q_EMIT channelDataChanged();
}

void ChannelConfigWidget::onBandwidthChanged()
{
	if(m_updatingData)
		return;

	Q_EMIT channelDataChanged();
}

void ChannelConfigWidget::onFreqOffsetChanged()
{
	if(m_updatingData)
		return;

	Q_EMIT channelDataChanged();
}

#include "moc_channelconfigwidget.cpp"
