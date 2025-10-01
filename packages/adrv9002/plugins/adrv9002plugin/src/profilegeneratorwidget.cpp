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

#include <profilegeneratorwidget.h>

#include <QLoggingCategory>
#include <QFileDialog>
#include <QStandardPaths>
#include <stylehelper.h>

Q_LOGGING_CATEGORY(CAT_PROFILEGENERATORWIDGET, "ProfileGeneratorWidget")

using namespace scopy::adrv9002;
using namespace scopy;

// Frequency table from iio-oscilloscope (exact Hz values)
const QStringList FrequencyTable::SAMPLE_RATES_HZ = {
	"1920000", "3840000", "7680000", "15360000", "30720000", "61440000"
};

const QStringList FrequencyTable::BANDWIDTHS_HZ = {
	"1008000", "2700000", "4500000", "9000000", "18000000", "38000000"
};

// RF Input options (descriptive names from screenshots)
const QStringList RFInputOptions::RX1_OPTIONS = {
        "Rx1A", "Rx1B"
};

const QStringList RFInputOptions::RX2_OPTIONS = {
        "Rx2A", "Rx2B"
};

// LTE defaults from lte_defaults() function (exact values)
const int LTEDefaults::SAMPLE_RATE_HZ = 61440000;
const int LTEDefaults::BANDWIDTH_HZ = 38000000;
const bool LTEDefaults::ENABLED = true;
const bool LTEDefaults::FREQ_OFFSET_CORRECTION = false;
const int LTEDefaults::RF_PORT = 0; // Maps to Rx1A/Rx2A

const QString LTEDefaults::SSI_INTERFACE = "LVDS";
const QString LTEDefaults::DUPLEX_MODE = "TDD";
const int LTEDefaults::SSI_LANES = 2;

const int LTEDefaults::DEVICE_CLOCK_FREQUENCY_KHZ = 38400;
const bool LTEDefaults::DEVICE_CLOCK_OUTPUT_ENABLE = true;
const int LTEDefaults::DEVICE_CLOCK_OUTPUT_DIVIDER = 2;

// Interface options
const QStringList IOOOptions::SSI_INTERFACE_OPTIONS = {
	"CMOS", "LVDS"
};

const QStringList IOOOptions::DUPLEX_MODE_OPTIONS = {
	"TDD", "FDD"
};

// Frequency table helper functions
int FrequencyTable::getIndexOfSampleRate(const QString &sampleRate)
{
	return SAMPLE_RATES_HZ.indexOf(sampleRate);
}

QString FrequencyTable::getBandwidthForSampleRate(const QString &sampleRate)
{
	int index = getIndexOfSampleRate(sampleRate);
	if (index >= 0 && index < BANDWIDTHS_HZ.size()) {
		return BANDWIDTHS_HZ[index];
	}
	return "38000000"; // Default fallback
}

QStringList FrequencyTable::getSampleRatesForSSILanes(int ssiLanes)
{
	// Based on iio-oscilloscope logic in profile_gen_config_set_LTE()
	if (ssiLanes == 1) {
		return QStringList(); // No options for 1 lane
	} else if (ssiLanes == 2) {
		return SAMPLE_RATES_HZ; // All 6 options for 2 lanes (LVDS)
	} else if (ssiLanes == 4) {
		return QStringList() << SAMPLE_RATES_HZ.first(); // Only first option for 4 lanes (CMOS)
	}
	return SAMPLE_RATES_HZ; // Default to all options
}

ProfileGeneratorWidget::ProfileGeneratorWidget(iio_device *device, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
	, m_cliAvailable(false)
	, m_statusLabel(nullptr)
	, m_contentWidget(nullptr)
	, m_lastAppliedPreset("")
	, m_updatingFromPreset(false)
{
	setupUi();
}

ProfileGeneratorWidget::~ProfileGeneratorWidget() {}

void ProfileGeneratorWidget::setupUi()
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setSpacing(15); // Increased section spacing from prompt pattern
	mainLayout->setContentsMargins(10, 10, 10, 10);

	// Step 1: Detect CLI availability
	m_cliAvailable = detectProfileGeneratorCLI();

	// Step 2: Create status notification (always at top)
	m_statusLabel = new QLabel(this);
	mainLayout->addWidget(m_statusLabel);

	// Step 3: Create content widget (all controls go here)
	m_contentWidget = new QWidget(this);
	QVBoxLayout *contentLayout = new QVBoxLayout(m_contentWidget);
	contentLayout->setSpacing(15); // Standard section spacing from prompt pattern

	// Add all the profile generator controls to content widget
	contentLayout->addWidget(createProfileActionBar());
	contentLayout->addWidget(createRadioConfigSection());
	contentLayout->addWidget(createChannelConfigSection());
	contentLayout->addWidget(createOrxConfigSection());
	contentLayout->addWidget(createDebugInfoSection());

	// Add stretch to push content to top (from prompt pattern)
	contentLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	mainLayout->addWidget(m_contentWidget);

	// Step 4: Configure UI based on CLI availability
	setupUIBasedOnCLIAvailability();

	// Step 5: Setup connections (only if CLI available)
	if(m_cliAvailable) {
		setupConnections();
		// Initialize with default LTE preset
		loadPresetData("LTE");
		// Initialize control states
		updateChannelControlsVisibility();
		updateOrxControlsState();
	}
}

bool ProfileGeneratorWidget::detectProfileGeneratorCLI()
{
	// Search for CLI tool in common locations
	QStringList searchPaths = {"/usr/bin/", "/usr/local/bin/", "/opt/analog/bin/",
				   QCoreApplication::applicationDirPath() + "/", QDir::homePath() + "/.local/bin/"};

	QStringList cliNames = {"adrv9002-iio-cli", // Primary CLI tool found on this system
				"adrv9002_profile_generator", "profile_gen_cli", "adrv9002-profile-gen"};

	for(const QString &path : searchPaths) {
		for(const QString &cliName : cliNames) {
			QString fullPath = path + cliName;
			if(QFile::exists(fullPath) && QFileInfo(fullPath).isExecutable()) {
				m_cliPath = fullPath;
				return validateCLIVersion();
			}
		}
	}

	// Also check PATH environment variable
	QProcess process;
	for(const QString &cliName : cliNames) {
		process.start("which", QStringList() << cliName);
		process.waitForFinished(3000);
		if(process.exitCode() == 0) {
			m_cliPath = process.readAllStandardOutput().trimmed();
			return validateCLIVersion();
		}
	}

	return false;
}

bool ProfileGeneratorWidget::validateCLIVersion()
{
	QProcess process;
	process.start(m_cliPath, QStringList() << "--version");
	process.waitForFinished(5000);

	if(process.exitCode() != 0) {
		// Try alternative version check
		process.start(m_cliPath, QStringList() << "-v");
		process.waitForFinished(5000);
		if(process.exitCode() != 0) {
			return false;
		}
	}

	m_cliVersion = process.readAllStandardOutput().trimmed();
	if(m_cliVersion.isEmpty()) {
		m_cliVersion = "unknown";
	}

	qDebug(CAT_PROFILEGENERATORWIDGET) << "Found Profile Generator CLI:" << m_cliPath << "Version:" << m_cliVersion;
	return true;
}

void ProfileGeneratorWidget::setupUIBasedOnCLIAvailability()
{
	m_statusLabel->setWordWrap(true);
	m_statusLabel->setMargin(10);

	if(m_cliAvailable) {
		// CLI available - show success status
		m_statusLabel->setText(QString("✓ Profile Generator CLI detected: %1 (%2)")
					       .arg(QFileInfo(m_cliPath).fileName())
					       .arg(m_cliVersion));
		m_statusLabel->setStyleSheet("QLabel { "
					     "background-color: #d4edda; "
					     "color: #155724; "
					     "border: 1px solid #c3e6cb; "
					     "border-radius: 4px; "
					     "padding: 8px; "
					     "}");

		// Enable all functionality
		m_contentWidget->setEnabled(true);

	} else {
		// CLI not available - show error status
		m_statusLabel->setText(
			"⚠ Profile Generator CLI not found. "
			"Advanced profile generation features are disabled. "
			"Please install the ADRV9002 Profile Generator CLI tool to enable this functionality. "
			"Refer to the ADI documentation for installation instructions.");
		m_statusLabel->setStyleSheet("QLabel { "
					     "background-color: #f8d7da; "
					     "color: #721c24; "
					     "border: 1px solid #f5c6cb; "
					     "border-radius: 4px; "
					     "padding: 8px; "
					     "}");

		// Disable all controls
		m_contentWidget->setEnabled(false);

		// Add download info button with proper Scopy styling right under the error message
		QPushButton *infoBtn = new QPushButton("Download Profile Generator CLI", this);
		Style::setStyle(infoBtn, style::properties::button::basicButton);
		infoBtn->setMinimumWidth(250);
		infoBtn->setMaximumWidth(300);
		connect(infoBtn, &QPushButton::clicked, this, &ProfileGeneratorWidget::onDownloadCLI);

		// Insert button right after the status label (position 1 in mainLayout)
		static_cast<QVBoxLayout *>(layout())->insertWidget(1, infoBtn);
	}
}

QWidget *ProfileGeneratorWidget::createProfileActionBar()
{
	QWidget *actionBar = new QWidget();
	// Apply prompt pattern: section background and border styling
	Style::setBackgroundColor(actionBar, json::theme::background_primary);
	Style::setStyle(actionBar, style::properties::widget::border_interactive);

	QHBoxLayout *layout = new QHBoxLayout(actionBar);
	layout->setContentsMargins(15, 15, 15, 15); // Standard margins from prompt
	layout->setSpacing(15);

	// Preset dropdown with styling - matching iio-oscilloscope exactly
	QLabel *presetLabel = new QLabel("Preset:");
	Style::setStyle(presetLabel, style::properties::label::menuSmall);

	m_presetCombo = new QComboBox();
	m_presetCombo->addItems({"Live Device", "LTE"});
	m_presetCombo->setMinimumWidth(120);

	// Action buttons with proper Scopy styling
	m_refreshProfileBtn = new QPushButton("Refresh");
	m_saveToFileBtn = new QPushButton("Save to file");
	m_loadToDeviceBtn = new QPushButton("Load to device");

	// Apply Scopy button styling from prompt patterns
	Style::setStyle(m_refreshProfileBtn, style::properties::button::basicButton);
	Style::setStyle(m_saveToFileBtn, style::properties::button::basicButton);
	Style::setStyle(m_loadToDeviceBtn, style::properties::button::basicButton);

	// Set minimum button widths for consistency
	m_refreshProfileBtn->setMinimumWidth(100);
	m_saveToFileBtn->setMinimumWidth(120);
	m_loadToDeviceBtn->setMinimumWidth(130);

	// Layout with proper spacing - matching iio-oscilloscope exactly
	layout->addWidget(presetLabel);
	layout->addWidget(m_presetCombo);
	layout->addWidget(m_refreshProfileBtn);
	layout->addStretch();
	layout->addWidget(m_saveToFileBtn);
	layout->addWidget(m_loadToDeviceBtn);

	return actionBar;
}

QWidget *ProfileGeneratorWidget::createRadioConfigSection()
{
	QWidget *section = new QWidget();
	// Apply prompt pattern: individual section background and border
	Style::setBackgroundColor(section, json::theme::background_primary);
	Style::setStyle(section, style::properties::widget::border_interactive);

	QGridLayout *layout = new QGridLayout(section);
	layout->setSpacing(10);			    // Standard Scopy spacing from prompt
	layout->setContentsMargins(15, 15, 15, 15); // Standard margins from prompt

	// Title
	QLabel *title = new QLabel("Radio Config");
	Style::setStyle(title, style::properties::label::menuBig);
	layout->addWidget(title, 0, 0, 1, 4);

	// SSI Interface - using correct IOO options
	layout->addWidget(new QLabel("SSI Interface:"), 1, 0);
	m_ssiInterfaceCombo = new QComboBox();
	m_ssiInterfaceCombo->addItems(IOOOptions::SSI_INTERFACE_OPTIONS);
	layout->addWidget(m_ssiInterfaceCombo, 1, 1);

	// Duplex mode - using correct IOO options
	layout->addWidget(new QLabel("Duplex mode:"), 1, 2);
	m_duplexModeCombo = new QComboBox();
	m_duplexModeCombo->addItems(IOOOptions::DUPLEX_MODE_OPTIONS);
	layout->addWidget(m_duplexModeCombo, 1, 3);

	return section;
}

QWidget *ProfileGeneratorWidget::createChannelConfigSection()
{
	QWidget *section = new QWidget();
	// Apply prompt pattern: section background and border
	Style::setBackgroundColor(section, json::theme::background_primary);
	Style::setStyle(section, style::properties::widget::border_interactive);

	QGridLayout *layout = new QGridLayout(section);
	layout->setSpacing(15);
	layout->setContentsMargins(15, 15, 15, 15); // Standard margins from prompt

	// Create 2x2 grid: RX1, RX2, TX1, TX2
	layout->addWidget(createChannelConfigWidget("RX 1", CHANNEL_RX1), 0, 0);
	layout->addWidget(createChannelConfigWidget("RX 2", CHANNEL_RX2), 0, 1);
	layout->addWidget(createChannelConfigWidget("TX 1", CHANNEL_TX1), 1, 0);
	layout->addWidget(createChannelConfigWidget("TX 2", CHANNEL_TX2), 1, 1);

	return section;
}

QWidget *ProfileGeneratorWidget::createChannelConfigWidget(const QString &title, ChannelType type)
{
	QWidget *widget = new QWidget();
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QGridLayout *layout = new QGridLayout(widget);
	layout->setContentsMargins(15, 15, 15, 15); // Standard margins from prompt
	layout->setSpacing(10);			    // Standard spacing from prompt

	// Title
	QLabel *titleLabel = new QLabel(title);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	layout->addWidget(titleLabel, 0, 0, 1, 2);

	// Get widgets for this channel
	ChannelWidgets *widgets = nullptr;
	switch(type) {
	case CHANNEL_RX1:
		widgets = &m_rx1Widgets;
		break;
	case CHANNEL_RX2:
		widgets = &m_rx2Widgets;
		break;
	case CHANNEL_TX1:
		widgets = &m_tx1Widgets;
		break;
	case CHANNEL_TX2:
		widgets = &m_tx2Widgets;
		break;
	}

	// Enabled checkbox
	widgets->enabledCb = new QCheckBox("Enabled");
	widgets->enabledCb->setChecked(true); // Default enabled
	layout->addWidget(widgets->enabledCb, 1, 0, 1, 2);

	// Frequency Offset Correction
	widgets->freqOffsetCb = new QCheckBox("Frequency Offset Correction");
	layout->addWidget(widgets->freqOffsetCb, 2, 0, 1, 2);

	// Bandwidth (Hz) - matching iio-oscilloscope exactly
	layout->addWidget(new QLabel("Bandwidth (Hz):"), 3, 0);
	widgets->bandwidthCombo = new QComboBox();
	widgets->bandwidthCombo->addItems(FrequencyTable::BANDWIDTHS_HZ);
	layout->addWidget(widgets->bandwidthCombo, 3, 1);

	// Interface Sample Rate (Hz) - matching iio-oscilloscope exactly
	layout->addWidget(new QLabel("Interface Sample Rate (Hz):"), 4, 0);
	widgets->sampleRateCombo = new QComboBox();
	widgets->sampleRateCombo->addItems(FrequencyTable::SAMPLE_RATES_HZ);
	layout->addWidget(widgets->sampleRateCombo, 4, 1);

	// RX RF Input (only for RX channels) - using descriptive names
	if(type == CHANNEL_RX1 || type == CHANNEL_RX2) {
		layout->addWidget(new QLabel("RX RF Input:"), 5, 0);
		widgets->rfInputCombo = new QComboBox();
		if(type == CHANNEL_RX1) {
			widgets->rfInputCombo->addItems(RFInputOptions::RX1_OPTIONS);
		} else {
			widgets->rfInputCombo->addItems(RFInputOptions::RX2_OPTIONS);
		}
		layout->addWidget(widgets->rfInputCombo, 5, 1);
	} else {
		widgets->rfInputCombo = nullptr;
	}

	return widget;
}

QWidget *ProfileGeneratorWidget::createOrxConfigSection()
{
	QWidget *section = new QWidget();
	// Apply prompt pattern: section background and border
	Style::setBackgroundColor(section, json::theme::background_primary);
	Style::setStyle(section, style::properties::widget::border_interactive);

	QHBoxLayout *layout = new QHBoxLayout(section);
	layout->setSpacing(15);
	layout->setContentsMargins(15, 15, 15, 15); // Standard margins from prompt

	// ORX 1
	layout->addWidget(createOrxWidget("ORX 1"));
	// ORX 2
	layout->addWidget(createOrxWidget("ORX 2"));

	return section;
}

QWidget *ProfileGeneratorWidget::createOrxWidget(const QString &title)
{
	QWidget *widget = new QWidget();
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setContentsMargins(15, 15, 15, 15); // Standard margins from prompt
	layout->setSpacing(10);			    // Standard spacing from prompt

	QLabel *titleLabel = new QLabel(title);
	Style::setStyle(titleLabel, style::properties::label::menuBig);
	layout->addWidget(titleLabel);

	QCheckBox *enabledCb = new QCheckBox("Enabled");
	enabledCb->setChecked(false); // Default disabled
	layout->addWidget(enabledCb);

	// Store reference to checkbox
	if(title == "ORX 1") {
		m_orx1EnabledCb = enabledCb;
	} else {
		m_orx2EnabledCb = enabledCb;
	}

	return widget;
}

QWidget *ProfileGeneratorWidget::createDebugInfoSection()
{
	QWidget *section = new QWidget();
	// Apply prompt pattern: section background and border
	Style::setBackgroundColor(section, json::theme::background_primary);
	Style::setStyle(section, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(section);
	layout->setContentsMargins(15, 15, 15, 15); // Standard margins from prompt
	layout->setSpacing(10);			    // Standard spacing from prompt

	QLabel *title = new QLabel("Debug Info");
	Style::setStyle(title, style::properties::label::menuBig);
	layout->addWidget(title);

	m_debugInfoText = new QTextEdit();
	m_debugInfoText->setReadOnly(true);
	m_debugInfoText->setMaximumHeight(200);
	m_debugInfoText->setFont(QFont("monospace"));
	layout->addWidget(m_debugInfoText);

	return section;
}

void ProfileGeneratorWidget::setupConnections()
{
	// Action buttons
	connect(m_presetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&ProfileGeneratorWidget::onPresetChanged);
	connect(m_refreshProfileBtn, &QPushButton::clicked, this, &ProfileGeneratorWidget::onRefreshProfile);
	connect(m_saveToFileBtn, &QPushButton::clicked, this, &ProfileGeneratorWidget::onSaveToFile);
	connect(m_loadToDeviceBtn, &QPushButton::clicked, this, &ProfileGeneratorWidget::onLoadToDevice);

	// Sample rate change signals for bandwidth updates
	connect(m_rx1Widgets.sampleRateCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
		this, [this]() { onSampleRateChanged(CHANNEL_RX1); });
	connect(m_rx2Widgets.sampleRateCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
		this, [this]() { onSampleRateChanged(CHANNEL_RX2); });
	connect(m_tx1Widgets.sampleRateCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
		this, [this]() { onSampleRateChanged(CHANNEL_TX1); });
	connect(m_tx2Widgets.sampleRateCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
		this, [this]() { onSampleRateChanged(CHANNEL_TX2); });

	// Data change signals
	connect(m_ssiInterfaceCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged), this,
		&ProfileGeneratorWidget::updateProfileData);
	connect(m_duplexModeCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged), this,
		&ProfileGeneratorWidget::updateProfileData);

	// Signal dependencies (critical for iio-oscilloscope compatibility)
	connect(m_duplexModeCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged), this,
		&ProfileGeneratorWidget::onTddModeChanged);
	connect(m_ssiInterfaceCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged), this,
		&ProfileGeneratorWidget::onInterfaceChanged);

	// Channel enable changes affect UI controls and ORX state
	connect(m_rx1Widgets.enabledCb, &QCheckBox::toggled, this, &ProfileGeneratorWidget::onChannelEnableChanged);
	connect(m_rx2Widgets.enabledCb, &QCheckBox::toggled, this, &ProfileGeneratorWidget::onChannelEnableChanged);
	connect(m_tx1Widgets.enabledCb, &QCheckBox::toggled, this, &ProfileGeneratorWidget::onTxEnableChanged);
	connect(m_tx2Widgets.enabledCb, &QCheckBox::toggled, this, &ProfileGeneratorWidget::onTxEnableChanged);

	// ORX enable changes
	connect(m_orx1EnabledCb, &QCheckBox::toggled, this, &ProfileGeneratorWidget::updateProfileData);
	connect(m_orx2EnabledCb, &QCheckBox::toggled, this, &ProfileGeneratorWidget::updateProfileData);
}

void ProfileGeneratorWidget::loadPresetData(const QString &presetName)
{
	if(presetName == "LTE") {
		// Apply LTE defaults exactly matching lte_defaults() function
		applyLTEDefaults();

		// Enable RX1 and RX2
		m_rx1Widgets.enabledCb->setChecked(true);
		m_rx2Widgets.enabledCb->setChecked(true);
		m_rx1Widgets.bandwidthCombo->setCurrentText("38000000");
		m_rx2Widgets.bandwidthCombo->setCurrentText("38000000");
		m_rx1Widgets.sampleRateCombo->setCurrentText("61440000");
		m_rx2Widgets.sampleRateCombo->setCurrentText("61440000");

		// Enable TX1 and TX2
		m_tx1Widgets.enabledCb->setChecked(true);
		m_tx2Widgets.enabledCb->setChecked(true);
		m_tx1Widgets.bandwidthCombo->setCurrentText("38000000");
		m_tx2Widgets.bandwidthCombo->setCurrentText("38000000");
		m_tx1Widgets.sampleRateCombo->setCurrentText("61440000");
		m_tx2Widgets.sampleRateCombo->setCurrentText("61440000");

		// Disable ORX by default
		m_orx1EnabledCb->setChecked(false);
		m_orx2EnabledCb->setChecked(false);
	}

	updateProfileData();
}

void ProfileGeneratorWidget::updateProfileData()
{
	updateConfigFromUI();
	updateDebugInfo();
}

void ProfileGeneratorWidget::updateConfigFromUI()
{
	// Radio config
	m_radioConfig.lvds = (m_ssiInterfaceCombo->currentText() == "LVDS");
	m_radioConfig.fdd = (m_duplexModeCombo->currentText() == "FDD");
	m_radioConfig.ssi_lanes = m_radioConfig.lvds ? 2 : 4;

	// RX channels
	m_radioConfig.rx_config[0].enabled = m_rx1Widgets.enabledCb->isChecked();
	m_radioConfig.rx_config[0].freqOffsetCorrection = m_rx1Widgets.freqOffsetCb->isChecked();
	m_radioConfig.rx_config[0].bandwidth = m_rx1Widgets.bandwidthCombo->currentText();
	m_radioConfig.rx_config[0].sampleRate = m_rx1Widgets.sampleRateCombo->currentText();
	if(m_rx1Widgets.rfInputCombo) {
		m_radioConfig.rx_config[0].rfInput = m_rx1Widgets.rfInputCombo->currentText();
	}

	m_radioConfig.rx_config[1].enabled = m_rx2Widgets.enabledCb->isChecked();
	m_radioConfig.rx_config[1].freqOffsetCorrection = m_rx2Widgets.freqOffsetCb->isChecked();
	m_radioConfig.rx_config[1].bandwidth = m_rx2Widgets.bandwidthCombo->currentText();
	m_radioConfig.rx_config[1].sampleRate = m_rx2Widgets.sampleRateCombo->currentText();
	if(m_rx2Widgets.rfInputCombo) {
		m_radioConfig.rx_config[1].rfInput = m_rx2Widgets.rfInputCombo->currentText();
	}

	// TX channels
	m_radioConfig.tx_config[0].enabled = m_tx1Widgets.enabledCb->isChecked();
	m_radioConfig.tx_config[0].freqOffsetCorrection = m_tx1Widgets.freqOffsetCb->isChecked();
	m_radioConfig.tx_config[0].bandwidth = m_tx1Widgets.bandwidthCombo->currentText();
	m_radioConfig.tx_config[0].sampleRate = m_tx1Widgets.sampleRateCombo->currentText();

	m_radioConfig.tx_config[1].enabled = m_tx2Widgets.enabledCb->isChecked();
	m_radioConfig.tx_config[1].freqOffsetCorrection = m_tx2Widgets.freqOffsetCb->isChecked();
	m_radioConfig.tx_config[1].bandwidth = m_tx2Widgets.bandwidthCombo->currentText();
	m_radioConfig.tx_config[1].sampleRate = m_tx2Widgets.sampleRateCombo->currentText();

	// ORX
	m_radioConfig.orx_enabled[0] = m_orx1EnabledCb->isChecked();
	m_radioConfig.orx_enabled[1] = m_orx2EnabledCb->isChecked();
}

void ProfileGeneratorWidget::updateDebugInfo()
{
	QJsonObject config = generateBasicProfile();
	QJsonDocument doc(config);
	m_debugInfoText->setPlainText(doc.toJson(QJsonDocument::Indented));
}

QJsonObject ProfileGeneratorWidget::generateBasicProfile()
{
	QJsonObject profile;

	// Radio configuration section
	QJsonObject radioConfig;
	radioConfig["ssi_type"] = m_radioConfig.lvds ? "LVDS" : "CMOS";
	radioConfig["ssi_lanes"] = m_radioConfig.ssi_lanes;
	radioConfig["duplex_mode"] = m_radioConfig.fdd ? "FDD" : "TDD";
	radioConfig["ddr"] = 1;

	// Channel configurations
	QJsonArray rxChannels, txChannels;
	for(int i = 0; i < 2; i++) {
		QJsonObject rxCh;
		rxCh["enabled"] = m_radioConfig.rx_config[i].enabled;
		rxCh["bandwidth_hz"] = m_radioConfig.rx_config[i].bandwidth.toInt();
		rxCh["sample_rate_hz"] = m_radioConfig.rx_config[i].sampleRate.toInt();
		rxCh["frequency_offset_correction"] = m_radioConfig.rx_config[i].freqOffsetCorrection;
		rxCh["rf_input"] = m_radioConfig.rx_config[i].rfInput;
		rxChannels.append(rxCh);

		QJsonObject txCh;
		txCh["enabled"] = m_radioConfig.tx_config[i].enabled;
		txCh["bandwidth_hz"] = m_radioConfig.tx_config[i].bandwidth.toInt();
		txCh["sample_rate_hz"] = m_radioConfig.tx_config[i].sampleRate.toInt();
		txCh["frequency_offset_correction"] = m_radioConfig.tx_config[i].freqOffsetCorrection;
		txCh["orx_enabled"] = m_radioConfig.orx_enabled[i];
		txChannels.append(txCh);
	}

	profile["radio_cfg"] = radioConfig;
	profile["rx_channels"] = rxChannels;
	profile["tx_channels"] = txChannels;

	return profile;
}

// Slot implementations
void ProfileGeneratorWidget::onPresetChanged()
{
	QString presetName = m_presetCombo->currentText();
	loadPresetData(presetName);
}

void ProfileGeneratorWidget::onRefreshProfile()
{
	if(!m_cliAvailable) {
		StatusBarManager::pushMessage("Profile Generator CLI not available", 5000);
		return;
	}

	// Refresh current configuration
	updateProfileData();
	StatusBarManager::pushMessage("Profile refreshed", 3000);
}

void ProfileGeneratorWidget::onSaveToFile()
{
	if(!m_cliAvailable) {
		StatusBarManager::pushMessage("Profile Generator CLI not available - cannot generate profile files",
					      5000);
		return;
	}

	QString fileName = QFileDialog::getSaveFileName(
		this, "Save Profile", QDir::homePath() + "/adrv9002_profile.json", "JSON Files (*.json)");

	if(!fileName.isEmpty()) {
		if(saveProfileToFile(fileName)) {
			StatusBarManager::pushMessage("Profile saved successfully", 3000);
		} else {
			StatusBarManager::pushMessage("Failed to save profile", 5000);
		}
	}
}

void ProfileGeneratorWidget::onLoadToDevice()
{
	if(!m_cliAvailable) {
		StatusBarManager::pushMessage("Profile Generator CLI not available - cannot load to device", 5000);
		return;
	}

	if(loadProfileToDevice()) {
		StatusBarManager::pushMessage("Profile loaded to device successfully", 3000);
	} else {
		StatusBarManager::pushMessage("Failed to load profile to device", 5000);
	}
}

void ProfileGeneratorWidget::onDownloadCLI()
{
	QDesktopServices::openUrl(QUrl("https://analogdevicesinc.github.io/libadrv9002-iio"));
}

void ProfileGeneratorWidget::refreshProfileData() { onRefreshProfile(); }

// CLI-based operations following iio-oscilloscope pattern
bool ProfileGeneratorWidget::generateProfile()
{
	if(!m_cliAvailable) {
		return false;
	}

	// Create temp files
	QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
	QString configFile = tempDir + "/adrv9002_config.json";
	QString profileFile = tempDir + "/adrv9002_profile.json";
	QString streamFile = tempDir + "/adrv9002_stream.json";

	// Write config to temp file
	if(!writeConfigToFile(configFile)) {
		return false;
	}

	// Run CLI command
	QString command = QString("%1 --config %2 --profile %3 --stream %4")
				  .arg(m_cliPath)
				  .arg(configFile)
				  .arg(profileFile)
				  .arg(streamFile);

	QProcess process;
	process.start(command);
	process.waitForFinished(30000); // 30 second timeout

	// Cleanup config file
	QFile::remove(configFile);

	if(process.exitCode() != 0) {
		qWarning(CAT_PROFILEGENERATORWIDGET) << "CLI command failed:" << process.readAllStandardError();
		return false;
	}

	// Store file paths for later use
	m_currentProfilePath = profileFile;
	m_currentStreamPath = streamFile;

	return true;
}

bool ProfileGeneratorWidget::loadProfileToDevice()
{
	if(!generateProfile()) {
		return false;
	}

	// Read generated profile file
	QFile profileFile(m_currentProfilePath);
	if(!profileFile.open(QIODevice::ReadOnly)) {
		qWarning(CAT_PROFILEGENERATORWIDGET) << "Failed to read generated profile file";
		return false;
	}
	QByteArray profileData = profileFile.readAll();
	profileFile.close();

	// Read generated stream file
	QFile streamFile(m_currentStreamPath);
	if(!streamFile.open(QIODevice::ReadOnly)) {
		qWarning(CAT_PROFILEGENERATORWIDGET) << "Failed to read generated stream file";
		return false;
	}
	QByteArray streamData = streamFile.readAll();
	streamFile.close();

	// Write to device
	bool success = writeDeviceProfile(profileData) && writeDeviceStream(streamData);

	// Cleanup temp files
	QFile::remove(m_currentProfilePath);
	QFile::remove(m_currentStreamPath);

	if(success) {
		qInfo(CAT_PROFILEGENERATORWIDGET) << "Successfully loaded profile and stream to device";
	}

	return success;
}

bool ProfileGeneratorWidget::saveProfileToFile(const QString &filename)
{
	if(!m_cliAvailable) {
		// Fallback: save the basic JSON representation
		QJsonObject profile = generateBasicProfile();
		QJsonDocument doc(profile);

		QFile file(filename);
		if(file.open(QIODevice::WriteOnly)) {
			file.write(doc.toJson());
			return true;
		}
		return false;
	}

	// Use CLI to generate profile file (following iio-oscilloscope pattern)
	QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
	QString configFile = tempDir + "/adrv9002_config.json";

	// Write config to temp file
	if(!writeConfigToFile(configFile)) {
		return false;
	}

	// Determine if saving profile or stream based on filename extension
	bool isProfile = filename.endsWith(".json", Qt::CaseInsensitive);

	// Run CLI command
	QString command;
	if(isProfile) {
		command = QString("%1 --config %2 --profile %3").arg(m_cliPath).arg(configFile).arg(filename);
	} else {
		command = QString("%1 --config %2 --stream %3").arg(m_cliPath).arg(configFile).arg(filename);
	}

	QProcess process;
	process.start(command);
	process.waitForFinished(30000); // 30 second timeout

	// Cleanup config file
	QFile::remove(configFile);

	if(process.exitCode() != 0) {
		qWarning(CAT_PROFILEGENERATORWIDGET) << "CLI save command failed:" << process.readAllStandardError();
		return false;
	}

	return true;
}

bool ProfileGeneratorWidget::writeDeviceProfile(const QByteArray &profileData)
{
	return writeDeviceAttribute("profile_config", profileData);
}

bool ProfileGeneratorWidget::writeDeviceStream(const QByteArray &streamData)
{
	return writeDeviceAttribute("stream_config", streamData);
}

QString ProfileGeneratorWidget::readDeviceAttribute(const QString &attributeName)
{
	if(!m_device) {
		return QString();
	}

	char buffer[1024];
	int ret = iio_device_attr_read(m_device, attributeName.toLocal8Bit().data(), buffer, sizeof(buffer));

	if(ret > 0) {
		return QString::fromLocal8Bit(buffer, ret).trimmed();
	} else {
		qDebug(CAT_PROFILEGENERATORWIDGET)
			<< "Failed to read device attribute:" << attributeName << "ret:" << ret;
		return QString();
	}
}

bool ProfileGeneratorWidget::writeDeviceAttribute(const QString &attributeName, const QByteArray &data)
{
	if(!m_device) {
		qWarning(CAT_PROFILEGENERATORWIDGET) << "No device available for writing attribute:" << attributeName;
		return false;
	}

	int ret =
		iio_device_attr_write_raw(m_device, attributeName.toLocal8Bit().data(), data.constData(), data.size());

	if(ret == data.size()) {
		qDebug(CAT_PROFILEGENERATORWIDGET)
			<< "Successfully wrote" << data.size() << "bytes to attribute:" << attributeName;
		return true;
	} else {
		qWarning(CAT_PROFILEGENERATORWIDGET) << "Failed to write attribute:" << attributeName
						     << "expected:" << data.size() << "written:" << ret;
		return false;
	}
}

void ProfileGeneratorWidget::updateUIFromConfig()
{
	// Update UI widgets from m_radioConfig
	m_ssiInterfaceCombo->setCurrentText(m_radioConfig.lvds ? "LVDS" : "CMOS");
	m_duplexModeCombo->setCurrentText(m_radioConfig.fdd ? "FDD" : "TDD");

	// Update channel widgets...
	// This can be implemented when needed for reading from device
}

bool ProfileGeneratorWidget::writeConfigToFile(const QString &filename)
{
	QJsonObject config = createFullConfigJson();
	QJsonDocument doc(config);

	QFile file(filename);
	if(!file.open(QIODevice::WriteOnly)) {
		qWarning(CAT_PROFILEGENERATORWIDGET) << "Failed to write config file:" << filename;
		return false;
	}

	file.write(doc.toJson());
	file.close();
	return true;
}

QJsonObject ProfileGeneratorWidget::createFullConfigJson()
{
	QJsonObject config;

	// Radio configuration (matching iio-oscilloscope format)
	QJsonObject radioCfg;
	radioCfg["ssi_lanes"] = m_radioConfig.ssi_lanes;
	radioCfg["ddr"] = 1; // Always enabled
	radioCfg["short_strobe"] = true;
	radioCfg["lvds"] = m_radioConfig.lvds;
	radioCfg["adc_rate_mode"] = 3; // High performance
	radioCfg["fdd"] = m_radioConfig.fdd;

	// RX configuration array
	QJsonArray rxConfigArray;
	for(int i = 0; i < 2; i++) {
		QJsonObject rxCh;
		rxCh["enabled"] = m_radioConfig.rx_config[i].enabled;
		rxCh["adc_high_performance_mode"] = true;
		rxCh["frequency_offset_correction_enable"] = m_radioConfig.rx_config[i].freqOffsetCorrection;
		rxCh["analog_filter_power_mode"] = 2; // High power
		rxCh["analog_filter_biquad"] = false;
		rxCh["analog_filter_bandwidth_hz"] = 0;
		rxCh["channel_bandwidth_hz"] = m_radioConfig.rx_config[i].bandwidth.toInt();
		rxCh["sample_rate_hz"] = m_radioConfig.rx_config[i].sampleRate.toInt();
		rxCh["nco_enable"] = false;
		rxCh["nco_frequency_hz"] = 0;
		rxCh["rf_port"] =
			m_radioConfig.rx_config[i].rfInput == "Rx1A" || m_radioConfig.rx_config[i].rfInput == "Rx2A"
			? 0
			: 1;
		rxConfigArray.append(rxCh);
	}
	radioCfg["rx_config"] = rxConfigArray;

	// TX configuration array
	QJsonArray txConfigArray;
	for(int i = 0; i < 2; i++) {
		QJsonObject txCh;
		txCh["enabled"] = m_radioConfig.tx_config[i].enabled;
		txCh["sample_rate_hz"] = m_radioConfig.tx_config[i].sampleRate.toInt();
		txCh["frequency_offset_correction_enable"] = m_radioConfig.tx_config[i].freqOffsetCorrection;
		txCh["analog_filter_power_mode"] = 2; // High power
		txCh["channel_bandwidth_hz"] = m_radioConfig.tx_config[i].bandwidth.toInt();
		txCh["orx_enabled"] = m_radioConfig.orx_enabled[i];
		txCh["elb_type"] = 2; // Default value
		txConfigArray.append(txCh);
	}
	radioCfg["tx_config"] = txConfigArray;

	config["radio_cfg"] = radioCfg;

	// Clock configuration (matching iio-oscilloscope format exactly)
	QJsonObject clkCfg;
	clkCfg["device_clock_frequency_khz"] = 38400;
	clkCfg["device_clock_output_enable"] = 0; // Default disabled
	clkCfg["device_clock_output_divider"] = 1;
	clkCfg["clock_pll_high_performance_enable"] = true;
	clkCfg["clock_pll_power_mode"] = 2; // High power
	clkCfg["processor_clock_divider"] = 1;

	config["clk_cfg"] = clkCfg;

	return config;
}

// Signal Dependencies Implementation

void ProfileGeneratorWidget::onTxEnableChanged()
{
	// 1. Update TX channel controls (enable/disable fields)
	updateChannelControlsVisibility();

	// 2. Update ORX controls (both visibility and enable state)
	updateOrxControlsState();

	// 3. Update profile data
	updateProfileData();
}

void ProfileGeneratorWidget::onTddModeChanged()
{
	// TDD/FDD mode change affects ORX availability
	updateOrxControlsState();
	updateProfileData();
}

void ProfileGeneratorWidget::onInterfaceChanged()
{
	// Interface changed - update profile data
	updateProfileData();
}

void ProfileGeneratorWidget::onChannelEnableChanged()
{
	// Handle both RX and TX channel enable changes
	updateChannelControlsVisibility();
	updateOrxControlsState();  // TX changes affect ORX
	updateProfileData();
}

void ProfileGeneratorWidget::onSampleRateChanged(ChannelType channel)
{
	if (m_updatingFromPreset) return;

	// Get the channel widgets
	ChannelWidgets *widgets = nullptr;
	switch(channel) {
	case CHANNEL_RX1: widgets = &m_rx1Widgets; break;
	case CHANNEL_RX2: widgets = &m_rx2Widgets; break;
	case CHANNEL_TX1: widgets = &m_tx1Widgets; break;
	case CHANNEL_TX2: widgets = &m_tx2Widgets; break;
	}

	if (!widgets || !widgets->sampleRateCombo || !widgets->bandwidthCombo) return;

	// Update bandwidth based on sample rate (iio-oscilloscope logic)
	QString sampleRate = widgets->sampleRateCombo->currentText();
	QString correspondingBandwidth = FrequencyTable::getBandwidthForSampleRate(sampleRate);

	widgets->bandwidthCombo->blockSignals(true);
	widgets->bandwidthCombo->setCurrentText(correspondingBandwidth);
	widgets->bandwidthCombo->blockSignals(false);

	qDebug(CAT_PROFILEGENERATORWIDGET) << "Sample rate changed for channel" << channel
	                                   << "to" << sampleRate << "-> bandwidth" << correspondingBandwidth;
}

// LTE Defaults Implementation

void ProfileGeneratorWidget::applyLTEDefaults()
{
	qInfo(CAT_PROFILEGENERATORWIDGET) << "Applying LTE defaults from lte_defaults() function";

	m_updatingFromPreset = true;

	// Radio config - exact values from lte_defaults()
	m_ssiInterfaceCombo->setCurrentText(LTEDefaults::SSI_INTERFACE); // "LVDS"
	m_duplexModeCombo->setCurrentText(LTEDefaults::DUPLEX_MODE); // "TDD"

	// Apply to all channels - exact values from lte_defaults()
	QString sampleRateStr = QString::number(LTEDefaults::SAMPLE_RATE_HZ); // "61440000"
	QString bandwidthStr = QString::number(LTEDefaults::BANDWIDTH_HZ); // "38000000"

	// RX1 and RX2
	m_rx1Widgets.enabledCb->setChecked(LTEDefaults::ENABLED);
	m_rx1Widgets.freqOffsetCb->setChecked(LTEDefaults::FREQ_OFFSET_CORRECTION);
	m_rx1Widgets.sampleRateCombo->setCurrentText(sampleRateStr);
	m_rx1Widgets.bandwidthCombo->setCurrentText(bandwidthStr);
	if (m_rx1Widgets.rfInputCombo) {
		m_rx1Widgets.rfInputCombo->setCurrentIndex(LTEDefaults::RF_PORT); // 0 = Rx1A
	}

	m_rx2Widgets.enabledCb->setChecked(LTEDefaults::ENABLED);
	m_rx2Widgets.freqOffsetCb->setChecked(LTEDefaults::FREQ_OFFSET_CORRECTION);
	m_rx2Widgets.sampleRateCombo->setCurrentText(sampleRateStr);
	m_rx2Widgets.bandwidthCombo->setCurrentText(bandwidthStr);
	if (m_rx2Widgets.rfInputCombo) {
		m_rx2Widgets.rfInputCombo->setCurrentIndex(LTEDefaults::RF_PORT); // 0 = Rx2A
	}

	// TX1 and TX2
	m_tx1Widgets.enabledCb->setChecked(LTEDefaults::ENABLED);
	m_tx1Widgets.freqOffsetCb->setChecked(LTEDefaults::FREQ_OFFSET_CORRECTION);
	m_tx1Widgets.sampleRateCombo->setCurrentText(sampleRateStr);
	m_tx1Widgets.bandwidthCombo->setCurrentText(bandwidthStr);

	m_tx2Widgets.enabledCb->setChecked(LTEDefaults::ENABLED);
	m_tx2Widgets.freqOffsetCb->setChecked(LTEDefaults::FREQ_OFFSET_CORRECTION);
	m_tx2Widgets.sampleRateCombo->setCurrentText(sampleRateStr);
	m_tx2Widgets.bandwidthCombo->setCurrentText(bandwidthStr);

	// ORX disabled by default in LTE preset
	m_orx1EnabledCb->setChecked(false);
	m_orx2EnabledCb->setChecked(false);

	m_lastAppliedPreset = "LTE";
	m_updatingFromPreset = false;

	// Update dependent UI elements
	updateOrxVisibility();
	updateChannelControlsVisibility();

	StatusBarManager::pushMessage("Applied LTE defaults", 3000);
}

// Signal Dependencies Helper Methods

void ProfileGeneratorWidget::updateOrxVisibility()
{
	bool tx1Enabled = getTxChannelEnabled(0);
	bool tx2Enabled = getTxChannelEnabled(1);
	bool tddMode = getTddModeEnabled();

	// ORX only visible when TX enabled + TDD mode (matching iio-oscilloscope)
	bool orxShouldBeVisible = (tx1Enabled || tx2Enabled) && tddMode;

	// Find ORX widgets and update visibility
	if (m_orx1EnabledCb) {
		m_orx1EnabledCb->parentWidget()->setVisible(orxShouldBeVisible);
	}
	if (m_orx2EnabledCb) {
		m_orx2EnabledCb->parentWidget()->setVisible(orxShouldBeVisible);
	}

	qDebug(CAT_PROFILEGENERATORWIDGET) << "ORX visibility:" << orxShouldBeVisible
	                                   << "(TX1:" << tx1Enabled << "TX2:" << tx2Enabled << "TDD:" << tddMode << ")";
}

void ProfileGeneratorWidget::updateOrxControlsState()
{
	bool tx1Enabled = getChannelEnabled(CHANNEL_TX1);
	bool tx2Enabled = getChannelEnabled(CHANNEL_TX2);
	bool tddMode = getTddModeEnabled();

	// ORX1 logic: TX1 enabled AND TDD mode
	bool orx1CanBeEnabled = tx1Enabled && tddMode;
	if (!orx1CanBeEnabled && m_orx1EnabledCb && m_orx1EnabledCb->isChecked()) {
		m_orx1EnabledCb->setChecked(false);  // Force disable
	}
	if (m_orx1EnabledCb) {
		m_orx1EnabledCb->setEnabled(orx1CanBeEnabled);
	}

	// ORX2 logic: TX2 enabled AND TDD mode
	bool orx2CanBeEnabled = tx2Enabled && tddMode;
	if (!orx2CanBeEnabled && m_orx2EnabledCb && m_orx2EnabledCb->isChecked()) {
		m_orx2EnabledCb->setChecked(false);  // Force disable
	}
	if (m_orx2EnabledCb) {
		m_orx2EnabledCb->setEnabled(orx2CanBeEnabled);
	}

	qDebug(CAT_PROFILEGENERATORWIDGET) << "ORX controls state - ORX1 can be enabled:" << orx1CanBeEnabled
	                                   << "ORX2 can be enabled:" << orx2CanBeEnabled;
}


void ProfileGeneratorWidget::updateChannelControlsVisibility()
{
	// Update all channel controls based on their enable state
	setChannelControlsEnabled(CHANNEL_RX1, getChannelEnabled(CHANNEL_RX1));
	setChannelControlsEnabled(CHANNEL_RX2, getChannelEnabled(CHANNEL_RX2));
	setChannelControlsEnabled(CHANNEL_TX1, getChannelEnabled(CHANNEL_TX1));
	setChannelControlsEnabled(CHANNEL_TX2, getChannelEnabled(CHANNEL_TX2));

	qDebug(CAT_PROFILEGENERATORWIDGET) << "Updated channel controls visibility";
}

bool ProfileGeneratorWidget::getTxChannelEnabled(int channel)
{
	if (channel == 0) {
		return m_tx1Widgets.enabledCb ? m_tx1Widgets.enabledCb->isChecked() : false;
	} else if (channel == 1) {
		return m_tx2Widgets.enabledCb ? m_tx2Widgets.enabledCb->isChecked() : false;
	}
	return false;
}

bool ProfileGeneratorWidget::getTddModeEnabled()
{
	return m_duplexModeCombo ? m_duplexModeCombo->currentText() == "TDD" : false;
}

bool ProfileGeneratorWidget::validateConfiguration()
{
	// Basic validation - can be extended
	return true;
}

// Channel Control Helper Methods

ProfileGeneratorWidget::ChannelWidgets* ProfileGeneratorWidget::getChannelWidgets(ChannelType channel)
{
	switch(channel) {
	case CHANNEL_RX1: return &m_rx1Widgets;
	case CHANNEL_RX2: return &m_rx2Widgets;
	case CHANNEL_TX1: return &m_tx1Widgets;
	case CHANNEL_TX2: return &m_tx2Widgets;
	}
	return nullptr;
}

bool ProfileGeneratorWidget::getChannelEnabled(ChannelType channel)
{
	ChannelWidgets* widgets = getChannelWidgets(channel);
	return widgets && widgets->enabledCb ? widgets->enabledCb->isChecked() : false;
}

void ProfileGeneratorWidget::setChannelControlsEnabled(ChannelType channel, bool enabled)
{
	ChannelWidgets* widgets = getChannelWidgets(channel);
	if (!widgets) return;

	// Enable/disable all channel controls except the enable checkbox itself
	if (widgets->freqOffsetCb) {
		widgets->freqOffsetCb->setEnabled(enabled);
	}
	if (widgets->bandwidthCombo) {
		widgets->bandwidthCombo->setEnabled(enabled);
	}
	if (widgets->sampleRateCombo) {
		widgets->sampleRateCombo->setEnabled(enabled);
	}
	if (widgets->rfInputCombo) {  // RX only
		widgets->rfInputCombo->setEnabled(enabled);
	}

	qDebug(CAT_PROFILEGENERATORWIDGET) << "Channel" << channel << "controls enabled:" << enabled;
}

#include "moc_profilegeneratorwidget.cpp"
