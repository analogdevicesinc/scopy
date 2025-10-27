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

#ifndef PROFILEGENERATORWIDGET_H
#define PROFILEGENERATORWIDGET_H

#include "scopy-adrv9002plugin_export.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QTextEdit>
#include <QSpacerItem>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QString>
#include <QStandardPaths>
#include <QFile>
#include <QProcess>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QTimer>
#include <QFutureWatcher>
#include <functional>
#include <array>
#include <iio.h>
#include <pluginbase/statusbarmanager.h>
#include <iiowidget.h>
#include <style.h>
#include <channelconfigwidget.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/animatedrefreshbtn.h>
#include <gui/widgets/animatedloadingbutton.h>
#include <profilegeneratorconstants.h>
#include <profilegeneratortypes.h>
#include <profileclimanager.h>

namespace scopy::adrv9002 {

// Device Configuration Parser (Phase 1 addition)
class DeviceConfigurationParser
{

public:
	struct ParsedChannelData
	{
		//  Enable channel
		bool enabled;
		uint32_t analogFilterBandwidthHz;
		// Channel bandwidth of interest at ADC in Hz
		uint32_t channelBandwidthHz;
		// RX channel sample rate at digital interface
		uint32_t sampleRateHz;
		// Enable ADC frequency offset correction
		bool freqOffsetCorrectionEnable;
		// RF port source used for channel Options are:
		//        0 - RX_A
		//        1 - RX_B
		uint8_t rfPort; // RX only
		// Enable observation path
		bool orxEnabled; // TX only

		// Data source tracking (NEW)
		bool hasChannelData; // indicates if IIO channel attributes were read
		QString dataSource;  // "profile_config", "iio_attributes", or "unavailable"
	};

	struct ParsedDeviceConfig
	{
		QString duplexMode;   // "TDD"/"FDD"
		QString ssiInterface; // "CMOS"/"LVDS"
		int ssiLanes;	      // 1, 2, or 4

		ParsedChannelData rxChannels[2];
		ParsedChannelData txChannels[2];

		// Clock config
		uint32_t deviceClock;
		uint32_t clockDivider;
	};

	static ParsedDeviceConfig parseProfileConfig(iio_device *dev, const QString &profileConfigText);
	static QString extractValueBetween(const QString &text, const QString &begin, const QString &end);
	static QString mapRfPortFromDevice(const QString &devicePort, int channel);
};

class SCOPY_ADRV9002PLUGIN_EXPORT ProfileGeneratorWidget : public QWidget
{
	Q_OBJECT

public:
	explicit ProfileGeneratorWidget(iio_device *device, QWidget *parent = nullptr);
	~ProfileGeneratorWidget();

Q_SIGNALS:
	// Worker thread operation signals
	void saveProfileSuccess(const QString &fileName);
	void saveProfileFailed(const QString &error);
	void saveStreamSuccess(const QString &fileName);
	void saveStreamFailed(const QString &error);
	void loadToDeviceSuccess();
	void loadToDeviceFailed(const QString &error);

public Q_SLOTS:
	void refreshProfileData();

private Q_SLOTS:
	void onPresetChanged();
	void onRefreshProfile();
	void onSaveToFile(bool isStreamFile);
	void onLoadToDevice();
	void updateProfileData();
	void onDownloadCLI();

	// Signal dependency handlers
	void onChannelEnableChanged();

private:
	// UI Creation Methods
	void setupUi();
	MenuSectionCollapseWidget *createProfileActionSection();
	MenuSectionCollapseWidget *createRadioConfigSection();
	MenuSectionCollapseWidget *createChannelConfigSection();
	QWidget *createChannelConfigWidget(const QString &title, ChannelType type);
	MenuSectionCollapseWidget *createOrxConfigSection();
	QWidget *createOrxWidget(const QString &title);
	MenuSectionCollapseWidget *createDebugInfoSection();

	// Data Management
	void loadPresetData(const QString &presetName);
	void updateDebugInfo();
	void updateConfigFromUI();
	int getConfigFromCurrent(RadioConfig &config);
	int getConfigFromDefault(RadioConfig &config);
	int getConfigFromDevice(RadioConfig &config);
	void populateConfigFromUI(RadioConfig &config);
	void populateConfigFromDeviceData(RadioConfig &config,
					  const DeviceConfigurationParser::ParsedDeviceConfig &deviceConfig);

	// Preset Management
	void applyLTEDefaults();
	bool isLTEModeActive() const;
	bool isLiveDeviceModeActive() const;

	// Unified channel index system
	enum ChannelIndex
	{
		RX1 = 0,
		RX2 = 1,
		TX1 = 2,
		TX2 = 3
	};

	// Signal Dependencies
	void updateOrxControls();
	bool getTddModeEnabled();
	bool isOrxAvailable(int orxIndex);

	// Phase 2: Complete UI Refresh System
	void refreshAllUIStates();

	// Channel control helpers
	bool getChannelEnabled(ChannelType channel);

	// Enhanced Signal Connection Architecture (simplified)
	void setupEnhancedConnections();

	// Operation completion helper
	void handleOperationComplete(const QString &message, bool isSuccess);

	// Frequency table helpers
	void onSampleRateChangedSynchronized(const QString &newSampleRate); // iio-oscilloscope compatibility
	void updateSampleRateOptionsForSSI();
	QString calculateBandwidthForSampleRate(const QString &sampleRate) const;

	// Worker functions for threaded operations
	void doSaveProfileWork(const QString &fileName, const RadioConfig &config);
	void doSaveStreamWork(const QString &fileName, const RadioConfig &config);
	void doLoadToDeviceWork(const RadioConfig &config);

	// Device Communication
	QString readDeviceAttribute(const QString &attributeName);
	QString getDeviceDriverVersion();

	// Phase 1: Device Configuration Reading
	bool readDeviceConfiguration();
	iio_channel *findIIOChannel(const QString &channelName, bool isOutput);
	void populateUIFromDeviceConfig(const DeviceConfigurationParser::ParsedDeviceConfig &config);
	void resetPresetTracking();
	void forceUpdateAllUIControls();
	bool readAndApplyDeviceConfiguration();
	void applyLTEConstraintsToDeviceData();
	void updateChannelUIControls(int channelIndex, const ChannelConfigWidget::ChannelData &data);

	// Device handle
	iio_device *m_device;

	// CLI management
	ProfileCliManager *m_cliManager;
	bool m_cliAvailable;

	// Profile data
	RadioConfig m_radioConfig;
	DeviceConfigurationParser::ParsedDeviceConfig m_deviceConfig;

	// Main UI components
	QWidget *m_contentWidget;
	QWidget *generateDeviceDriverAPIWidget(QWidget *parent);

	// Action Bar Components
	QComboBox *m_presetCombo;
	QPushButton *m_refreshProfileBtn;
	AnimatedLoadingButton *m_saveStreamToFileBtn;
	AnimatedLoadingButton *m_saveProfileToFileBtn;
	AnimatedLoadingButton *m_loadToDeviceBtn;

	// Radio Config Components
	QLabel *m_ssiInterfaceLabel; // Read-only like iio-oscilloscope
	QComboBox *m_duplexModeCombo;

	// Unified channel widgets system - replaces old ChannelWidgets structs
	std::array<ChannelConfigWidget *, 4> m_channelWidgets; // RX1, RX2, TX1, TX2

	// ORX Components
	QCheckBox *m_orx1EnabledCb;
	QCheckBox *m_orx2EnabledCb;

	// Debug Info
	QTextEdit *m_debugInfoText;

	// State management
	QString m_lastAppliedPreset;
	bool m_updatingFromPreset;

	// ORX state - simplified for iio-oscilloscope compatibility (no user preference tracking)
};

} // namespace scopy::adrv9002

#endif // PROFILEGENERATORWIDGET_H
