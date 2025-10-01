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
#include <iio.h>
#include <pluginbase/statusbarmanager.h>
#include <iiowidget.h>
#include <style.h>

namespace scopy::adrv9002 {

enum ChannelType
{
	CHANNEL_RX1,
	CHANNEL_RX2,
	CHANNEL_TX1,
	CHANNEL_TX2
};

struct ChannelConfig
{
	bool enabled;
	bool freqOffsetCorrection;
	QString bandwidth;
	QString sampleRate;
	QString rfInput; // For RX only
};

// Frequency table from iio-oscilloscope (exact Hz values)
class FrequencyTable
{
public:
	// Sample rates in Hz (from freq_table[0])
	static const QStringList SAMPLE_RATES_HZ;
	// Corresponding bandwidths in Hz (from freq_table[1])
	static const QStringList BANDWIDTHS_HZ;

	// Helper functions
	static int getIndexOfSampleRate(const QString &sampleRate);
	static QString getBandwidthForSampleRate(const QString &sampleRate);
	static QStringList getSampleRatesForSSILanes(int ssiLanes);
};

// RF Input options (descriptive names from screenshots)
class RFInputOptions
{
public:
	static const QStringList RX1_OPTIONS;
	static const QStringList RX2_OPTIONS;
};

// LTE defaults from lte_defaults() function
struct LTEDefaults
{
	// Channel defaults (Hz values)
	static const int SAMPLE_RATE_HZ;
	static const int BANDWIDTH_HZ;
	static const bool ENABLED;
	static const bool FREQ_OFFSET_CORRECTION;
	static const int RF_PORT;

	// Radio defaults
	static const QString SSI_INTERFACE;
	static const QString DUPLEX_MODE;
	static const int SSI_LANES;

	// Clock defaults
	static const int DEVICE_CLOCK_FREQUENCY_KHZ;
	static const bool DEVICE_CLOCK_OUTPUT_ENABLE;
	static const int DEVICE_CLOCK_OUTPUT_DIVIDER;
};

// Interface options
class IOOOptions
{
public:
	static const QStringList SSI_INTERFACE_OPTIONS;
	static const QStringList DUPLEX_MODE_OPTIONS;
};

struct OrxConfig
{
	bool enabled;
};

struct RadioConfig
{
	bool fdd;	   // Duplex mode (false=TDD, true=FDD)
	bool lvds;	   // SSI interface (true=LVDS, false=CMOS)
	uint8_t ssi_lanes; // 1, 2, or 4 lanes

	ChannelConfig rx_config[2]; // RX1, RX2
	ChannelConfig tx_config[2]; // TX1, TX2
	bool orx_enabled[2];	    // ORX1, ORX2
};

class SCOPY_ADRV9002PLUGIN_EXPORT ProfileGeneratorWidget : public QWidget
{
	Q_OBJECT

public:
	explicit ProfileGeneratorWidget(iio_device *device, QWidget *parent = nullptr);
	~ProfileGeneratorWidget();

Q_SIGNALS:
	void profileGenerated(const QString &profileData);
	void profileError(const QString &errorMessage);

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
	void onTxEnableChanged();
	void onTddModeChanged();
	void onInterfaceChanged();
	void onChannelEnableChanged();

private:
	// UI Creation Methods
	void setupUi();
	QWidget *createProfileActionBar();
	QWidget *createRadioConfigSection();
	QWidget *createChannelConfigSection();
	QWidget *createChannelConfigWidget(const QString &title, ChannelType type);
	QWidget *createOrxConfigSection();
	QWidget *createOrxWidget(const QString &title);
	QWidget *createDebugInfoSection();

	// CLI Detection & Management
	bool detectProfileGeneratorCLI();
	QString findCLIExecutable();
	bool validateCLIVersion();
	void setupUIBasedOnCLIAvailability();
	QStringList getCLISearchPatterns();

	// Data Management
	void loadPresetData(const QString &presetName);
	void updateDebugInfo();
	QJsonObject generateBasicProfile();
	void setupConnections();
	void updateConfigFromUI();
	void updateUIFromConfig();

	// Preset Management
	void applyLTEDefaults();
	bool validateConfiguration();

	// Channel Config Components structure
	struct ChannelWidgets
	{
		QCheckBox *enabledCb;
		QCheckBox *freqOffsetCb;
		QComboBox *bandwidthCombo;
		QComboBox *sampleRateCombo;
		QComboBox *rfInputCombo; // Only for RX
	};

	// Signal Dependencies
	void updateOrxVisibility();
	void updateChannelControlsVisibility();
	void updateOrxControlsState();
	bool getTxChannelEnabled(int channel);
	bool getTddModeEnabled();

	// Channel control helpers
	ChannelWidgets* getChannelWidgets(ChannelType channel);
	bool getChannelEnabled(ChannelType channel);
	void setChannelControlsEnabled(ChannelType channel, bool enabled);

	// Frequency table helpers
	void onSampleRateChanged(ChannelType channel);

	// Profile Operations (require CLI)
	bool generateProfile();
	bool loadProfileToDevice();
	bool saveProfileToFile(const QString &filename);
	bool writeConfigToFile(const QString &filename);
	QJsonObject createFullConfigJson();

	// Device Communication
	bool writeDeviceProfile(const QByteArray &profileData);
	bool writeDeviceStream(const QByteArray &streamData);
	QString readDeviceAttribute(const QString &attributeName);
	bool writeDeviceAttribute(const QString &attributeName, const QByteArray &data);

	// Device handle
	iio_device *m_device;

	// CLI availability
	bool m_cliAvailable;
	QString m_cliPath;
	QString m_cliVersion;

	// Profile data
	RadioConfig m_radioConfig;

	// Main UI components
	QLabel *m_statusLabel;
	QWidget *m_contentWidget;

	// Action Bar Components
	QComboBox *m_presetCombo;
	QPushButton *m_refreshProfileBtn;
	QPushButton *m_saveToFileBtn;
	QPushButton *m_loadToDeviceBtn;

	// Radio Config Components
	QComboBox *m_ssiInterfaceCombo;
	QComboBox *m_duplexModeCombo;

	// Channel Config Components (RX1, RX2, TX1, TX2)
	ChannelWidgets m_rx1Widgets;
	ChannelWidgets m_rx2Widgets;
	ChannelWidgets m_tx1Widgets;
	ChannelWidgets m_tx2Widgets;

	// ORX Components
	QCheckBox *m_orx1EnabledCb;
	QCheckBox *m_orx2EnabledCb;

	// Debug Info
	QTextEdit *m_debugInfoText;

	// Current file paths
	QString m_currentProfilePath;
	QString m_currentStreamPath;

	// State management
	QString m_lastAppliedPreset;
	bool m_updatingFromPreset;
};

} // namespace scopy::adrv9002

#endif // PROFILEGENERATORWIDGET_H
