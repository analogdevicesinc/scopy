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

#include "genalyzersettings.h"
#include <gui/style.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleValidator>
#include <QSpacerItem>

using namespace scopy;
using namespace scopy::adc;
using namespace scopy::grutil;

GenalyzerSettings::GenalyzerSettings(QWidget *parent)
	: QWidget(parent)
{
	setupUI();
}

GenalyzerSettings::~GenalyzerSettings() {}

void GenalyzerSettings::setupUI()
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	// Mode selection
	QWidget *modeContainer = new QWidget(this);
	QHBoxLayout *modeLayout = new QHBoxLayout(modeContainer);
	modeLayout->setContentsMargins(0, 0, 0, 0);

	QLabel *modeLabel = new QLabel("Mode:", modeContainer);
	Style::setStyle(modeLabel, style::properties::label::subtle);

	m_modeCombo = new QComboBox(modeContainer);
	m_modeCombo->addItem("Auto", static_cast<int>(GenalyzerMode::AUTO));
	m_modeCombo->addItem("Fixed Tone", static_cast<int>(GenalyzerMode::FIXED_TONE));

	modeLayout->addWidget(modeLabel);
	modeLayout->addWidget(m_modeCombo);
	modeLayout->addStretch();

	mainLayout->addWidget(modeContainer);

	// Auto mode container
	m_autoModeContainer = new QWidget(this);
	QHBoxLayout *ssbLayout = new QHBoxLayout(m_autoModeContainer);
	ssbLayout->setContentsMargins(10, 0, 0, 0);

	QLabel *ssbLabel = new QLabel("SSB Width:", m_autoModeContainer);
	Style::setStyle(ssbLabel, style::properties::label::subtle);

	m_ssbWidthSpinbox = new QSpinBox(m_autoModeContainer);
	m_ssbWidthSpinbox->setRange(0, 255);

	ssbLayout->addWidget(ssbLabel);
	ssbLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	ssbLayout->addWidget(m_ssbWidthSpinbox);
	ssbLayout->addStretch();

	mainLayout->addWidget(m_autoModeContainer);

	// Fixed tone container
	m_fixedToneContainer = new QWidget(this);
	QVBoxLayout *ftLayout = new QVBoxLayout(m_fixedToneContainer);
	ftLayout->setContentsMargins(10, 0, 0, 0);

	// Expected frequency
	QWidget *freqContainer = new QWidget(m_fixedToneContainer);
	QHBoxLayout *freqLayout = new QHBoxLayout(freqContainer);
	freqLayout->setContentsMargins(0, 0, 0, 0);

	QLabel *freqLabel = new QLabel("Expected Freq (Hz):", freqContainer);
	Style::setStyle(freqLabel, style::properties::label::subtle);

	m_expectedFreqEdit = new QLineEdit(freqContainer);
	m_expectedFreqEdit->setValidator(new QDoubleValidator(-1000000000, 1000000000, 0, this));

	freqLayout->addWidget(freqLabel);
	freqLayout->addWidget(m_expectedFreqEdit);
	ftLayout->addWidget(freqContainer);

	// Harmonic order
	QWidget *harmContainer = new QWidget(m_fixedToneContainer);
	QHBoxLayout *harmLayout = new QHBoxLayout(harmContainer);
	harmLayout->setContentsMargins(0, 0, 0, 0);

	QLabel *harmLabel = new QLabel("Harmonic Order:", harmContainer);
	Style::setStyle(harmLabel, style::properties::label::subtle);

	m_harmonicOrderSpinbox = new QSpinBox(harmContainer);
	m_harmonicOrderSpinbox->setRange(0, 20);

	harmLayout->addWidget(harmLabel);
	harmLayout->addWidget(m_harmonicOrderSpinbox);
	ftLayout->addWidget(harmContainer);

	// SSB Fundamental
	QWidget *ssbFundContainer = new QWidget(m_fixedToneContainer);
	QHBoxLayout *ssbFundLayout = new QHBoxLayout(ssbFundContainer);
	ssbFundLayout->setContentsMargins(0, 0, 0, 0);

	QLabel *ssbFundLabel = new QLabel("SSB Fundamental:", ssbFundContainer);
	Style::setStyle(ssbFundLabel, style::properties::label::subtle);

	m_ssbFundamentalSpinbox = new QSpinBox(ssbFundContainer);
	m_ssbFundamentalSpinbox->setRange(0, 50);

	ssbFundLayout->addWidget(ssbFundLabel);
	ssbFundLayout->addWidget(m_ssbFundamentalSpinbox);
	ftLayout->addWidget(ssbFundContainer);

	// SSB Default
	QWidget *ssbDefContainer = new QWidget(m_fixedToneContainer);
	QHBoxLayout *ssbDefLayout = new QHBoxLayout(ssbDefContainer);
	ssbDefLayout->setContentsMargins(0, 0, 0, 0);

	QLabel *ssbDefLabel = new QLabel("SSB Default:", ssbDefContainer);
	Style::setStyle(ssbDefLabel, style::properties::label::subtle);

	m_ssbDefaultSpinbox = new QSpinBox(ssbDefContainer);
	m_ssbDefaultSpinbox->setRange(0, 50);

	ssbDefLayout->addWidget(ssbDefLabel);
	ssbDefLayout->addWidget(m_ssbDefaultSpinbox);
	ftLayout->addWidget(ssbDefContainer);
	mainLayout->addWidget(m_fixedToneContainer);

	// Connect mode change to show/hide appropriate controls
	connect(m_modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
		bool isFixedTone = (m_modeCombo->currentData().toInt() == static_cast<int>(GenalyzerMode::FIXED_TONE));
		m_fixedToneContainer->setVisible(isFixedTone);
		m_autoModeContainer->setVisible(!isFixedTone);
		onUIChanged();
	});

	// Connect all UI changes
	connect(m_ssbWidthSpinbox, QOverload<int>::of(&QSpinBox::valueChanged), this, &GenalyzerSettings::onUIChanged);
	connect(m_expectedFreqEdit, &QLineEdit::textChanged, this, &GenalyzerSettings::onUIChanged);
	connect(m_harmonicOrderSpinbox, QOverload<int>::of(&QSpinBox::valueChanged), this,
		&GenalyzerSettings::onUIChanged);
	connect(m_ssbFundamentalSpinbox, QOverload<int>::of(&QSpinBox::valueChanged), this,
		&GenalyzerSettings::onUIChanged);
	connect(m_ssbDefaultSpinbox, QOverload<int>::of(&QSpinBox::valueChanged), this,
		&GenalyzerSettings::onUIChanged);

	// Initialize visibility
	m_fixedToneContainer->setVisible(false);
}

GenalyzerConfig GenalyzerSettings::getConfig() const { return m_config; }

void GenalyzerSettings::setConfig(const GenalyzerConfig &config)
{
	m_config = config;
	updateUIFromConfig();
}

void GenalyzerSettings::enableAnalysis(bool en)
{
	m_config.enabled = en;
	Q_EMIT configChanged(m_config);
}

void GenalyzerSettings::onUIChanged()
{
	m_config.mode = static_cast<GenalyzerMode>(m_modeCombo->currentData().toInt());

	if(m_config.mode == GenalyzerMode::AUTO) {
		m_config.auto_params.ssb_width = static_cast<uint8_t>(m_ssbWidthSpinbox->value());
	}
	if(m_config.mode == GenalyzerMode::FIXED_TONE) {
		m_config.fixed_tone.expected_freq = m_expectedFreqEdit->text().toDouble();
		m_config.fixed_tone.harmonic_order = m_harmonicOrderSpinbox->value();
		m_config.fixed_tone.ssb_fundamental = m_ssbFundamentalSpinbox->value();
		m_config.fixed_tone.ssb_default = m_ssbDefaultSpinbox->value();
	}

	Q_EMIT configChanged(m_config);
}

void GenalyzerSettings::updateUIFromConfig()
{
	m_modeCombo->blockSignals(true);
	m_ssbWidthSpinbox->blockSignals(true);
	m_expectedFreqEdit->blockSignals(true);
	m_harmonicOrderSpinbox->blockSignals(true);
	m_ssbFundamentalSpinbox->blockSignals(true);
	m_ssbDefaultSpinbox->blockSignals(true);

	m_modeCombo->setCurrentIndex(m_modeCombo->findData(static_cast<int>(m_config.mode)));
	m_ssbWidthSpinbox->setValue(m_config.auto_params.ssb_width);
	m_expectedFreqEdit->setText(QString::number(m_config.fixed_tone.expected_freq));
	m_harmonicOrderSpinbox->setValue(m_config.fixed_tone.harmonic_order);
	m_ssbFundamentalSpinbox->setValue(m_config.fixed_tone.ssb_fundamental);
	m_ssbDefaultSpinbox->setValue(m_config.fixed_tone.ssb_default);

	bool isFixedTone = (m_config.mode == GenalyzerMode::FIXED_TONE);
	m_fixedToneContainer->setVisible(isFixedTone);
	m_autoModeContainer->setVisible(!isFixedTone);

	m_modeCombo->blockSignals(false);
	m_ssbWidthSpinbox->blockSignals(false);
	m_expectedFreqEdit->blockSignals(false);
	m_harmonicOrderSpinbox->blockSignals(false);
	m_ssbFundamentalSpinbox->blockSignals(false);
	m_ssbDefaultSpinbox->blockSignals(false);
}
