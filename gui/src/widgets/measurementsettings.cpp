/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#include "measurementsettings.h"

#include <widgets/menuonoffswitch.h>
#include <widgets/menusectionwidget.h>
#include <style.h>
#include <QSpinBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QDoubleValidator>
#include <QVariantMap>
#include <limits>

using namespace scopy;

MeasurementSettings::MeasurementSettings(QWidget *parent)
	: QWidget(parent)
	, m_mode(0)  // AUTO mode
	, m_auto_ssb_width(120)
	, m_expected_freq(1e6)
	, m_harmonic_order(10)
	, m_ssb_fundamental(4)
	, m_ssb_harmonics(3)
	, m_ssb_default(3)
	, m_coherent_sampling(false)
{

	QVBoxLayout *lay = new QVBoxLayout(this);
	setLayout(lay);
	//		setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	lay->setMargin(0);

	measureSection = new MenuSectionWidget(this);
	Style::setStyle(measureSection, style::properties::widget::border);
	measurePanelSwitch = new MenuOnOffSwitch("Measure Panel", this);
	Style::setStyle(measurePanelSwitch->label(), style::properties::label::subtle, false);
	Style::setStyle(measurePanelSwitch->label(), style::properties::label::defaultLabel);
	measurePanelSwitch->onOffswitch()->setChecked(true);
	QHBoxLayout *hlay1 = new QHBoxLayout();
	hlay1->setContentsMargins(0, 6, 0, 6);
	hlay1->setSpacing(9);
	measureSection->contentLayout()->addWidget(measurePanelSwitch);
	measureSection->contentLayout()->addLayout(hlay1);

	QPushButton *showAllMeasure = new QPushButton("Show All", measureSection);
	StyleHelper::BasicSmallButton(showAllMeasure);

	QPushButton *hideAllMeasure = new QPushButton("Hide All", measureSection);
	StyleHelper::BasicSmallButton(hideAllMeasure);

	hideAllMeasure->setVisible(false);

	connect(measurePanelSwitch->onOffswitch(), &QAbstractButton::toggled, this,
		[=](bool b) { Q_EMIT enableMeasurementPanel(b); });
	connect(showAllMeasure, &QPushButton::clicked, this, [=]() {
		Q_EMIT toggleAllMeasurements(true);
		showAllMeasure->setVisible(false);
		hideAllMeasure->setVisible(true);
	});

	connect(hideAllMeasure, &QPushButton::clicked, this, [=]() {
		Q_EMIT toggleAllMeasurements(false);
		hideAllMeasure->setVisible(false);
		showAllMeasure->setVisible(true);
	});
	hlay1->addWidget(showAllMeasure);
	hlay1->addWidget(hideAllMeasure);

	QHBoxLayout *hlay2 = new QHBoxLayout();
	hlay2->setContentsMargins(0, 6, 0, 6);
	hlay2->setSpacing(9);
	measureSection->contentLayout()->addLayout(hlay2);

	QPushButton *mesaureSortByChannel = new QPushButton("Sort by channel", measureSection);
	StyleHelper::BasicSmallButton(mesaureSortByChannel);

	QPushButton *measureSortByType = new QPushButton("Sort by type", measureSection);
	StyleHelper::BasicSmallButton(measureSortByType);

	mesaureSortByChannel->setVisible(false);

	connect(mesaureSortByChannel, &QPushButton::clicked, this, [=]() {
		Q_EMIT sortMeasurements(MPM_SORT_CHANNEL);
		mesaureSortByChannel->setVisible(false);
		measureSortByType->setVisible(true);
	});

	connect(measureSortByType, &QPushButton::clicked, this, [=]() {
		Q_EMIT sortMeasurements(MPM_SORT_TYPE);
		measureSortByType->setVisible(false);
		mesaureSortByChannel->setVisible(true);
	});
	hlay2->addWidget(mesaureSortByChannel);
	hlay2->addWidget(measureSortByType);

	statsSection = new MenuSectionWidget(this);
	Style::setStyle(statsSection, style::properties::widget::border);
	statsPanelSwitch = new MenuOnOffSwitch("Stats Panel", this);
	Style::setStyle(statsPanelSwitch->label(), style::properties::label::subtle, false);
	Style::setStyle(statsPanelSwitch->label(), style::properties::label::defaultLabel);
	connect(statsPanelSwitch->onOffswitch(), &QAbstractButton::toggled, this,
		[=](bool b) { Q_EMIT enableStatsPanel(b); });
	statsSection->contentLayout()->addWidget(statsPanelSwitch);

	statsPanelSwitch->onOffswitch()->setChecked(false);

	QHBoxLayout *hlay3 = new QHBoxLayout();
	hlay3->setContentsMargins(0, 6, 0, 6);
	hlay3->setSpacing(9);
	statsSection->contentLayout()->addWidget(statsPanelSwitch);
	statsSection->contentLayout()->addLayout(hlay3);

	QPushButton *showAllStats = new QPushButton("Show All", statsSection);
	StyleHelper::BasicSmallButton(showAllStats);

	QPushButton *hideAllStats = new QPushButton("Hide All", statsSection);
	StyleHelper::BasicSmallButton(hideAllStats);

	hideAllStats->setVisible(false);

	connect(statsPanelSwitch->onOffswitch(), &QAbstractButton::toggled, this,
		[=](bool b) { Q_EMIT enableStatsPanel(b); });
	connect(showAllStats, &QPushButton::clicked, this, [=]() {
		Q_EMIT toggleAllStats(true);
		showAllStats->setVisible(false);
		hideAllStats->setVisible(true);
	});

	connect(hideAllStats, &QPushButton::clicked, this, [=]() {
		Q_EMIT toggleAllStats(false);
		hideAllStats->setVisible(false);
		showAllStats->setVisible(true);
	});
	hlay3->addWidget(showAllStats);
	hlay3->addWidget(hideAllStats);

	QHBoxLayout *hlay4 = new QHBoxLayout();
	hlay4->setContentsMargins(0, 6, 0, 6);
	hlay4->setSpacing(9);
	statsSection->contentLayout()->addLayout(hlay4);

	QPushButton *statsSortByChannel = new QPushButton("Sort by channel", statsSection);
	StyleHelper::BasicSmallButton(statsSortByChannel);

	QPushButton *statsSortByType = new QPushButton("Sort by type", statsSection);
	StyleHelper::BasicSmallButton(statsSortByType);

	statsSortByChannel->setVisible(false);

	connect(statsSortByChannel, &QPushButton::clicked, this, [=]() {
		Q_EMIT sortStats(MPM_SORT_CHANNEL);
		statsSortByChannel->setVisible(false);
		statsSortByType->setVisible(true);
	});

	connect(statsSortByType, &QPushButton::clicked, this, [=]() {
		Q_EMIT sortStats(MPM_SORT_TYPE);
		statsSortByType->setVisible(false);
		statsSortByChannel->setVisible(true);
	});
	hlay4->addWidget(statsSortByChannel);
	hlay4->addWidget(statsSortByType);

	markerSection = new MenuSectionWidget(this);
	Style::setStyle(markerSection, style::properties::widget::border);
	markerPanelSwitch = new MenuOnOffSwitch("Marker Panel", this);
	Style::setStyle(markerPanelSwitch->label(), style::properties::label::subtle, false);
	Style::setStyle(markerPanelSwitch->label(), style::properties::label::defaultLabel, true, true);

	connect(markerPanelSwitch->onOffswitch(), &QAbstractButton::toggled, this,
		[=](bool b) { Q_EMIT enableMarkerPanel(b); });
	markerSection->contentLayout()->addWidget(markerPanelSwitch);

	markerPanelSwitch->onOffswitch()->setChecked(false);

	genalyzerSection = new MenuSectionWidget(this);
	Style::setStyle(genalyzerSection, style::properties::widget::border);
	genalyzerPanelSwitch = new MenuOnOffSwitch("Genalyzer analysis", this);
	Style::setStyle(genalyzerPanelSwitch->label(), style::properties::label::subtle, false);
	Style::setStyle(genalyzerPanelSwitch->label(), style::properties::label::defaultLabel);

	connect(genalyzerPanelSwitch->onOffswitch(), &QAbstractButton::toggled, this,
		[=](bool b) { Q_EMIT enableGenalyzerPanel(b); });
	genalyzerSection->contentLayout()->addWidget(genalyzerPanelSwitch);

	// Mode selection
	QWidget *modeContainer = new QWidget(this);
	QHBoxLayout *modeLayout = new QHBoxLayout(modeContainer);
	modeLayout->setContentsMargins(0, 0, 0, 0);

	QLabel *modeLabel = new QLabel("Mode:", modeContainer);
	Style::setStyle(modeLabel, style::properties::label::subtle);

	modeCombo = new QComboBox(modeContainer);
	modeCombo->addItem("Auto", 0);  // AUTO mode
	modeCombo->addItem("Fixed Tone", 1);  // FIXED_TONE mode

	modeLayout->addWidget(modeLabel);
	modeLayout->addWidget(modeCombo);
	modeLayout->addStretch();

	genalyzerSection->contentLayout()->addWidget(modeContainer);

	// SSB Width for auto mode
	QWidget *ssbWidthContainer = new QWidget(this);
	QHBoxLayout *ssbLayout = new QHBoxLayout(ssbWidthContainer);
	ssbLayout->setContentsMargins(10, 0, 0, 0);  // Indent for sub-option

	QLabel *ssbLabel = new QLabel("SSB Width:", ssbWidthContainer);
	Style::setStyle(ssbLabel, style::properties::label::subtle);
	ssbWidthSpinbox = new QSpinBox(ssbWidthContainer);
	ssbWidthSpinbox->setRange(0, std::numeric_limits<uint8_t>::max());
	ssbWidthSpinbox->setValue(120);

	ssbLayout->addWidget(ssbLabel);
	ssbLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	ssbLayout->addWidget(ssbWidthSpinbox);
	ssbLayout->addStretch();

	genalyzerSection->contentLayout()->addWidget(ssbWidthContainer);

	// Fixed tone controls container
	QWidget *fixedToneContainer = new QWidget(this);
	QVBoxLayout *ftLayout = new QVBoxLayout(fixedToneContainer);
	ftLayout->setContentsMargins(10, 0, 0, 0);  // Indent for sub-options

	// Expected frequency
	QWidget *freqContainer = new QWidget(fixedToneContainer);
	QHBoxLayout *freqLayout = new QHBoxLayout(freqContainer);
	freqLayout->setContentsMargins(0, 0, 0, 0);

	QLabel *freqLabel = new QLabel("Expected Freq (Hz):", freqContainer);
	Style::setStyle(freqLabel, style::properties::label::subtle);

	expectedFreqEdit = new QLineEdit("1000000", freqContainer);
	expectedFreqEdit->setValidator(new QDoubleValidator(0, 1e12, 6, this));

	freqLayout->addWidget(freqLabel);
	freqLayout->addWidget(expectedFreqEdit);
	ftLayout->addWidget(freqContainer);

	// Harmonic order
	QWidget *harmContainer = new QWidget(fixedToneContainer);
	QHBoxLayout *harmLayout = new QHBoxLayout(harmContainer);
	harmLayout->setContentsMargins(0, 0, 0, 0);

	QLabel *harmLabel = new QLabel("Harmonic Order:", harmContainer);
	Style::setStyle(harmLabel, style::properties::label::subtle);

	harmonicOrderSpinbox = new QSpinBox(harmContainer);
	harmonicOrderSpinbox->setRange(0, 20);
	harmonicOrderSpinbox->setValue(10);

	harmLayout->addWidget(harmLabel);
	harmLayout->addWidget(harmonicOrderSpinbox);
	ftLayout->addWidget(harmContainer);

	// SSB Fundamental
	QWidget *ssbFundContainer = new QWidget(fixedToneContainer);
	QHBoxLayout *ssbFundLayout = new QHBoxLayout(ssbFundContainer);
	ssbFundLayout->setContentsMargins(0, 0, 0, 0);

	QLabel *ssbFundLabel = new QLabel("SSB Fundamental:", ssbFundContainer);
	Style::setStyle(ssbFundLabel, style::properties::label::subtle);

	ssbFundamentalSpinbox = new QSpinBox(ssbFundContainer);
	ssbFundamentalSpinbox->setRange(0, 50);
	ssbFundamentalSpinbox->setValue(4);

	ssbFundLayout->addWidget(ssbFundLabel);
	ssbFundLayout->addWidget(ssbFundamentalSpinbox);
	ftLayout->addWidget(ssbFundContainer);

	// SSB Harmonics
	QWidget *ssbHarmContainer = new QWidget(fixedToneContainer);
	QHBoxLayout *ssbHarmLayout = new QHBoxLayout(ssbHarmContainer);
	ssbHarmLayout->setContentsMargins(0, 0, 0, 0);

	QLabel *ssbHarmLabel = new QLabel("SSB Harmonics:", ssbHarmContainer);
	Style::setStyle(ssbHarmLabel, style::properties::label::subtle);

	ssbHarmonicsSpinbox = new QSpinBox(ssbHarmContainer);
	ssbHarmonicsSpinbox->setRange(0, 50);
	ssbHarmonicsSpinbox->setValue(3);

	ssbHarmLayout->addWidget(ssbHarmLabel);
	ssbHarmLayout->addWidget(ssbHarmonicsSpinbox);
	ftLayout->addWidget(ssbHarmContainer);

	// SSB Default
	QWidget *ssbDefContainer = new QWidget(fixedToneContainer);
	QHBoxLayout *ssbDefLayout = new QHBoxLayout(ssbDefContainer);
	ssbDefLayout->setContentsMargins(0, 0, 0, 0);

	QLabel *ssbDefLabel = new QLabel("SSB Default:", ssbDefContainer);
	Style::setStyle(ssbDefLabel, style::properties::label::subtle);

	ssbDefaultSpinbox = new QSpinBox(ssbDefContainer);
	ssbDefaultSpinbox->setRange(0, 50);
	ssbDefaultSpinbox->setValue(3);

	ssbDefLayout->addWidget(ssbDefLabel);
	ssbDefLayout->addWidget(ssbDefaultSpinbox);
	ftLayout->addWidget(ssbDefContainer);

	// Coherent sampling checkbox
	coherentSamplingCheckbox = new QCheckBox("Coherent Sampling", fixedToneContainer);
	ftLayout->addWidget(coherentSamplingCheckbox);

	genalyzerSection->contentLayout()->addWidget(fixedToneContainer);

	// Connect mode change to show/hide appropriate controls
	connect(modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
		bool isFixedTone = (modeCombo->currentData().toInt() == 1);  // 1 = FIXED_TONE mode
		fixedToneContainer->setVisible(isFixedTone);
		ssbWidthContainer->setVisible(!isFixedTone);  // Show SSB width only for auto mode
		onGenalyzerUIChanged();
	});

	// Connect all UI changes
	connect(ssbWidthSpinbox, QOverload<int>::of(&QSpinBox::valueChanged),
		this, &MeasurementSettings::onGenalyzerUIChanged);
	connect(expectedFreqEdit, &QLineEdit::textChanged, this, &MeasurementSettings::onGenalyzerUIChanged);
	connect(harmonicOrderSpinbox, QOverload<int>::of(&QSpinBox::valueChanged),
		this, &MeasurementSettings::onGenalyzerUIChanged);
	connect(ssbFundamentalSpinbox, QOverload<int>::of(&QSpinBox::valueChanged),
		this, &MeasurementSettings::onGenalyzerUIChanged);
	connect(ssbHarmonicsSpinbox, QOverload<int>::of(&QSpinBox::valueChanged),
		this, &MeasurementSettings::onGenalyzerUIChanged);
	connect(ssbDefaultSpinbox, QOverload<int>::of(&QSpinBox::valueChanged),
		this, &MeasurementSettings::onGenalyzerUIChanged);
	connect(coherentSamplingCheckbox, &QCheckBox::toggled,
		this, &MeasurementSettings::onGenalyzerUIChanged);

	// Initialize visibility
	fixedToneContainer->setVisible(false);

	// Emit initial configuration
	Q_EMIT genalyzerConfigChanged(getGenalyzerConfig());

	genalyzerPanelSwitch->onOffswitch()->setChecked(false);

	lay->addWidget(measureSection);
	lay->addWidget(statsSection);
	lay->addWidget(markerSection);
	lay->addWidget(genalyzerSection);
}

MeasurementSettings::~MeasurementSettings() {}

bool MeasurementSettings::measurementEnabled() { return measurePanelSwitch->onOffswitch()->isChecked(); }
bool MeasurementSettings::statsEnabled() { return statsPanelSwitch->onOffswitch()->isChecked(); }
bool MeasurementSettings::markerEnabled() { return markerPanelSwitch->onOffswitch()->isChecked(); }
bool MeasurementSettings::genalyzerEnabled() { return genalyzerPanelSwitch->onOffswitch()->isChecked(); }
uint8_t MeasurementSettings::ssbWidth() const { return m_auto_ssb_width; }

QVariantMap MeasurementSettings::getGenalyzerConfig() const
{
	QVariantMap config;
	config["mode"] = m_mode;
	config["auto_ssb_width"] = m_auto_ssb_width;
	config["expected_freq"] = m_expected_freq;
	config["harmonic_order"] = m_harmonic_order;
	config["ssb_fundamental"] = m_ssb_fundamental;
	config["ssb_harmonics"] = m_ssb_harmonics;
	config["ssb_default"] = m_ssb_default;
	config["coherent_sampling"] = m_coherent_sampling;
	config["component_label"] = QString("A");  // Default label
	return config;
}

void MeasurementSettings::setGenalyzerConfig(const QVariantMap& config)
{
	if (config.contains("mode")) m_mode = config["mode"].toInt();
	if (config.contains("auto_ssb_width")) m_auto_ssb_width = config["auto_ssb_width"].toUInt();
	if (config.contains("expected_freq")) m_expected_freq = config["expected_freq"].toDouble();
	if (config.contains("harmonic_order")) m_harmonic_order = config["harmonic_order"].toInt();
	if (config.contains("ssb_fundamental")) m_ssb_fundamental = config["ssb_fundamental"].toInt();
	if (config.contains("ssb_harmonics")) m_ssb_harmonics = config["ssb_harmonics"].toInt();
	if (config.contains("ssb_default")) m_ssb_default = config["ssb_default"].toInt();
	if (config.contains("coherent_sampling")) m_coherent_sampling = config["coherent_sampling"].toBool();
	updateUIFromConfig();
}

void MeasurementSettings::onGenalyzerUIChanged()
{
	// Update configuration from UI
	m_mode = modeCombo->currentData().toInt();

	if (m_mode == 0) {  // AUTO mode
		m_auto_ssb_width = static_cast<uint8_t>(ssbWidthSpinbox->value());
	} else {  // FIXED_TONE mode
		// Fixed tone parameters
		m_expected_freq = expectedFreqEdit->text().toDouble();
		m_harmonic_order = harmonicOrderSpinbox->value();
		m_ssb_fundamental = ssbFundamentalSpinbox->value();
		m_ssb_harmonics = ssbHarmonicsSpinbox->value();
		m_ssb_default = ssbDefaultSpinbox->value();
		m_coherent_sampling = coherentSamplingCheckbox->isChecked();
	}

	// Emit the configuration change
	Q_EMIT genalyzerConfigChanged(getGenalyzerConfig());

	// Also emit deprecated signal for backward compatibility
	Q_EMIT ssbWidthChanged(m_auto_ssb_width);
}

void MeasurementSettings::updateUIFromConfig()
{
	// Block signals to prevent recursive updates
	modeCombo->blockSignals(true);
	ssbWidthSpinbox->blockSignals(true);
	expectedFreqEdit->blockSignals(true);
	harmonicOrderSpinbox->blockSignals(true);
	ssbFundamentalSpinbox->blockSignals(true);
	ssbHarmonicsSpinbox->blockSignals(true);
	ssbDefaultSpinbox->blockSignals(true);
	coherentSamplingCheckbox->blockSignals(true);

	// Update UI from config
	modeCombo->setCurrentIndex(modeCombo->findData(m_mode));
	ssbWidthSpinbox->setValue(m_auto_ssb_width);
	expectedFreqEdit->setText(QString::number(m_expected_freq));
	harmonicOrderSpinbox->setValue(m_harmonic_order);
	ssbFundamentalSpinbox->setValue(m_ssb_fundamental);
	ssbHarmonicsSpinbox->setValue(m_ssb_harmonics);
	ssbDefaultSpinbox->setValue(m_ssb_default);
	coherentSamplingCheckbox->setChecked(m_coherent_sampling);

	// Update visibility
	bool isFixedTone = (m_mode == 1);  // 1 = FIXED_TONE mode
	auto fixedToneContainer = coherentSamplingCheckbox->parentWidget();
	auto ssbWidthContainer = ssbWidthSpinbox->parentWidget();
	if (fixedToneContainer) fixedToneContainer->setVisible(isFixedTone);
	if (ssbWidthContainer) ssbWidthContainer->setVisible(!isFixedTone);

	// Restore signals
	modeCombo->blockSignals(false);
	ssbWidthSpinbox->blockSignals(false);
	expectedFreqEdit->blockSignals(false);
	harmonicOrderSpinbox->blockSignals(false);
	ssbFundamentalSpinbox->blockSignals(false);
	ssbHarmonicsSpinbox->blockSignals(false);
	ssbDefaultSpinbox->blockSignals(false);
	coherentSamplingCheckbox->blockSignals(false);
}

MenuSectionWidget *MeasurementSettings::getMarkerSection() const { return markerSection; }

MenuSectionWidget *MeasurementSettings::getStatsSection() const { return statsSection; }

MenuSectionWidget *MeasurementSettings::getMeasureSection() const { return measureSection; }

MenuSectionWidget *MeasurementSettings::getGenalyzerSection() const { return genalyzerSection; }

#include "moc_measurementsettings.cpp"
