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
#include <channelconfigwidget.h>
#include <profilegeneratorconstants.h>

#include <QLoggingCategory>
#include <QFileDialog>
#include <QStandardPaths>
#include <QApplication>
#include <QtConcurrent>
#include <stylehelper.h>

Q_LOGGING_CATEGORY(CAT_PROFILEGENERATORWIDGET, "ProfileGeneratorWidget")

using namespace scopy::adrv9002;
using namespace scopy;

// DeviceConfigurationParser implementation (Phase 1)
QString DeviceConfigurationParser::extractValueBetween(const QString &text, const QString &begin, const QString &end)
{
	// Port from iio-oscilloscope extract_value_between() function (line 1365-1383)
	int beginPos = text.indexOf(begin);
	if(beginPos == -1) {
		return QString();
	}

	beginPos += begin.length();
	int endPos = end.isEmpty() ? text.length() : text.indexOf(end, beginPos);
	if(endPos == -1) {
		endPos = text.length();
	}

	return text.mid(beginPos, endPos - beginPos).trimmed();
}

QString DeviceConfigurationParser::mapRfPortFromDevice(const QString &devicePort, int channel)
{
	// Map device RF port numbers to UI strings
	// Based on iio-oscilloscope rf_port mapping: 0=RxA, 1=RxB
	bool ok;
	int portNum = devicePort.toInt(&ok);
	if(!ok) {
		return (channel == 0) ? "Rx1A" : "Rx2A"; // Default fallback
	}

	if(channel == 0) {
		return (portNum == 0) ? "Rx1A" : "Rx1B";
	} else {
		return (portNum == 0) ? "Rx2A" : "Rx2B";
	}
}

DeviceConfigurationParser::ParsedDeviceConfig
DeviceConfigurationParser::parseProfileConfig(iio_device *dev, const QString &profileConfigText)
{
	ParsedDeviceConfig config;

	// Parse duplex mode - look for "Duplex Mode:" line
	QString duplexMode = extractValueBetween(profileConfigText, "Duplex Mode:", "\n");
	config.duplexMode = duplexMode.contains("TDD", Qt::CaseInsensitive) ? "TDD" : "FDD";

	// Parse SSI interface - look for "SSI interface:" line
	QString ssiInterface = extractValueBetween(profileConfigText, "SSI interface:", "\n");
	config.ssiInterface = ssiInterface.contains("LVDS", Qt::CaseInsensitive) ? "LVDS" : "CMOS";
	config.ssiLanes = ssiInterface.contains("LVDS", Qt::CaseInsensitive) ? 2 : 4;

	// Parse device clock info
	config.deviceClock = extractValueBetween(profileConfigText, "Device clk(Hz):", "\n").toUInt();
	config.clockDivider = extractValueBetween(profileConfigText, "ARM Power Saving Clk Divider:", "\n").toUInt();

	// Parse channel masks (like iio-oscilloscope)
	QString rxChannelMask = extractValueBetween(profileConfigText, "RX Channel Mask:", "\n");
	QString txChannelMask = extractValueBetween(profileConfigText, "TX Channel Mask:", "\n");

	// Convert hex masks to channel enable states
	bool rxMaskOk, txMaskOk;
	uint32_t rxMask = rxChannelMask.trimmed().toUInt(&rxMaskOk, 16);
	uint32_t txMask = txChannelMask.trimmed().toUInt(&txMaskOk, 16);

	// Extract channel enable states from masks (iio-oscilloscope logic)
	for(int ch = 0; ch < 2; ch++) {
		// RX channel enable from mask bits
		config.rxChannels[ch].enabled = rxMaskOk && (rxMask & (1 << ch));

		// TX channel enable from mask bits
		config.txChannels[ch].enabled = txMaskOk && (txMask & (1 << ch));

		QString chnannelName = QString("voltage%1").arg(ch);

		iio_channel *rxChannel = iio_device_find_channel(dev, chnannelName.toUtf8(), false);
		iio_channel *txChannel = iio_device_find_channel(dev, chnannelName.toUtf8(), true);

		int ret = 0;
		// Channel data will be populated from individual IIO attributes
		if(rxChannel != nullptr) {
			long long sampling_freq;
			ret = iio_channel_attr_read_longlong(rxChannel, "sampling_frequency", &sampling_freq);
			if(ret == 0)
				config.rxChannels[ch].sampleRateHz = static_cast<uint32_t>(sampling_freq);

			long long rf_bandwidth;
			ret = iio_channel_attr_read_longlong(rxChannel, "rf_bandwidth", &rf_bandwidth);
			if(ret == 0)
				config.rxChannels[ch].channelBandwidthHz = static_cast<uint32_t>(rf_bandwidth);

			config.rxChannels[ch].freqOffsetCorrectionEnable = false;
			config.rxChannels[ch].rfPort = 0; // Default
			config.rxChannels[ch].hasChannelData = false;
			config.rxChannels[ch].dataSource = "unavailable";
		}

		if(txChannel != nullptr) {

			long long sampling_freq;
			ret = iio_channel_attr_read_longlong(txChannel, "sampling_frequency", &sampling_freq);
			if(ret == 0)
				config.txChannels[ch].sampleRateHz = sampling_freq;

			long long rf_bandwidth;
			ret = iio_channel_attr_read_longlong(txChannel, "rf_bandwidth", &rf_bandwidth);
			if(ret == 0)
				config.txChannels[ch].channelBandwidthHz = rf_bandwidth;

			config.txChannels[ch].freqOffsetCorrectionEnable = false;
			config.txChannels[ch].orxEnabled = false;
			config.txChannels[ch].hasChannelData = false;
			config.txChannels[ch].dataSource = "unavailable";
		}
	}

	return config;
}

ProfileGeneratorWidget::ProfileGeneratorWidget(iio_device *device, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
	, m_cliManager(nullptr)
	, m_cliAvailable(false)
	, m_contentWidget(nullptr)
	, m_lastAppliedPreset("")
	, m_updatingFromPreset(false)
{
	// Initialize m_radioConfig with LTE template FIRST (prevents garbage values)
	m_radioConfig = ProfileGeneratorConstants::lteDefaults();

	// Initialize CLI manager
	m_cliManager = new ProfileCliManager(m_device, this);
	m_cliAvailable = m_cliManager->isCliAvailable();

	// Connect CLI manager signals for user feedback
	connect(
		m_cliManager, &ProfileCliManager::operationProgress, this,
		[](const QString &msg) { StatusBarManager::pushMessage(msg, 3000); }, Qt::QueuedConnection);
	connect(
		m_cliManager, &ProfileCliManager::operationError, this,
		[](const QString &error) { StatusBarManager::pushMessage(error, 5000); }, Qt::QueuedConnection);

	setupUi();
}

ProfileGeneratorWidget::~ProfileGeneratorWidget() {}

void ProfileGeneratorWidget::setupUi()
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setSpacing(15);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	// Create content widget (all controls go here)
	m_contentWidget = new QWidget(this);
	QVBoxLayout *contentLayout = new QVBoxLayout(m_contentWidget);
	contentLayout->setSpacing(15);
	contentLayout->setContentsMargins(0, 0, 0, 0);

	mainLayout->addWidget(generateDeviceDriverAPIWidget(this));

	// Add all the profile generator collapsible sections to content widget
	contentLayout->addWidget(createProfileActionSection());
	contentLayout->addWidget(createRadioConfigSection());
	contentLayout->addWidget(createChannelConfigSection());
	contentLayout->addWidget(createOrxConfigSection());
	contentLayout->addWidget(createDebugInfoSection());

	// Add stretch to push content to top (from prompt pattern)
	contentLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	mainLayout->addWidget(m_contentWidget);

	if(m_cliAvailable) {
		setupEnhancedConnections();

		m_ssiInterfaceLabel->setText(LTEDefaults::SSI_INTERFACE);
		m_radioConfig.lvds = true;
		m_radioConfig.ssi_lanes = LTEDefaults::SSI_LANES;
		m_presetCombo->setCurrentText("LTE");

		// Perform complete UI state refresh after preset initialization
		refreshAllUIStates();
	}
}

MenuSectionCollapseWidget *ProfileGeneratorWidget::createProfileActionSection()
{
	auto section = new MenuSectionCollapseWidget("Profile Actions", MenuCollapseSection::MHCW_ARROW,
						     MenuCollapseSection::MHW_BASEWIDGET, this);

	QWidget *actionBar = new QWidget();
	QHBoxLayout *layout = new QHBoxLayout(actionBar);
	layout->setContentsMargins(10, 10, 10, 10);
	layout->setSpacing(15);

	// Preset dropdown with styling - matching iio-oscilloscope exactly
	QLabel *presetLabel = new QLabel("Preset:");
	Style::setStyle(presetLabel, style::properties::label::menuSmall);

	m_presetCombo = new QComboBox();
	m_presetCombo->addItems({"Live Device", "LTE"});
	m_presetCombo->setMinimumWidth(120);

	// Action buttons with proper Scopy styling
	m_refreshProfileBtn = new QPushButton("Refresh");

	m_saveStreamToFileBtn = new AnimatedLoadingButton("Save Stream to file", this);
	m_saveProfileToFileBtn = new AnimatedLoadingButton("Save Profile to file", this);

	m_loadToDeviceBtn = new AnimatedLoadingButton("Load to device", this);

	// Apply Scopy button styling from prompt patterns
	Style::setStyle(m_refreshProfileBtn, style::properties::button::basicButton);
	Style::setStyle(m_saveStreamToFileBtn, style::properties::button::basicButton);
	Style::setStyle(m_saveProfileToFileBtn, style::properties::button::basicButton);
	Style::setStyle(m_loadToDeviceBtn, style::properties::button::basicButton);

	// Set minimum button widths for consistency
	m_refreshProfileBtn->setMinimumWidth(100);
	m_saveStreamToFileBtn->setMinimumWidth(120);
	m_saveProfileToFileBtn->setMinimumWidth(120);
	m_loadToDeviceBtn->setMinimumWidth(130);

	// Layout with proper spacing - matching iio-oscilloscope exactly
	layout->addWidget(presetLabel);
	layout->addWidget(m_presetCombo);
	layout->addWidget(m_refreshProfileBtn);
	layout->addStretch();
	layout->addWidget(m_saveStreamToFileBtn);
	layout->addWidget(m_saveProfileToFileBtn);
	layout->addWidget(m_loadToDeviceBtn);

	section->add(actionBar);
	return section;
}

MenuSectionCollapseWidget *ProfileGeneratorWidget::createRadioConfigSection()
{
	auto section = new MenuSectionCollapseWidget("Radio Configuration", MenuCollapseSection::MHCW_ARROW,
						     MenuCollapseSection::MHW_BASEWIDGET, this);

	QWidget *radioConfigWidget = new QWidget();
	QGridLayout *layout = new QGridLayout(radioConfigWidget);
	layout->setSpacing(10);
	layout->setContentsMargins(10, 10, 10, 10);

	// SSI Interface - READ-ONLY LABEL like iio-oscilloscope (device-determined)
	layout->addWidget(new QLabel("SSI Interface:"), 0, 0);
	m_ssiInterfaceLabel = new QLabel("Reading...");
	Style::setStyle(m_ssiInterfaceLabel, style::properties::label::menuSmall);
	layout->addWidget(m_ssiInterfaceLabel, 0, 1);

	// Duplex mode - using correct IOO options
	layout->addWidget(new QLabel("Duplex mode:"), 0, 2);
	m_duplexModeCombo = new QComboBox();
	m_duplexModeCombo->addItems(IOOOptions::DUPLEX_MODE_OPTIONS);
	layout->addWidget(m_duplexModeCombo, 0, 3);

	section->add(radioConfigWidget);
	return section;
}

MenuSectionCollapseWidget *ProfileGeneratorWidget::createChannelConfigSection()
{
	auto section = new MenuSectionCollapseWidget("Channel Configuration", MenuCollapseSection::MHCW_ARROW,
						     MenuCollapseSection::MHW_BASEWIDGET, this);

	QWidget *channelConfigWidget = new QWidget();
	QGridLayout *layout = new QGridLayout(channelConfigWidget);
	layout->setSpacing(15);
	layout->setContentsMargins(10, 10, 10, 10);

	// Create 2x2 grid: RX1, RX2, TX1, TX2
	QWidget *rx1Frame = createChannelConfigWidget("RX 1", CHANNEL_RX1);
	QWidget *rx2Frame = createChannelConfigWidget("RX 2", CHANNEL_RX2);
	QWidget *tx1Frame = createChannelConfigWidget("TX 1", CHANNEL_TX1);
	QWidget *tx2Frame = createChannelConfigWidget("TX 2", CHANNEL_TX2);

	layout->addWidget(rx1Frame, 0, 0);
	layout->addWidget(rx2Frame, 0, 1);
	layout->addWidget(tx1Frame, 1, 0);
	layout->addWidget(tx2Frame, 1, 1);

	section->add(channelConfigWidget);
	return section;
}

QWidget *ProfileGeneratorWidget::createChannelConfigWidget(const QString &title, ChannelType type)
{
	// Determine channel mode and RF input options
	ChannelConfigWidget::ChannelMode mode = (type == CHANNEL_RX1 || type == CHANNEL_RX2)
		? ChannelConfigWidget::RX_MODE
		: ChannelConfigWidget::TX_MODE;

	QStringList rfOptions;
	if(type == CHANNEL_RX1) {
		rfOptions = RFInputOptions::RX1_OPTIONS;
	} else if(type == CHANNEL_RX2) {
		rfOptions = RFInputOptions::RX2_OPTIONS;
	}

	// Create unified channel widget
	ChannelConfigWidget *channelWidget = new ChannelConfigWidget(title, mode, rfOptions, this);
	m_channelWidgets[type] = channelWidget;

	// Connect unified signals for immediate updates
	connect(channelWidget, &ChannelConfigWidget::sampleRateChanged, this,
		[this](const QString &rate) { onSampleRateChangedSynchronized(rate); });
	connect(channelWidget, &ChannelConfigWidget::enabledChanged, this,
		&ProfileGeneratorWidget::onChannelEnableChanged);
	connect(channelWidget, &ChannelConfigWidget::channelDataChanged, this,
		&ProfileGeneratorWidget::updateProfileData);

	return channelWidget;
}

MenuSectionCollapseWidget *ProfileGeneratorWidget::createOrxConfigSection()
{
	auto section = new MenuSectionCollapseWidget("ORX Configuration", MenuCollapseSection::MHCW_ARROW,
						     MenuCollapseSection::MHW_BASEWIDGET, this);

	QWidget *orxConfigWidget = new QWidget();
	QHBoxLayout *layout = new QHBoxLayout(orxConfigWidget);
	layout->setSpacing(15);
	layout->setContentsMargins(10, 10, 10, 10);

	// ORX 1
	layout->addWidget(createOrxWidget("ORX 1"));
	// ORX 2
	layout->addWidget(createOrxWidget("ORX 2"));

	section->add(orxConfigWidget);
	return section;
}

QWidget *ProfileGeneratorWidget::createOrxWidget(const QString &title)
{
	QWidget *widget = new QWidget();
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setContentsMargins(10, 10, 10, 10);
	layout->setSpacing(10);

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

MenuSectionCollapseWidget *ProfileGeneratorWidget::createDebugInfoSection()
{
	auto section = new MenuSectionCollapseWidget("Debug Information", MenuCollapseSection::MHCW_ARROW,
						     MenuCollapseSection::MHW_BASEWIDGET, this);

	section->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	QWidget *debugInfoWidget = new QWidget();
	debugInfoWidget->setMinimumHeight(500);
	QVBoxLayout *layout = new QVBoxLayout(debugInfoWidget);
	layout->setContentsMargins(10, 10, 10, 10);
	layout->setSpacing(10);

	m_debugInfoText = new QTextEdit();
	m_debugInfoText->setReadOnly(true);
	m_debugInfoText->setFont(QFont("monospace"));
	layout->addWidget(m_debugInfoText);

	section->add(debugInfoWidget);
	return section;
}

void ProfileGeneratorWidget::loadPresetData(const QString &presetName)
{
	if(presetName == "LTE") {
		// Apply LTE defaults
		applyLTEDefaults();
	} else if(presetName == "Live Device") {
		// Read current device configuration and populate UI
		if(readDeviceConfiguration()) {
			populateUIFromDeviceConfig(m_deviceConfig);
		} else {
			qWarning(CAT_PROFILEGENERATORWIDGET) << "Failed to read device configuration";
			StatusBarManager::pushMessage("Failed to read Live Device configuration", 5000);
		}
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

	// Radio config - read from device-determined SSI interface label (preserve template values except
	// user-configurable ones)
	QString ssiInterface = m_ssiInterfaceLabel->text();
	m_radioConfig.lvds = (ssiInterface == "LVDS");
	m_radioConfig.fdd = (m_duplexModeCombo->currentText() == "FDD");
	m_radioConfig.ssi_lanes = m_radioConfig.lvds ? 2 : 4;

	// Channel configs - only update user-configurable fields, preserve template values
	for(int i = 0; i < 4; ++i) {
		auto data = m_channelWidgets[i]->getChannelData();

		if(i < 2) { // RX channels (RX1, RX2)
			m_radioConfig.rx_config[i].enabled = data.enabled;
			m_radioConfig.rx_config[i].freqOffsetCorrectionEnable = data.freqOffsetCorrectionEnable;
			m_radioConfig.rx_config[i].channelBandwidthHz = data.channelBandwidthHz;
			m_radioConfig.rx_config[i].sampleRateHz = data.sampleRateHz;
			m_radioConfig.rx_config[i].rfPort = data.rfPort;
		} else { // TX channels (TX1, TX2)
			int txIndex = i - 2;
			m_radioConfig.tx_config[txIndex].enabled = data.enabled;
			m_radioConfig.tx_config[txIndex].freqOffsetCorrectionEnable = data.freqOffsetCorrectionEnable;
			m_radioConfig.tx_config[txIndex].channelBandwidthHz = data.channelBandwidthHz;
			m_radioConfig.tx_config[txIndex].sampleRateHz = data.sampleRateHz;
		}
	}

	// ORX - update from UI
	m_radioConfig.tx_config[0].orxEnabled = m_orx1EnabledCb->isChecked();
	m_radioConfig.tx_config[1].orxEnabled = m_orx2EnabledCb->isChecked();
}

int ProfileGeneratorWidget::getConfigFromCurrent(RadioConfig &config)
{
	int ret = getConfigFromDefault(config);
	if(ret < 0)
		return ret;

	populateConfigFromUI(config);

	return 0;
}

int ProfileGeneratorWidget::getConfigFromDefault(RadioConfig &config)
{
	QString preset = m_presetCombo->currentText();

	if(preset == "Live Device") {
		return getConfigFromDevice(config); // Read device state
	} else if(preset == "LTE") {
		config = ProfileGeneratorConstants::lteDefaults();
		return 0;
	}
	return -1;
}

int ProfileGeneratorWidget::getConfigFromDevice(RadioConfig &config)
{
	// Start with template as base
	config = ProfileGeneratorConstants::lte_lvs3072MHz10();

	// Read device configuration and override template
	if(!readDeviceConfiguration()) {
		return -1;
	}

	// Apply device values to config struct
	populateConfigFromDeviceData(config, m_deviceConfig);
	return 0;
}

void ProfileGeneratorWidget::populateConfigFromUI(RadioConfig &config)
{
	QString ssiInterface = m_ssiInterfaceLabel->text();
	config.lvds = (ssiInterface == "LVDS");
	config.fdd = (m_duplexModeCombo->currentText() == "FDD");
	config.ssi_lanes = m_radioConfig.lvds ? 2 : 4;

	// Channel configs - only update user-configurable fields, preserve template values
	for(int i = 0; i < 4; ++i) {
		auto data = m_channelWidgets[i]->getChannelData();

		if(i < 2) { // RX channels (RX1, RX2)
			config.rx_config[i].enabled = data.enabled;
			config.rx_config[i].freqOffsetCorrectionEnable = data.freqOffsetCorrectionEnable;
			config.rx_config[i].channelBandwidthHz = data.channelBandwidthHz;
			config.rx_config[i].sampleRateHz = data.sampleRateHz;
			config.rx_config[i].rfPort = data.rfPort;
		} else { // TX channels (TX1, TX2)
			int txIndex = i - 2;
			config.tx_config[txIndex].enabled = data.enabled;
			config.tx_config[txIndex].freqOffsetCorrectionEnable = data.freqOffsetCorrectionEnable;
			config.tx_config[txIndex].channelBandwidthHz = data.channelBandwidthHz;
			config.tx_config[txIndex].sampleRateHz = data.sampleRateHz;
		}
	}

	// ORX - update from UI
	config.tx_config[0].orxEnabled = m_orx1EnabledCb->isChecked();
	config.tx_config[1].orxEnabled = m_orx2EnabledCb->isChecked();
}

void ProfileGeneratorWidget::populateConfigFromDeviceData(
	RadioConfig &config, const DeviceConfigurationParser::ParsedDeviceConfig &deviceConfig)
{
	// Apply device values to config structure
	config.lvds = (deviceConfig.ssiInterface == "LVDS");
	config.ssi_lanes = deviceConfig.ssiLanes;
	config.fdd = (deviceConfig.duplexMode == "FDD");

	// Device clock configuration
	if(deviceConfig.deviceClock > 0) {
		config.clk_config.deviceClockFrequencyKhz = deviceConfig.deviceClock / 1000; // Convert Hz to kHz
	}
	if(deviceConfig.clockDivider > 0) {
		config.clk_config.processorClockDivider = deviceConfig.clockDivider;
	}

	// Apply RX channel configuration
	for(int i = 0; i < 2; i++) {
		const auto &rxChannelData = deviceConfig.rxChannels[i];

		// Apply enabled state from device mask
		config.rx_config[i].enabled = rxChannelData.enabled;

		if(rxChannelData.hasChannelData) {
			// Apply device-read channel parameters
			config.rx_config[i].sampleRateHz = rxChannelData.sampleRateHz;
			config.rx_config[i].channelBandwidthHz = rxChannelData.channelBandwidthHz;
			config.rx_config[i].freqOffsetCorrectionEnable = rxChannelData.freqOffsetCorrectionEnable;
			config.rx_config[i].rfPort = rxChannelData.rfPort;
			// Keep template values for fields not read from device
		} else {
			// If channel data unavailable, disabled channels use zero values
			if(!rxChannelData.enabled) {
				config.rx_config[i].sampleRateHz = 0;
				config.rx_config[i].channelBandwidthHz = 0;
				config.rx_config[i].freqOffsetCorrectionEnable = false;
				config.rx_config[i].rfPort = 0;
			}
		}
	}

	// Apply TX channel configuration
	for(int i = 0; i < 2; i++) {
		const auto &txChannelData = deviceConfig.txChannels[i];

		// Apply enabled state from device mask
		config.tx_config[i].enabled = txChannelData.enabled;

		if(txChannelData.hasChannelData) {
			// Apply device-read channel parameters
			config.tx_config[i].sampleRateHz = txChannelData.sampleRateHz;
			config.tx_config[i].channelBandwidthHz = txChannelData.channelBandwidthHz;
			config.tx_config[i].freqOffsetCorrectionEnable = txChannelData.freqOffsetCorrectionEnable;
			config.tx_config[i].orxEnabled = txChannelData.orxEnabled;
			// Keep template values for fields not read from device
		} else {
			// If channel data unavailable, disabled channels use zero values
			if(!txChannelData.enabled) {
				config.tx_config[i].sampleRateHz = 0;
				config.tx_config[i].channelBandwidthHz = 0;
				config.tx_config[i].freqOffsetCorrectionEnable = false;
				config.tx_config[i].orxEnabled = false;
			}
		}
	}

	qDebug(CAT_PROFILEGENERATORWIDGET)
		<< "Applied device configuration:"
		<< "SSI:" << deviceConfig.ssiInterface << "Duplex:" << deviceConfig.duplexMode
		<< "RX1 enabled:" << config.rx_config[0].enabled << "RX1 rate:" << config.rx_config[0].sampleRateHz
		<< "TX1 enabled:" << config.tx_config[0].enabled << "TX1 rate:" << config.tx_config[0].sampleRateHz;
}

void ProfileGeneratorWidget::updateDebugInfo()
{
	if(m_cliAvailable && m_cliManager) {
		// Show ONLY the pure CLI configuration JSON like iio-oscilloscope profile_gen_config_to_str()
		// This matches the exact output format from iio-oscilloscope lines 2142-2214
		QString configJson = m_cliManager->generateConfigPreview(m_radioConfig);

		// Format with indentation like iio-oscilloscope cJSON_Print()
		QJsonDocument configDoc = QJsonDocument::fromJson(configJson.toUtf8());
		m_debugInfoText->setPlainText(configDoc.toJson(QJsonDocument::Indented));
	} else {
		// Match iio-oscilloscope behavior when CLI not available (line 3326)
		m_debugInfoText->setPlainText(
			"Profile generation is only available with profile generator CLI installed!");
	}
}

// Slot implementations
void ProfileGeneratorWidget::onPresetChanged()
{
	QString presetName = m_presetCombo->currentText();

	if(presetName == "LTE") {
		// Apply LTE defaults
		applyLTEDefaults();

	} else if(presetName == "Live Device") {
		readAndApplyDeviceConfiguration();
	}
}

void ProfileGeneratorWidget::onRefreshProfile()
{
	resetPresetTracking();

	if(isLTEModeActive()) { // reset to default lte mode
		applyLTEConstraintsToDeviceData();
	} else {
		readAndApplyDeviceConfiguration();
	}

	refreshAllUIStates();

	updateDebugInfo();
}

void ProfileGeneratorWidget::onSaveToFile(bool isStreamFile)
{
	if(!m_cliAvailable) {
		qDebug(CAT_PROFILEGENERATORWIDGET)
			<< "Profile Generator CLI not available - cannot generate profile files";
		return;
	}

	QString filter = "Stream files (*.stream)";
	QString defaultName = QDir::home().filePath("adrv9002_profile.stream");
	if(!isStreamFile) {
		filter = "Profile files (*.json)";
		defaultName = QDir::home().filePath("adrv9002_profile.json");
	}
	QString fileName = QFileDialog::getSaveFileName(this, "Save File", defaultName, filter);

	if(!fileName.isEmpty()) {
		// UPDATE CONFIG ON MAIN THREAD BEFORE THREADING!
		m_radioConfig = ProfileGeneratorConstants::lteDefaults();
		updateConfigFromUI();

		// Copy data for safe worker thread access
		RadioConfig configCopy = m_radioConfig;

		// Execute appropriate worker function - fire and forget
		if(isStreamFile) {
			// Start animation
			m_saveStreamToFileBtn->startAnimation();
			QtConcurrent::run(this, &ProfileGeneratorWidget::doSaveStreamWork, fileName, configCopy);
		} else {
			m_saveProfileToFileBtn->startAnimation();
			QtConcurrent::run(this, &ProfileGeneratorWidget::doSaveProfileWork, fileName, configCopy);
		}
	}
}

void ProfileGeneratorWidget::onLoadToDevice()
{
	if(!m_cliAvailable) {
		qDebug(CAT_PROFILEGENERATORWIDGET) << "Profile Generator CLI not available - cannot load to device";
		return;
	}

	RadioConfig config;
	int ret = getConfigFromCurrent(config);
	if(ret < 0) {
		Q_EMIT loadToDeviceFailed("Failed to build configuration from preset and UI");
		return;
	}

	// UPDATE CONFIG ON MAIN THREAD BEFORE THREADING!
	updateConfigFromUI();

	// Copy data for safe worker thread access
	RadioConfig configCopy = m_radioConfig;

	// Start animation
	m_loadToDeviceBtn->startAnimation();

	// Execute worker function - fire and forget
	QtConcurrent::run(this, &ProfileGeneratorWidget::doLoadToDeviceWork, configCopy);
}

void ProfileGeneratorWidget::onDownloadCLI()
{
	QDesktopServices::openUrl(QUrl("https://analogdevicesinc.github.io/libadrv9002-iio"));
}

void ProfileGeneratorWidget::refreshProfileData() { onRefreshProfile(); }

// Worker functions for threaded operations
void ProfileGeneratorWidget::doSaveProfileWork(const QString &fileName, const RadioConfig &config)
{
	if(!m_cliManager || !m_cliAvailable) {
		Q_EMIT saveProfileFailed("Profile Generator CLI not available");
		return;
	}

	// Use CLI manager to save profile to file
	auto result = m_cliManager->saveProfileToFile(fileName, config);
	if(result == ProfileCliManager::Success) {
		Q_EMIT saveProfileSuccess(fileName);
	} else {
		Q_EMIT saveProfileFailed("CLI operation failed");
	}
}

void ProfileGeneratorWidget::doSaveStreamWork(const QString &fileName, const RadioConfig &config)
{
	if(!m_cliManager || !m_cliAvailable) {
		Q_EMIT saveStreamFailed("Profile Generator CLI not available");
		return;
	}

	// Use CLI manager to save stream to file
	auto result = m_cliManager->saveStreamToFile(fileName, config);
	if(result == ProfileCliManager::Success) {
		Q_EMIT saveStreamSuccess(fileName);
	} else {
		Q_EMIT saveStreamFailed("CLI operation failed");
	}
}

void ProfileGeneratorWidget::doLoadToDeviceWork(const RadioConfig &config)
{
	if(!m_cliManager || !m_cliAvailable) {
		Q_EMIT loadToDeviceFailed("Profile Generator CLI not available");
		return;
	}

	// Use CLI manager to load profile to device
	auto result = m_cliManager->loadProfileToDevice(config);
	if(result == ProfileCliManager::Success) {
		Q_EMIT loadToDeviceSuccess();
	} else {
		Q_EMIT loadToDeviceFailed("CLI operation failed");
	}
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

QString ProfileGeneratorWidget::getDeviceDriverVersion()
{
	// Try to get version from context or device attributes
	if(m_device != nullptr) {
		char api_version[16];
		auto ret = iio_device_debug_attr_read(m_device, "api_version", api_version, sizeof(api_version));
		if(ret < 0) {
			return "";
		} else {
			return QString(api_version);
		}
	}

	return "";
}

void ProfileGeneratorWidget::onChannelEnableChanged()
{
	updateOrxControls();
	updateProfileData();
}

void ProfileGeneratorWidget::updateSampleRateOptionsForSSI()
{
	// Skip sample rate restrictions in Live Device mode - preserve device values
	if(isLiveDeviceModeActive()) {
		qDebug(CAT_PROFILEGENERATORWIDGET) << "Live Device mode - skipping SSI sample rate restrictions";
		return;
	}

	// Get available sample rates for current SSI lanes configuration
	QStringList availableRates = FrequencyTable::getSampleRatesForSSILanes(m_radioConfig.ssi_lanes);

	// Block signals to prevent cascade updates
	m_updatingFromPreset = true;

	// Update all channel sample rate combo boxes using unified system
	for(int i = 0; i < 4; i++) {
		if(!m_channelWidgets[i])
			continue;

		// Store current selection before updating options
		QString currentRate = m_channelWidgets[i]->getSampleRateCombo()->currentText();

		// Update sample rate options through the ChannelConfigWidget
		m_channelWidgets[i]->setSampleRateOptions(availableRates);

		// If current rate is no longer valid, update bandwidth accordingly
		if(!availableRates.contains(currentRate) && !availableRates.isEmpty()) {
			QString newBandwidth = FrequencyTable::getBandwidthForSampleRate(availableRates.first());
			m_channelWidgets[i]->setBandwidthOptions(QStringList() << newBandwidth);
		}
	}

	m_updatingFromPreset = false;
}

QString ProfileGeneratorWidget::calculateBandwidthForSampleRate(const QString &sampleRate) const
{
	return FrequencyTable::getBandwidthForSampleRate(sampleRate);
}

// LTE Defaults Implementation

void ProfileGeneratorWidget::applyLTEDefaults()
{
	m_updatingFromPreset = true;

	m_ssiInterfaceLabel->blockSignals(m_updatingFromPreset);
	m_duplexModeCombo->blockSignals(m_updatingFromPreset);

	// Apply complete LTE template to m_radioConfig (new template architecture)
	RadioConfig radioConfig = ProfileGeneratorConstants::lteDefaults();

	if(radioConfig.lvds)
		m_ssiInterfaceLabel->setText("LVDS");

	m_duplexModeCombo->setCurrentText(radioConfig.fdd ? "FDD" : "TDD");

	// Get current enabled state to preserve it
	// Set RF input for RX1 and RX2 in LTE mode RX1 = RX2

	ChannelConfigWidget::ChannelData rxLteData;
	rxLteData.enabled = radioConfig.rx_config[0].enabled;
	rxLteData.freqOffsetCorrectionEnable = radioConfig.rx_config[0].freqOffsetCorrectionEnable;
	rxLteData.sampleRateHz = radioConfig.rx_config[0].sampleRateHz;
	rxLteData.channelBandwidthHz = radioConfig.rx_config[0].channelBandwidthHz;
	rxLteData.rfPort = radioConfig.rx_config[0].rfPort;

	// apply settings to RX1
	m_channelWidgets[0]->updateControlsVisibility(true); // Bandwidth read-only, sample rate selectable
	m_channelWidgets[0]->setChannelData(rxLteData);
	// apply settings to RX2
	m_channelWidgets[1]->updateControlsVisibility(true); // Bandwidth read-only, sample rate selectable
	m_channelWidgets[1]->setChannelData(rxLteData);

	// TX channels (TX1, TX2) in LTE mode TX1 = TX2
	ChannelConfigWidget::ChannelData txLteData;
	txLteData.enabled = radioConfig.tx_config[0].enabled;
	txLteData.freqOffsetCorrectionEnable = radioConfig.tx_config[0].freqOffsetCorrectionEnable;
	txLteData.sampleRateHz = radioConfig.tx_config[0].sampleRateHz;
	txLteData.channelBandwidthHz = radioConfig.tx_config[0].channelBandwidthHz;

	// apply settings to TX1
	m_channelWidgets[2]->updateControlsVisibility(true); // Bandwidth read-only, sample rate selectable
	m_channelWidgets[2]->setChannelData(txLteData);
	// apply settings to TX2
	m_channelWidgets[3]->updateControlsVisibility(true); // Bandwidth read-only, sample rate selectable
	m_channelWidgets[3]->setChannelData(txLteData);

	m_lastAppliedPreset = "LTE";
	m_updatingFromPreset = false;

	m_ssiInterfaceLabel->blockSignals(m_updatingFromPreset);
	m_duplexModeCombo->blockSignals(m_updatingFromPreset);

	// Update dependent UI elements
	updateOrxControls();
}

bool ProfileGeneratorWidget::isLTEModeActive() const { return m_presetCombo && m_presetCombo->currentText() == "LTE"; }

bool ProfileGeneratorWidget::isLiveDeviceModeActive() const
{
	return m_presetCombo && m_presetCombo->currentText() == "Live Device";
}

// Signal Dependencies Helper Methods

bool ProfileGeneratorWidget::isOrxAvailable(int orxIndex)
{
	bool txEnabled = getChannelEnabled(orxIndex == 0 ? CHANNEL_TX1 : CHANNEL_TX2);
	bool tddMode = getTddModeEnabled();
	return txEnabled && tddMode;
}

void ProfileGeneratorWidget::updateOrxControls()
{
	QCheckBox *orxCheckboxes[2] = {m_orx1EnabledCb, m_orx2EnabledCb};

	for(int i = 0; i < 2; ++i) {
		if(!orxCheckboxes[i])
			continue;

		bool orxAvailable = isOrxAvailable(i);
		if(QWidget *orxFrame = orxCheckboxes[i]->parentWidget()) {
			orxFrame->setEnabled(orxAvailable);
		}
		orxCheckboxes[i]->setEnabled(orxAvailable);

		if(!orxAvailable) {
			orxCheckboxes[i]->setChecked(false);
		}
	}
}

void ProfileGeneratorWidget::refreshAllUIStates()
{
	// Match iio-oscilloscope's profile_gen_preset_update() logic

	// Use extracted coordination method
	updateOrxControls();
	updateProfileData();
}

bool ProfileGeneratorWidget::getTddModeEnabled()
{
	return m_duplexModeCombo ? m_duplexModeCombo->currentText() == "TDD" : false;
}

bool ProfileGeneratorWidget::getChannelEnabled(ChannelType channel)
{
	if(channel >= 0 && channel < 4 && m_channelWidgets[channel]) {
		return m_channelWidgets[channel]->getChannelData().enabled;
	}
	return false;
}

void ProfileGeneratorWidget::setupEnhancedConnections()
{
	// Direct signal connections matching iio-oscilloscope behavior exactly

	// Action buttons - immediate execution
	connect(m_presetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&ProfileGeneratorWidget::onPresetChanged);
	connect(m_refreshProfileBtn, &QPushButton::clicked, this, &ProfileGeneratorWidget::onRefreshProfile);
	connect(m_saveStreamToFileBtn, &QPushButton::clicked, this, [=, this]() { onSaveToFile(true); });
	connect(m_saveProfileToFileBtn, &QPushButton::clicked, this, [=, this]() { onSaveToFile(false); });
	connect(m_loadToDeviceBtn, &QPushButton::clicked, this, &ProfileGeneratorWidget::onLoadToDevice);

	// Channel connections - simplified through ChannelConfigWidget signals
	// (Already connected in createChannelConfigWidget)
	// Special TX → ORX connections for immediate updates
	connect(m_channelWidgets[TX1], &ChannelConfigWidget::enabledChanged, this, [this]() {
		updateOrxControls(); // TX changes affect ORX immediately
	});
	connect(m_channelWidgets[TX2], &ChannelConfigWidget::enabledChanged, this, [this]() {
		updateOrxControls(); // TX changes affect ORX immediately
	});

	// Radio configuration changes - immediate updates matching iio-oscilloscope
	// SSI Interface is read-only label (device-determined) - no user interaction
	connect(m_duplexModeCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged), this, [this]() {
		updateOrxControls(); // Duplex mode immediately affects ORX availability
		updateProfileData();
	});

	// ORX changes - simple immediate updates (no user preference tracking like iio-oscilloscope)
	connect(m_orx1EnabledCb, &QCheckBox::toggled, this, [this]() { updateProfileData(); });
	connect(m_orx2EnabledCb, &QCheckBox::toggled, this, [this]() { updateProfileData(); });

	// Worker thread signals - animation control and user feedback
	connect(this, &ProfileGeneratorWidget::saveProfileSuccess, this, [this](const QString &fileName) {
		m_saveProfileToFileBtn->stopAnimation();
		QFileInfo fileInfo(fileName);
		StatusBarManager::pushMessage("Profile saved: " + fileInfo.fileName(), 3000);
	});
	connect(this, &ProfileGeneratorWidget::saveProfileFailed, this, [this](const QString &error) {
		m_saveProfileToFileBtn->stopAnimation();
		StatusBarManager::pushMessage("Save failed: " + error, 5000);
	});

	connect(this, &ProfileGeneratorWidget::saveStreamSuccess, this, [this](const QString &fileName) {
		m_saveStreamToFileBtn->stopAnimation();
		QFileInfo fileInfo(fileName);
		StatusBarManager::pushMessage("Stream saved: " + fileInfo.fileName(), 3000);
	});
	connect(this, &ProfileGeneratorWidget::saveStreamFailed, this, [this](const QString &error) {
		m_saveStreamToFileBtn->stopAnimation();
		StatusBarManager::pushMessage("Save failed: " + error, 5000);
	});

	connect(this, &ProfileGeneratorWidget::loadToDeviceSuccess, this, [this]() {
		m_loadToDeviceBtn->stopAnimation();
		m_debugInfoText->setText("Profile loaded to device successfully");
		StatusBarManager::pushMessage("Profile loaded to device successfully", 3000);
	});
	connect(this, &ProfileGeneratorWidget::loadToDeviceFailed, this, [this](const QString &error) {
		m_loadToDeviceBtn->stopAnimation();
		m_debugInfoText->setText("Load failed: " + error);
		StatusBarManager::pushMessage("Load failed: " + error, 5000);
	});

	qInfo(CAT_PROFILEGENERATORWIDGET) << "Signal connections established with worker thread communication";
}

void ProfileGeneratorWidget::onSampleRateChangedSynchronized(const QString &newSampleRate)
{
	// Critical iio-oscilloscope compatibility: Sample rate synchronization
	// When ANY channel sample rate changes, ALL channels update to same value
	// Matches set_all_cb_to_same_text() from iio-oscilloscope exactly

	if(m_updatingFromPreset) {
		return; // Prevent recursive updates during preset loading
	}

	// Only synchronize in LTE mode - Live Device mode preserves individual values
	if(isLiveDeviceModeActive()) {
		qDebug(CAT_PROFILEGENERATORWIDGET) << "Live Device mode - skipping sample rate synchronization";
		updateProfileData();
		return;
	}

	// Block signals to prevent recursive calls during bulk update
	m_updatingFromPreset = true;

	// Update ALL channel sample rates to the same value (iio-oscilloscope behavior)
	for(int i = 0; i < 4; ++i) {
		QComboBox *sampleRateCombo = m_channelWidgets[i]->getSampleRateCombo();
		if(sampleRateCombo && sampleRateCombo->currentText() != newSampleRate) {
			sampleRateCombo->setCurrentText(newSampleRate);
		}
	}

	// Auto-update corresponding bandwidths for each channel (LTE mode only)
	QString correspondingBandwidth = calculateBandwidthForSampleRate(newSampleRate);

	for(int i = 0; i < 4; ++i) {
		// In LTE mode: clear bandwidth combo and set only the calculated value
		m_channelWidgets[i]->setBandwidthOptions(QStringList() << correspondingBandwidth);
		m_channelWidgets[i]->setBandwidthReadOnly(true); // Read-only like iio-oscilloscope
	}

	m_updatingFromPreset = false;

	// Update profile data after all changes complete
	updateProfileData();
}

bool ProfileGeneratorWidget::readDeviceConfiguration()
{
	// Phase 1: Read profile_config for high-level configuration
	QString profileConfigText = readDeviceAttribute("profile_config");

	if(profileConfigText.isEmpty() || profileConfigText.startsWith("error", Qt::CaseInsensitive) ||
	   profileConfigText.startsWith("Device not", Qt::CaseInsensitive)) {
		qWarning(CAT_PROFILEGENERATORWIDGET) << "Profile config unavailable:" << profileConfigText;
		return false;
	}

	try {
		// Phase 2: Parse high-level config
		m_deviceConfig = DeviceConfigurationParser::parseProfileConfig(m_device, profileConfigText);

		return true;
	} catch(const std::exception &e) {
		qWarning(CAT_PROFILEGENERATORWIDGET) << "Failed to parse device configuration:" << e.what();
		return false;
	}
}

iio_channel *ProfileGeneratorWidget::findIIOChannel(const QString &channelName, bool isOutput)
{
	if(!m_device) {
		return nullptr;
	}

	return iio_device_find_channel(m_device, channelName.toLocal8Bit().data(), isOutput);
}

void ProfileGeneratorWidget::populateUIFromDeviceConfig(const DeviceConfigurationParser::ParsedDeviceConfig &config)
{

	// Block signals during bulk update to prevent cascading updates
	m_updatingFromPreset = true;

	// Update radio configuration - SSI interface from device configuration
	m_ssiInterfaceLabel->setText(config.ssiInterface); // Read-only label like iio-oscilloscope
	m_duplexModeCombo->setCurrentText(config.duplexMode);

	// Update only device-specific values in the complete template (preserve all template parameters)
	m_radioConfig.lvds = (config.ssiInterface == "LVDS");
	m_radioConfig.ssi_lanes = config.ssiLanes;
	m_radioConfig.fdd = (config.duplexMode == "FDD");

	// FIRST: Set all channels to Live Device mode to prepare for device data
	for(int i = 0; i < 4; ++i) {
		m_channelWidgets[i]->updateControlsVisibility(false); // Live Device mode - bandwidth editable
	}

	// THEN: Populate channel data from device configuration ONLY if data is available
	for(int i = 0; i < 4; ++i) {
		ChannelConfigWidget::ChannelData data;
		bool shouldPopulateData = false;

		if(i < 2) { // RX channels (RX1, RX2)
			const auto &rxConfig = config.rxChannels[i];
			data.enabled = rxConfig.enabled; // Always set enabled state from mask

			if(rxConfig.hasChannelData) {
				// Only populate channel data if it was successfully read from IIO attributes
				data.freqOffsetCorrectionEnable = rxConfig.freqOffsetCorrectionEnable;
				data.channelBandwidthHz = rxConfig.channelBandwidthHz;
				data.sampleRateHz = rxConfig.sampleRateHz;
				data.rfPort = rxConfig.rfPort;
				shouldPopulateData = true;

				// Update template with device values
				m_radioConfig.rx_config[i].enabled = rxConfig.enabled;
				m_radioConfig.rx_config[i].freqOffsetCorrectionEnable =
					rxConfig.freqOffsetCorrectionEnable;
				m_radioConfig.rx_config[i].channelBandwidthHz = rxConfig.channelBandwidthHz;
				m_radioConfig.rx_config[i].sampleRateHz = rxConfig.sampleRateHz;
				m_radioConfig.rx_config[i].rfPort = rxConfig.rfPort;
			}
		} else { // TX channels (TX1, TX2)
			int txIndex = i - 2;
			const auto &txConfig = config.txChannels[txIndex];
			data.enabled = txConfig.enabled; // Always set enabled state from mask

			if(txConfig.hasChannelData) {
				// Only populate channel data if it was successfully read from IIO attributes
				data.freqOffsetCorrectionEnable = txConfig.freqOffsetCorrectionEnable;
				data.channelBandwidthHz = txConfig.channelBandwidthHz;
				data.sampleRateHz = txConfig.sampleRateHz;
				shouldPopulateData = true;

				// Update template with device values
				m_radioConfig.tx_config[txIndex].enabled = txConfig.enabled;
				m_radioConfig.tx_config[txIndex].freqOffsetCorrectionEnable =
					txConfig.freqOffsetCorrectionEnable;
				m_radioConfig.tx_config[txIndex].channelBandwidthHz = txConfig.channelBandwidthHz;
				m_radioConfig.tx_config[txIndex].sampleRateHz = txConfig.sampleRateHz;
			}
		}

		// Only update UI if we have valid data to populate
		if(shouldPopulateData) {
			m_channelWidgets[i]->setChannelData(data);
		} else {
			// Only set the enabled state if no channel data is available
			ChannelConfigWidget::ChannelData enabledOnlyData;
			enabledOnlyData.enabled = data.enabled;
			m_channelWidgets[i]->setChannelData(enabledOnlyData);
		}
	}

	m_updatingFromPreset = false;

	// Trigger UI state updates after populating data
	updateOrxControls();
	updateProfileData();
}

void ProfileGeneratorWidget::resetPresetTracking()
{
	// Reset internal state like iio-oscilloscope does
	m_lastAppliedPreset = "";
	m_updatingFromPreset = false;
	// Clear any cached device config
	m_deviceConfig = DeviceConfigurationParser::ParsedDeviceConfig();
}

void ProfileGeneratorWidget::forceUpdateAllUIControls()
{
	// Force update ALL UI controls without preserving any state (like iio-oscilloscope)

	m_updatingFromPreset = true;

	// Update radio configuration from device
	m_ssiInterfaceLabel->setText(m_deviceConfig.ssiInterface);
	m_duplexModeCombo->setCurrentText(m_deviceConfig.duplexMode);

	// Update internal config to match device values
	m_radioConfig.lvds = (m_deviceConfig.ssiInterface == "LVDS");
	m_radioConfig.ssi_lanes = m_deviceConfig.ssiLanes;
	m_radioConfig.fdd = (m_deviceConfig.duplexMode == "FDD");

	// Force update all 4 channel widgets with device data (enabled states completely ignored)
	for(int i = 0; i < 4; ++i) {
		ChannelConfigWidget::ChannelData data;
		if(i < 2) { // RX channels (RX1, RX2)
			const auto &rxConfig = m_deviceConfig.rxChannels[i];
			// Don't set data.enabled - let UI checkbox maintain its state naturally
			data.freqOffsetCorrectionEnable = rxConfig.freqOffsetCorrectionEnable;
			data.channelBandwidthHz = rxConfig.channelBandwidthHz;
			data.sampleRateHz = rxConfig.sampleRateHz;
			data.rfPort = rxConfig.rfPort;
		} else { // TX channels (TX1, TX2)
			int txIndex = i - 2;
			const auto &txConfig = m_deviceConfig.txChannels[txIndex];
			// Don't set data.enabled - let UI checkbox maintain its state naturally
			data.freqOffsetCorrectionEnable = txConfig.freqOffsetCorrectionEnable;
			data.channelBandwidthHz = txConfig.channelBandwidthHz;
			data.sampleRateHz = txConfig.sampleRateHz;
		}

		// Update channel data (enabled state untouched by design)
		m_channelWidgets[i]->setChannelData(data);

		// Force update the actual UI combo boxes with device values (like populate_combo_box)
		updateChannelUIControls(i, data);
	}

	// Preserve current ORX enabled states (user-controlled only) - don't update from device
	// ORX enabled states are also user-controlled and should not be changed by refresh

	m_updatingFromPreset = false;
}

bool ProfileGeneratorWidget::readAndApplyDeviceConfiguration()
{
	// Always read from device like iio-oscilloscope
	if(!readDeviceConfiguration()) {
		qWarning(CAT_PROFILEGENERATORWIDGET) << "Failed to read device configuration";
		return false;
	}

	// Apply to current UI state without preserving anything
	forceUpdateAllUIControls();
	return true;
}

void ProfileGeneratorWidget::applyLTEConstraintsToDeviceData()
{
	// Apply LTE constraints to device-read data:
	// - Force bandwidth read-only mode
	// - Apply sample rate restrictions for SSI lanes
	// - Maintain LTE UI control visibility

	for(int i = 0; i < 4; ++i) {
		m_channelWidgets[i]->updateControlsVisibility(true);
	}

	// Apply sample rate restrictions for current SSI configuration
	updateSampleRateOptionsForSSI();
}

void ProfileGeneratorWidget::updateChannelUIControls(int channelIndex, const ChannelConfigWidget::ChannelData &data)
{
	// Update actual combo box contents like iio-oscilloscope's populate_combo_box()
	// This ensures the UI visually shows device values, not just internal data

	if(!m_channelWidgets[channelIndex]) {
		return;
	}

	// Force device values (like iio-oscilloscope populate_combo_box with TRUE parameter)
	QStringList bandwidthOptions = QStringList() << QString::number(data.channelBandwidthHz);
	m_channelWidgets[channelIndex]->setBandwidthOptions(bandwidthOptions, true);

	QStringList sampleRateOptions = QStringList() << QString::number(data.sampleRateHz);
	m_channelWidgets[channelIndex]->setSampleRateOptions(sampleRateOptions, true);
}

QWidget *ProfileGeneratorWidget::generateDeviceDriverAPIWidget(QWidget *parent)
{
	QWidget *driverAPIWidget = new QWidget(parent);
	Style::setBackgroundColor(driverAPIWidget, json::theme::background_primary);
	Style::setStyle(driverAPIWidget, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(driverAPIWidget);
	layout->setContentsMargins(10, 10, 10, 10);
	layout->setSpacing(2);

	// Device Driver API (like ADRV9002 controls)
	QLabel *deviceApiTitle = new QLabel("Device Driver API: ", driverAPIWidget);
	Style::setStyle(deviceApiTitle, style::properties::label::menuSmall);
	layout->addWidget(deviceApiTitle);

	QString deviceApiVersion = getDeviceDriverVersion();
	QLabel *deviceApiLabel = new QLabel(deviceApiVersion, driverAPIWidget);
	Style::setStyle(deviceApiLabel, style::properties::label::subtle);
	layout->addWidget(deviceApiLabel);

	// Profile Generator CLI
	QLabel *title = new QLabel("Profile Generator CLI: ", driverAPIWidget);
	Style::setStyle(title, style::properties::label::menuSmall);
	layout->addWidget(title);

	bool showDownloadBtn = false;

	if(m_cliAvailable) {

		// Enable all functionality
		m_contentWidget->setEnabled(true);

		// CLI Version label
		QString cliVersion = m_cliManager ? m_cliManager->getCliVersion() : "unknown";
		QLabel *versionLabel = new QLabel(cliVersion, driverAPIWidget);
		Style::setStyle(versionLabel, style::properties::label::subtle);
		layout->addWidget(versionLabel);

		if(!cliVersion.contains(deviceApiVersion)) {
			QString cliVersionNotMatching =
				"Locally installed CLI version is not a match for Device Driver API some of the "
				"functionalities might not work as expected";
			QLabel *cliVersionNotMatchingLabel = new QLabel(cliVersionNotMatching, driverAPIWidget);
			Style::setStyle(cliVersionNotMatchingLabel, style::properties::label::warning);
			layout->addWidget(cliVersionNotMatchingLabel);
			showDownloadBtn = true;
		}

	} else {
		// CLI not available - show error status
		title->setText("Profile Generator CLI not found. "
			       "Advanced profile generation features are disabled. "
			       "Please install the ADRV9002 Profile Generator CLI tool to enable this functionality. "
			       "Refer to the ADI documentation for installation instructions."
			       "Compatible CLI version must match Device Driver API");

		Style::setStyle(title, style::properties::label::warning);

		// Disable all controls
		m_contentWidget->setEnabled(false);
		showDownloadBtn = true;
	}

	if(showDownloadBtn) {
		// Add download info button with proper Scopy styling right under the error message
		QPushButton *infoBtn = new QPushButton("Download Profile Generator CLI", this);
		infoBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
		Style::setStyle(infoBtn, style::properties::button::basicButton);
		connect(infoBtn, &QPushButton::clicked, this, &ProfileGeneratorWidget::onDownloadCLI);

		// Insert button right after the status label (position 1 in mainLayout)
		layout->addWidget(infoBtn);
	}

	return driverAPIWidget;
}

#include "moc_profilegeneratorwidget.cpp"
