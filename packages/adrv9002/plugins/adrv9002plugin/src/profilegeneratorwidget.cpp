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
DeviceConfigurationParser::parseProfileConfig(const QString &profileConfigText)
{
	ParsedDeviceConfig config;

	qDebug(CAT_PROFILEGENERATORWIDGET) << "Parsing device profile configuration";

	// Parse duplex mode - look for "Duplex Mode:" line
	QString duplexMode = extractValueBetween(profileConfigText, "Duplex Mode:", "\n");
	config.duplexMode = duplexMode.contains("TDD", Qt::CaseInsensitive) ? "TDD" : "FDD";

	// Parse SSI interface - look for "SSI interface:" line
	QString ssiInterface = extractValueBetween(profileConfigText, "SSI interface:", "\n");
	config.ssiInterface = ssiInterface.contains("LVDS", Qt::CaseInsensitive) ? "LVDS" : "CMOS";
	config.ssiLanes = ssiInterface.contains("LVDS", Qt::CaseInsensitive) ? 2 : 4;

	// Parse device clock info
	config.deviceClock = extractValueBetween(profileConfigText, "Device clk(Hz):", "\n");
	config.clockDivider = extractValueBetween(profileConfigText, "ARM Power Saving Clk Divider:", "\n");

	// Parse channel configurations (matching iio-oscilloscope logic)
	for(int ch = 0; ch < 2; ch++) {
		// RX channel parsing
		QString rxPrefix = QString("RX%1").arg(ch + 1);
		config.rxChannels[ch].enabled =
			extractValueBetween(profileConfigText, rxPrefix + " enabled:", "\n").contains("1");
		config.rxChannels[ch].sampleRate =
			extractValueBetween(profileConfigText, rxPrefix + " sample rate(Hz):", "\n");
		config.rxChannels[ch].bandwidth =
			extractValueBetween(profileConfigText, rxPrefix + " channel bandwidth(Hz):", "\n");
		config.rxChannels[ch].freqOffsetCorrection =
			extractValueBetween(profileConfigText, rxPrefix + " frequency offset correction enable:", "\n")
				.contains("1");

		QString rxRfPort = extractValueBetween(profileConfigText, rxPrefix + " RF port:", "\n");
		config.rxChannels[ch].rfPort = mapRfPortFromDevice(rxRfPort, ch);

		// TX channel parsing
		QString txPrefix = QString("TX%1").arg(ch + 1);
		config.txChannels[ch].enabled =
			extractValueBetween(profileConfigText, txPrefix + " enabled:", "\n").contains("1");
		config.txChannels[ch].sampleRate =
			extractValueBetween(profileConfigText, txPrefix + " sample rate(Hz):", "\n");
		config.txChannels[ch].bandwidth =
			extractValueBetween(profileConfigText, txPrefix + " channel bandwidth(Hz):", "\n");
		config.txChannels[ch].freqOffsetCorrection =
			extractValueBetween(profileConfigText, txPrefix + " frequency offset correction enable:", "\n")
				.contains("1");
		config.txChannels[ch].orxEnabled =
			extractValueBetween(profileConfigText, txPrefix + " ORx enabled:", "\n").contains("1");
	}

	qDebug(CAT_PROFILEGENERATORWIDGET) << "Parsed device config - Duplex:" << config.duplexMode
					   << "SSI:" << config.ssiInterface << "Lanes:" << config.ssiLanes;

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
		loadPresetData("LTE");

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

	m_saveToFileBtn = new AnimatedLoadingButton("Save to file", this);

	m_loadToDeviceBtn = new AnimatedLoadingButton("Load to device", this);

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
		// Apply LTE defaults exactly matching lte_defaults() function
		applyLTEDefaults();
	} else if(presetName == "Live Device") {
		// Read current device configuration and populate UI
		qDebug(CAT_PROFILEGENERATORWIDGET) << "Loading Live Device preset from device configuration";

		if(isDeviceConfigurationAvailable()) {
			if(readDeviceConfiguration()) {
				populateUIFromDeviceConfig(m_deviceConfig);
				qDebug(CAT_PROFILEGENERATORWIDGET) << "Live Device configuration loaded successfully";
			} else {
				qWarning(CAT_PROFILEGENERATORWIDGET) << "Failed to read device configuration";
				StatusBarManager::pushMessage("Failed to read Live Device configuration", 5000);
			}
		} else {
			qWarning(CAT_PROFILEGENERATORWIDGET) << "Device configuration not available";
			StatusBarManager::pushMessage("Live Device configuration not available", 5000);
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
	// Radio config - read from device-determined SSI interface label
	QString ssiInterface = m_ssiInterfaceLabel->text();
	m_radioConfig.lvds = (ssiInterface == "LVDS");
	m_radioConfig.fdd = (m_duplexModeCombo->currentText() == "FDD");
	m_radioConfig.ssi_lanes = m_radioConfig.lvds ? 2 : 4;

	// Channel configs - unified processing using loop
	for(int i = 0; i < 4; ++i) {
		auto data = m_channelWidgets[i]->getChannelData();

		if(i < 2) { // RX channels (RX1, RX2)
			m_radioConfig.rx_config[i].enabled = data.enabled;
			m_radioConfig.rx_config[i].freqOffsetCorrection = data.freqOffsetCorrection;
			m_radioConfig.rx_config[i].bandwidth = data.bandwidth;
			m_radioConfig.rx_config[i].sampleRate = data.sampleRate;
			m_radioConfig.rx_config[i].rfInput = data.rfInput;
		} else { // TX channels (TX1, TX2)
			int txIndex = i - 2;
			m_radioConfig.tx_config[txIndex].enabled = data.enabled;
			m_radioConfig.tx_config[txIndex].freqOffsetCorrection = data.freqOffsetCorrection;
			m_radioConfig.tx_config[txIndex].bandwidth = data.bandwidth;
			m_radioConfig.tx_config[txIndex].sampleRate = data.sampleRate;
		}
	}

	// ORX
	m_radioConfig.orx_enabled[0] = m_orx1EnabledCb->isChecked();
	m_radioConfig.orx_enabled[1] = m_orx2EnabledCb->isChecked();
}

void ProfileGeneratorWidget::updateDebugInfo()
{
	QJsonObject debugInfo;

	// Add current preset information
	QString currentPreset = m_presetCombo->currentText();
	debugInfo["current_preset"] = currentPreset;
	debugInfo["cli_available"] = m_cliAvailable;

	// Add Live Device configuration if available and preset is Live Device
	if(isLiveDeviceModeActive() && isDeviceConfigurationAvailable()) {
		QJsonObject deviceConfig;
		deviceConfig["duplex_mode"] = m_deviceConfig.duplexMode;
		deviceConfig["ssi_interface"] = m_deviceConfig.ssiInterface;
		deviceConfig["ssi_lanes"] = m_deviceConfig.ssiLanes;
		deviceConfig["device_clock"] = m_deviceConfig.deviceClock;
		deviceConfig["clock_divider"] = m_deviceConfig.clockDivider;

		// RX channels from device
		QJsonArray rxChannelsDevice;
		for(int i = 0; i < 2; i++) {
			QJsonObject rxCh;
			rxCh["enabled"] = m_deviceConfig.rxChannels[i].enabled;
			rxCh["bandwidth"] = m_deviceConfig.rxChannels[i].bandwidth;
			rxCh["sample_rate"] = m_deviceConfig.rxChannels[i].sampleRate;
			rxCh["freq_offset_correction"] = m_deviceConfig.rxChannels[i].freqOffsetCorrection;
			rxCh["rf_port"] = m_deviceConfig.rxChannels[i].rfPort;
			rxChannelsDevice.append(rxCh);
		}
		deviceConfig["rx_channels"] = rxChannelsDevice;

		// TX channels from device
		QJsonArray txChannelsDevice;
		for(int i = 0; i < 2; i++) {
			QJsonObject txCh;
			txCh["enabled"] = m_deviceConfig.txChannels[i].enabled;
			txCh["bandwidth"] = m_deviceConfig.txChannels[i].bandwidth;
			txCh["sample_rate"] = m_deviceConfig.txChannels[i].sampleRate;
			txCh["freq_offset_correction"] = m_deviceConfig.txChannels[i].freqOffsetCorrection;
			txCh["orx_enabled"] = m_deviceConfig.txChannels[i].orxEnabled;
			txChannelsDevice.append(txCh);
		}
		deviceConfig["tx_channels"] = txChannelsDevice;

		debugInfo["device_configuration"] = deviceConfig;
	}

	// Add CLI configuration preview (shows what will be sent to CLI tool)
	if(m_cliAvailable && m_cliManager) {
		QString configPreview = m_cliManager->generateConfigPreview(m_radioConfig);
		QJsonDocument configDoc = QJsonDocument::fromJson(configPreview.toUtf8());
		debugInfo["cli_configuration"] = configDoc.object();
	}

	QJsonDocument doc(debugInfo);
	m_debugInfoText->setPlainText(doc.toJson(QJsonDocument::Indented));
}

// Slot implementations
void ProfileGeneratorWidget::onPresetChanged()
{
	QString presetName = m_presetCombo->currentText();

	// This fixes the Live Device → LTE → Live Device transition bug
	if(presetName == "Live Device") {
		// Switching TO Live Device mode: Restore full bandwidth options for all channels
		for(int i = 0; i < 4; ++i) {
			m_channelWidgets[i]->setBandwidthOptions(FrequencyTable::BANDWIDTHS_HZ);
		}
	}

	// Update all channel widgets based on preset mode
	bool isLTEMode = isLTEModeActive();
	for(int i = 0; i < 4; ++i) {
		m_channelWidgets[i]->updateControlsVisibility(isLTEMode);
	}

	if(presetName == "Live Device") {
		// Live Device preset: Auto-refresh from device like iio-oscilloscope
		onRefreshProfile();
	} else {
		// LTE preset: Apply static configuration
		loadPresetData(presetName);
	}
}

void ProfileGeneratorWidget::onRefreshProfile()
{
	if(!m_cliAvailable) {
		StatusBarManager::pushMessage("Profile Generator CLI not available", 5000);
		return;
	}

	resetPresetTracking();

	qDebug(CAT_PROFILEGENERATORWIDGET) << "Refreshing profile from device (iio-oscilloscope style)";

	if(!isDeviceConfigurationAvailable()) {
		qDebug(CAT_PROFILEGENERATORWIDGET) << "Device configuration not available", 5000;
		return;
	}

	if(!readAndApplyDeviceConfiguration()) {
		qDebug(CAT_PROFILEGENERATORWIDGET) << "Failed to read device configuration";
		return;
	}

	QString currentPreset = m_presetCombo->currentText();
	if(isLTEModeActive()) {
		applyLTEConstraintsToDeviceData();
		qDebug(CAT_PROFILEGENERATORWIDGET) << "Applied LTE constraints to device data";
	}
	refreshAllUIStates();

	updateDebugInfo();
}

void ProfileGeneratorWidget::onSaveToFile()
{
	if(!m_cliAvailable) {
		qDebug(CAT_PROFILEGENERATORWIDGET)
			<< "Profile Generator CLI not available - cannot generate profile files";
		return;
	}

	// UI stuff stays on main thread (file dialog)
	QString filter = "Profile files (*.json);;Stream files (*.stream)";
	QString defaultName = QDir::homePath() + "/adrv9002_profile.json";
	QString fileName = QFileDialog::getSaveFileName(this, "Save File", defaultName, filter);

	if(!fileName.isEmpty()) {
		// UPDATE CONFIG ON MAIN THREAD BEFORE THREADING!
		updateConfigFromUI();

		// Copy data for safe worker thread access
		RadioConfig configCopy = m_radioConfig;

		// Determine file type and operation based on extension
		bool isStreamFile = fileName.toLower().endsWith(".stream");

		// Start animation
		m_saveToFileBtn->startAnimation();

		// Execute appropriate worker function - fire and forget
		if(isStreamFile) {
			QtConcurrent::run(this, &ProfileGeneratorWidget::doSaveStreamWork, fileName, configCopy);
		} else {
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

	qDebug(CAT_PROFILEGENERATORWIDGET) << "Updating sample rate options for SSI lanes:" << m_radioConfig.ssi_lanes
					   << "Available rates:" << availableRates.size();

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

	qDebug(CAT_PROFILEGENERATORWIDGET) << "Sample rate options update complete";
}

QString ProfileGeneratorWidget::calculateBandwidthForSampleRate(const QString &sampleRate) const
{
	return FrequencyTable::getBandwidthForSampleRate(sampleRate);
}

// LTE Defaults Implementation

void ProfileGeneratorWidget::applyLTEDefaults()
{
	qDebug(CAT_PROFILEGENERATORWIDGET) << "Applying LTE defaults from lte_defaults() function";

	m_updatingFromPreset = true;

	// Apply the pure LTE preset logic
	applyLTEPresetLogic();

	m_lastAppliedPreset = "LTE";
	m_updatingFromPreset = false;

	// Update dependent UI elements
	updateAllDependentControls();
}

void ProfileGeneratorWidget::applyLTEPresetLogic()
{
	// Radio config - exact values from lte_defaults()
	m_ssiInterfaceLabel->setText(LTEDefaults::SSI_INTERFACE);    // "LVDS"
	m_duplexModeCombo->setCurrentText(LTEDefaults::DUPLEX_MODE); // "TDD"

	// Apply LTE defaults while PRESERVING current enabled states
	for(int i = 0; i < 4; ++i) {
		// Get current enabled state to preserve it
		bool currentEnabledState = m_channelWidgets[i]->getChannelData().enabled;

		// Create LTE channel data with preserved enabled state
		ChannelConfigWidget::ChannelData lteData;
		lteData.enabled = currentEnabledState; // PRESERVE current enabled state
		lteData.freqOffsetCorrection = LTEDefaults::FREQ_OFFSET_CORRECTION;
		lteData.sampleRate = QString::number(LTEDefaults::SAMPLE_RATE_HZ);
		lteData.bandwidth = QString::number(LTEDefaults::BANDWIDTH_HZ);

		// Set RF input for RX channels only
		if(i < 2) { // RX channels (RX1, RX2)
			lteData.rfInput = (i == 0) ? "Rx1A" : "Rx2A";
		}

		m_channelWidgets[i]->setChannelData(lteData);
		m_channelWidgets[i]->updateControlsVisibility(true); // Bandwidth read-only, sample rate selectable
	}
}

bool ProfileGeneratorWidget::isLTEModeActive() const { return m_presetCombo && m_presetCombo->currentText() == "LTE"; }

bool ProfileGeneratorWidget::isLiveDeviceModeActive() const
{
	return m_presetCombo && m_presetCombo->currentText() == "Live Device";
}

// Signal Dependencies Helper Methods

void ProfileGeneratorWidget::validateChannelConfiguration()
{
	// Channel validation logic - currently minimal as per iio-oscilloscope approach
	// In Live Device mode - accept any device values without validation
	if(isLiveDeviceModeActive()) {
		qDebug(CAT_PROFILEGENERATORWIDGET) << "Live Device mode - skipping channel validation";
		return;
	}

	// Basic channel dependency validation for LTE mode
	qDebug(CAT_PROFILEGENERATORWIDGET) << "Validating channel configuration for LTE mode";
}

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

void ProfileGeneratorWidget::updateAllDependentControls()
{
	// Coordinate all UI updates that depend on configuration changes
	validateChannelConfiguration();
	updateOrxControls();
}

void ProfileGeneratorWidget::refreshAllUIStates()
{
	// Match iio-oscilloscope's profile_gen_preset_update() logic
	qDebug(CAT_PROFILEGENERATORWIDGET) << "Performing complete UI state refresh";

	// Use extracted coordination method
	updateAllDependentControls();
	updateProfileData();

	qDebug(CAT_PROFILEGENERATORWIDGET) << "Completed full UI state refresh";
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
	connect(m_saveToFileBtn, &QPushButton::clicked, this, &ProfileGeneratorWidget::onSaveToFile);
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
		m_saveToFileBtn->stopAnimation();
		QFileInfo fileInfo(fileName);
		StatusBarManager::pushMessage("Profile saved: " + fileInfo.fileName(), 3000);
	});
	connect(this, &ProfileGeneratorWidget::saveProfileFailed, this, [this](const QString &error) {
		m_saveToFileBtn->stopAnimation();
		StatusBarManager::pushMessage("Save failed: " + error, 5000);
	});

	connect(this, &ProfileGeneratorWidget::saveStreamSuccess, this, [this](const QString &fileName) {
		m_saveToFileBtn->stopAnimation();
		QFileInfo fileInfo(fileName);
		StatusBarManager::pushMessage("Stream saved: " + fileInfo.fileName(), 3000);
	});
	connect(this, &ProfileGeneratorWidget::saveStreamFailed, this, [this](const QString &error) {
		m_saveToFileBtn->stopAnimation();
		StatusBarManager::pushMessage("Save failed: " + error, 5000);
	});

	connect(this, &ProfileGeneratorWidget::loadToDeviceSuccess, this, [this]() {
		m_loadToDeviceBtn->stopAnimation();
		StatusBarManager::pushMessage("Profile loaded to device successfully", 3000);
	});
	connect(this, &ProfileGeneratorWidget::loadToDeviceFailed, this, [this](const QString &error) {
		m_loadToDeviceBtn->stopAnimation();
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

	qDebug(CAT_PROFILEGENERATORWIDGET) << "Synchronizing sample rate:" << newSampleRate;

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

	qDebug(CAT_PROFILEGENERATORWIDGET) << "Sample rate synchronization complete";
}

bool ProfileGeneratorWidget::readDeviceConfiguration()
{
	QString profileConfigText = readDeviceAttribute("profile_config");
	if(profileConfigText.isEmpty()) {
		qWarning(CAT_PROFILEGENERATORWIDGET) << "Failed to read profile_config from device";
		return false;
	}

	qDebug(CAT_PROFILEGENERATORWIDGET) << "Read profile_config from device, length:" << profileConfigText.length();

	try {
		m_deviceConfig = DeviceConfigurationParser::parseProfileConfig(profileConfigText);
		qDebug(CAT_PROFILEGENERATORWIDGET) << "Successfully parsed device configuration";
		return true;
	} catch(const std::exception &e) {
		qWarning(CAT_PROFILEGENERATORWIDGET) << "Failed to parse device configuration:" << e.what();
		return false;
	}
}

void ProfileGeneratorWidget::populateUIFromDeviceConfig(const DeviceConfigurationParser::ParsedDeviceConfig &config)
{
	qDebug(CAT_PROFILEGENERATORWIDGET) << "Populating UI from device configuration";

	// Block signals during bulk update to prevent cascading updates
	m_updatingFromPreset = true;

	// Update radio configuration - SSI interface from device configuration
	m_ssiInterfaceLabel->setText(config.ssiInterface); // Read-only label like iio-oscilloscope
	m_duplexModeCombo->setCurrentText(config.duplexMode);

	// Update internal config to match device values
	m_radioConfig.lvds = (config.ssiInterface == "LVDS");
	m_radioConfig.ssi_lanes = config.ssiLanes;

	// FIRST: Set all channels to Live Device mode to prepare for device data
	for(int i = 0; i < 4; ++i) {
		m_channelWidgets[i]->updateControlsVisibility(false); // Live Device mode - bandwidth editable
	}

	// THEN: Populate channel data from device configuration while PRESERVING enabled states
	for(int i = 0; i < 4; ++i) {

		ChannelConfigWidget::ChannelData data;
		if(i < 2) { // RX channels (RX1, RX2)
			const auto &rxConfig = config.rxChannels[i];
			data.freqOffsetCorrection = rxConfig.freqOffsetCorrection;
			data.bandwidth = rxConfig.bandwidth;
			data.sampleRate = rxConfig.sampleRate;
			data.rfInput = rxConfig.rfPort;
		} else { // TX channels (TX1, TX2)
			int txIndex = i - 2;
			const auto &txConfig = config.txChannels[txIndex];
			data.freqOffsetCorrection = txConfig.freqOffsetCorrection;
			data.bandwidth = txConfig.bandwidth;
			data.sampleRate = txConfig.sampleRate;
		}

		// Set channel data - mode already set above
		m_channelWidgets[i]->setChannelData(data);
	}

	m_updatingFromPreset = false;

	// Trigger UI state updates after populating data
	updateOrxControls();
	updateProfileData();

	qDebug(CAT_PROFILEGENERATORWIDGET) << "UI population from device config completed";
}

bool ProfileGeneratorWidget::isDeviceConfigurationAvailable()
{
	// Test if we can read profile_config attribute from device
	QString profileConfig = readDeviceAttribute("profile_config");
	bool available = !profileConfig.isEmpty();

	qDebug(CAT_PROFILEGENERATORWIDGET) << "Device configuration available:" << available;
	return available;
}

void ProfileGeneratorWidget::resetPresetTracking()
{
	// Reset internal state like iio-oscilloscope does
	m_lastAppliedPreset = "";
	m_updatingFromPreset = false;
	// Clear any cached device config
	m_deviceConfig = DeviceConfigurationParser::ParsedDeviceConfig();
	qDebug(CAT_PROFILEGENERATORWIDGET) << "Reset preset tracking and internal state";
}

void ProfileGeneratorWidget::forceUpdateAllUIControls()
{
	// Force update ALL UI controls without preserving any state (like iio-oscilloscope)
	qDebug(CAT_PROFILEGENERATORWIDGET) << "Forcing complete UI update from device configuration";

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
			data.freqOffsetCorrection = rxConfig.freqOffsetCorrection;
			data.bandwidth = rxConfig.bandwidth;
			data.sampleRate = rxConfig.sampleRate;
			data.rfInput = rxConfig.rfPort;
		} else { // TX channels (TX1, TX2)
			int txIndex = i - 2;
			const auto &txConfig = m_deviceConfig.txChannels[txIndex];
			// Don't set data.enabled - let UI checkbox maintain its state naturally
			data.freqOffsetCorrection = txConfig.freqOffsetCorrection;
			data.bandwidth = txConfig.bandwidth;
			data.sampleRate = txConfig.sampleRate;
		}

		// Update channel data (enabled state untouched by design)
		m_channelWidgets[i]->setChannelData(data);

		// Force update the actual UI combo boxes with device values (like populate_combo_box)
		updateChannelUIControls(i, data);
	}

	// Preserve current ORX enabled states (user-controlled only) - don't update from device
	// ORX enabled states are also user-controlled and should not be changed by refresh

	m_updatingFromPreset = false;

	qDebug(CAT_PROFILEGENERATORWIDGET) << "Complete UI force update finished";
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
	qDebug(CAT_PROFILEGENERATORWIDGET) << "Applying LTE constraints to device data";

	for(int i = 0; i < 4; ++i) {
		m_channelWidgets[i]->updateControlsVisibility(true); // LTE mode - bandwidth read-only
	}

	// Apply sample rate restrictions for current SSI configuration
	updateSampleRateOptionsForSSI();

	qDebug(CAT_PROFILEGENERATORWIDGET) << "LTE constraints applied";
}

void ProfileGeneratorWidget::updateChannelUIControls(int channelIndex, const ChannelConfigWidget::ChannelData &data)
{
	// Update actual combo box contents like iio-oscilloscope's populate_combo_box()
	// This ensures the UI visually shows device values, not just internal data

	if(!m_channelWidgets[channelIndex]) {
		return;
	}

	// Force device values (like iio-oscilloscope populate_combo_box with TRUE parameter)
	QStringList bandwidthOptions = QStringList() << data.bandwidth;
	m_channelWidgets[channelIndex]->setBandwidthOptions(bandwidthOptions, true);

	QStringList sampleRateOptions = QStringList() << data.sampleRate;
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

	// Title
	QLabel *title = new QLabel("Profile Generator CLI detected: ", driverAPIWidget);
	Style::setStyle(title, style::properties::label::menuSmall);
	layout->addWidget(title);

	if(m_cliAvailable) {

		// Enable all functionality
		m_contentWidget->setEnabled(true);

		// Version label - simple display matching iio-oscilloscope
		QString cliVersion = m_cliManager ? m_cliManager->getCliVersion() : "unknown";
		QLabel *versionLabel = new QLabel(cliVersion, driverAPIWidget);
		Style::setStyle(versionLabel, style::properties::label::subtle);
		layout->addWidget(versionLabel);

	} else {
		// CLI not available - show error status
		title->setText("Profile Generator CLI not found. "
			       "Advanced profile generation features are disabled. "
			       "Please install the ADRV9002 Profile Generator CLI tool to enable this functionality. "
			       "Refer to the ADI documentation for installation instructions.");

		Style::setStyle(title, style::properties::label::warning);

		// Disable all controls
		m_contentWidget->setEnabled(false);

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
