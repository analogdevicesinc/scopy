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
#include <array>
#include <iio.h>
#include <pluginbase/statusbarmanager.h>
#include <iiowidget.h>
#include <style.h>
#include <channelconfigwidget.h>
#include <gui/widgets/menusectionwidget.h>
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
		bool enabled;
		QString bandwidth;
		QString sampleRate;
		bool freqOffsetCorrection;
		QString rfPort;	 // RX only
		bool orxEnabled; // TX only
	};

	struct ParsedDeviceConfig
	{
		QString duplexMode;   // "TDD"/"FDD"
		QString ssiInterface; // "CMOS"/"LVDS"
		int ssiLanes;	      // 1, 2, or 4

		ParsedChannelData rxChannels[2];
		ParsedChannelData txChannels[2];

		// Clock config
		QString deviceClock;
		QString clockDivider;
	};

	static ParsedDeviceConfig parseProfileConfig(const QString &profileConfigText);
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

public Q_SLOTS:
	void refreshProfileData();

private Q_SLOTS:
	void onPresetChanged();
	void onRefreshProfile();
	void onSaveToFile();
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

	// Preset Management
	void applyLTEDefaults();
	void applyLTEPresetLogic();
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

	// Validation helpers
	void validateChannelConfiguration();
	void updateAllDependentControls();

	// Phase 2: Complete UI Refresh System
	void refreshAllUIStates();

	// Channel control helpers
	bool getChannelEnabled(ChannelType channel);

	// Enhanced Signal Connection Architecture (simplified)
	void setupEnhancedConnections();

	// Frequency table helpers
	void onSampleRateChangedSynchronized(const QString &newSampleRate); // iio-oscilloscope compatibility
	void updateSampleRateOptionsForSSI();
	QString calculateBandwidthForSampleRate(const QString &sampleRate) const;

	// Profile Operations (require CLI)
	bool loadProfileToDevice();
	bool saveProfileToFile(const QString &filename);

	// Device Communication
	QString readDeviceAttribute(const QString &attributeName);

	// Phase 1: Device Configuration Reading
	bool readDeviceConfiguration();
	void populateUIFromDeviceConfig(const DeviceConfigurationParser::ParsedDeviceConfig &config);
	bool isDeviceConfigurationAvailable();
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
	QPushButton *m_saveToFileBtn;
	QPushButton *m_loadToDeviceBtn;

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
