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

#include <profileclimanager.h>
#include <QStandardPaths>

Q_LOGGING_CATEGORY(CAT_PROFILECLIMANAGER, "ProfileCliManager")

using namespace scopy::adrv9002;

// Static constants
const QString ProfileCliManager::CLI_NAME = "adrv9002-iio-cli";
const int ProfileCliManager::CLI_TIMEOUT_MS = 30000; // 30 seconds

// Helper function for boolean to numeric conversion (matching iio-oscilloscope cJSON_AddNumberToObject)
static int boolToInt(bool value) { return value ? 1 : 0; }

ProfileCliManager::ProfileCliManager(iio_device *device, QObject *parent)
	: QObject(parent)
	, m_device(device)
	, m_cliAvailable(false)
	, m_cliPath("")
	, m_cliVersion("unknown")
{
	// Detect CLI availability on construction
	m_cliAvailable = detectCli();

	if(m_cliAvailable) {
		qInfo(CAT_PROFILECLIMANAGER)
			<< "Profile Generator CLI detected:" << m_cliPath << "Version:" << m_cliVersion;
	} else {
		qWarning(CAT_PROFILECLIMANAGER) << "Profile Generator CLI not available";
	}
}

ProfileCliManager::~ProfileCliManager() = default;

bool ProfileCliManager::isCliAvailable() const { return m_cliAvailable; }

QString ProfileCliManager::getCliVersion() const { return m_cliVersion; }

QString ProfileCliManager::getCliPath() const { return m_cliPath; }

// CLI Detection (based on iio-oscilloscope profile_gen_cli_get_cmd)
bool ProfileCliManager::detectCli()
{
	QStringList searchPaths;
// Search for CLI tool in common locations
#ifdef Q_OS_WIN
	// Windows-specific paths
	searchPaths.append("C:/Program Files/Analog Devices/bin/");
	searchPaths.append("C:/Program Files (x86)/Analog Devices/bin/");
	searchPaths.append(QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + "/");
	searchPaths.append(QCoreApplication::applicationDirPath() + "/");
#else
	// Unix-like systems (Linux, macOS)
	searchPaths.append("/usr/bin/");
	searchPaths.append("/usr/local/bin/");
	searchPaths.append("/opt/analog/bin/");
	searchPaths.append(QCoreApplication::applicationDirPath() + "/");
	searchPaths.append(QDir::homePath() + "/.local/bin/");
#endif

	for(const QString &path : searchPaths) {
		QString fullPath = path + CLI_NAME;
#ifdef Q_OS_WIN
		if(!fullPath.endsWith(".exe")) {
			fullPath += ".exe";
		}
#endif
		if(QFile::exists(fullPath) && QFileInfo(fullPath).isExecutable()) {
			m_cliPath = QDir::toNativeSeparators(fullPath);
			return validateCliVersion();
		}
	}

	// Also check PATH environment variable
	QString pathResult = QStandardPaths::findExecutable(CLI_NAME);
	if(!pathResult.isEmpty()) {
		m_cliPath = QDir::toNativeSeparators(pathResult);
		return validateCliVersion();
	}

	return false;
}

bool ProfileCliManager::validateCliVersion()
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

	return true;
}

// Profile Operations
ProfileCliManager::OperationResult ProfileCliManager::saveProfileToFile(const QString &filename,
									const RadioConfig &config)
{
	if(!m_cliAvailable) {
		Q_EMIT operationError("Profile Generator CLI not available");
		return CliNotAvailable;
	}

	// Create temporary config file
	QDir tempDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation));
	QString configFile = tempDir.filePath("adrv9002_config.json");

	if(!writeConfigToTempFile(configFile, config)) {
		Q_EMIT operationError("Failed to write configuration file");
		return ConfigGenerationFailed;
	}

	// Execute CLI command (following iio-oscilloscope pattern)
	QStringList arguments;
	arguments << "--config" << configFile << "--profile" << filename;

	QString output, errorOutput;
	bool success = executeCli(arguments, output, errorOutput);

	// Cleanup temp file
	cleanupTempFiles(QStringList() << configFile);

	if(!success) {
		Q_EMIT operationError(QString("CLI execution failed: %1").arg(errorOutput));
		return CliExecutionFailed;
	}

	Q_EMIT operationProgress("Profile saved successfully");
	return Success;
}

ProfileCliManager::OperationResult ProfileCliManager::saveStreamToFile(const QString &filename,
								       const RadioConfig &config)
{
	if(!m_cliAvailable) {
		Q_EMIT operationError("Profile Generator CLI not available");
		return CliNotAvailable;
	}

	// Create temporary config file
	QDir tempDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation));
	QString configFile = tempDir.filePath("adrv9002_config.json");

	if(!writeConfigToTempFile(configFile, config)) {
		Q_EMIT operationError("Failed to write configuration file");
		return ConfigGenerationFailed;
	}

	// Execute CLI command for stream image
	QStringList arguments;
	arguments << "--config" << configFile << "--stream" << filename;

	QString output, errorOutput;
	bool success = executeCli(arguments, output, errorOutput);

	// Cleanup temp file
	cleanupTempFiles(QStringList() << configFile);

	if(!success) {
		Q_EMIT operationError(QString("CLI execution failed: %1").arg(errorOutput));
		return CliExecutionFailed;
	}

	Q_EMIT operationProgress("Stream image saved successfully");
	return Success;
}

ProfileCliManager::OperationResult ProfileCliManager::loadProfileToDevice(const RadioConfig &config)
{
	if(!m_cliAvailable) {
		Q_EMIT operationError("Profile Generator CLI not available");
		return CliNotAvailable;
	}

	// Create temporary files (following iio-oscilloscope pattern exactly)
	QDir tempDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation));
	QString configFile = tempDir.filePath("adrv9002_config.json");
	QString profileFile = tempDir.filePath("adrv9002_profile.json");
	QString streamFile = tempDir.filePath("adrv9002_stream.json");

	QStringList tempFiles = {configFile, profileFile, streamFile};

	// Write config to temp file
	if(!writeConfigToTempFile(configFile, config)) {
		Q_EMIT operationError("Failed to write configuration file");
		cleanupTempFiles(tempFiles);
		return ConfigGenerationFailed;
	}

	Q_EMIT operationProgress("Generating profile and stream files...");

	// Execute CLI command to generate both profile and stream
	QStringList arguments;
	arguments << "--config" << configFile << "--profile" << profileFile << "--stream" << streamFile;

	QString output, errorOutput;
	bool success = executeCli(arguments, output, errorOutput);

	if(!success) {
		Q_EMIT operationError(QString("CLI execution failed: %1").arg(errorOutput));
		cleanupTempFiles(tempFiles);
		return CliExecutionFailed;
	}

	Q_EMIT operationProgress("Loading profile to device...");

	// Read generated profile file
	QByteArray profileData = readFileContents(profileFile);
	if(profileData.isEmpty()) {
		Q_EMIT operationError("Failed to read generated profile file");
		cleanupTempFiles(tempFiles);
		return FileOperationFailed;
	}

	// Write profile to device
	if(!writeDeviceAttribute("profile_config", profileData)) {
		Q_EMIT operationError("Failed to write profile to device");
		cleanupTempFiles(tempFiles);
		return DeviceWriteFailed;
	}

	// Read generated stream file
	QByteArray streamData = readFileContents(streamFile);
	if(streamData.isEmpty()) {
		Q_EMIT operationError("Failed to read generated stream image file");
		cleanupTempFiles(tempFiles);
		return FileOperationFailed;
	}

	// Write stream to device
	// int ret = iio_device_attr_write_raw(m_device, "profile_config", streamData, sizeof(streamData));

	if(!writeDeviceAttribute("stream_config", streamData)) {
		Q_EMIT operationError("Failed to write stream image to device");
		cleanupTempFiles(tempFiles);
		return DeviceWriteFailed;
	}

	Q_EMIT operationProgress("Stream_config write completed successfully");

	// Cleanup temp files
	cleanupTempFiles(tempFiles);

	return Success;
}

// Config JSON Generation (simple format for CLI input)
QString ProfileCliManager::generateConfigPreview(const RadioConfig &config) { return createConfigJson(config); }

QString ProfileCliManager::createConfigJson(const RadioConfig &config)
{
	QJsonObject configJson;

	// Radio configuration - USE NUMERIC VALUES LIKE IOO (matching cJSON_AddNumberToObject)
	QJsonObject radioCfg;
	radioCfg["ssi_lanes"] = static_cast<int>(config.ssi_lanes);
	radioCfg["ddr"] = boolToInt(config.ddr);		  // 1/0 not true/false
	radioCfg["short_strobe"] = boolToInt(config.shortStrobe); // 1/0 not true/false
	radioCfg["lvds"] = boolToInt(config.lvds);		  // 1/0 not true/false
	radioCfg["adc_rate_mode"] = config.adcRateMode;
	radioCfg["fdd"] = boolToInt(config.fdd); // 1/0 not true/false

	// RX configuration array - ALL BOOLEAN VALUES AS NUMERIC (matching iio-oscilloscope)
	QJsonArray rxConfigArray;
	for(int i = 0; i < 2; i++) {
		QJsonObject rxCh;
		rxCh["enabled"] = boolToInt(config.rx_config[i].enabled);
		rxCh["adc_high_performance_mode"] = boolToInt(config.rx_config[i].adcHighPerformanceMode);
		rxCh["frequency_offset_correction_enable"] = boolToInt(config.rx_config[i].freqOffsetCorrectionEnable);
		rxCh["analog_filter_power_mode"] = config.rx_config[i].analogFilterPowerMode;
		rxCh["analog_filter_biquad"] = boolToInt(config.rx_config[i].analogFilterBiquad);
		rxCh["analog_filter_bandwidth_hz"] = static_cast<qint32>(config.rx_config[i].analogFilterBandwidthHz);
		rxCh["channel_bandwidth_hz"] = static_cast<qint32>(config.rx_config[i].channelBandwidthHz);
		rxCh["sample_rate_hz"] = static_cast<qint32>(config.rx_config[i].sampleRateHz);
		rxCh["nco_enable"] = boolToInt(config.rx_config[i].ncoEnable);
		rxCh["nco_frequency_hz"] = config.rx_config[i].ncoFrequencyHz;
		rxCh["rf_port"] = config.rx_config[i].rfPort;

		rxConfigArray.append(rxCh);
	}
	radioCfg["rx_config"] = rxConfigArray;

	// TX configuration array - ALL BOOLEAN VALUES AS NUMERIC (matching iio-oscilloscope)
	QJsonArray txConfigArray;
	for(int i = 0; i < 2; i++) {
		QJsonObject txCh;
		txCh["enabled"] = boolToInt(config.tx_config[i].enabled);
		txCh["sample_rate_hz"] = static_cast<qint32>(config.tx_config[i].sampleRateHz);
		txCh["frequency_offset_correction_enable"] = boolToInt(config.tx_config[i].freqOffsetCorrectionEnable);
		txCh["analog_filter_power_mode"] = config.tx_config[i].analogFilterPowerMode;
		txCh["channel_bandwidth_hz"] = static_cast<qint32>(config.tx_config[i].channelBandwidthHz);
		txCh["orx_enabled"] = boolToInt(config.tx_config[i].orxEnabled);
		txCh["elb_type"] = config.tx_config[i].elbType;

		txConfigArray.append(txCh);
	}
	radioCfg["tx_config"] = txConfigArray;

	configJson["radio_cfg"] = radioCfg;

	// Clock configuration - ALL BOOLEAN VALUES AS NUMERIC (matching iio-oscilloscope)
	QJsonObject clkCfg;
	clkCfg["device_clock_frequency_khz"] = static_cast<qint32>(config.clk_config.deviceClockFrequencyKhz);
	clkCfg["device_clock_output_enable"] = boolToInt(config.clk_config.deviceClockOutputEnable);
	clkCfg["device_clock_output_divider"] = config.clk_config.deviceClockOutputDivider;
	clkCfg["clock_pll_high_performance_enable"] = boolToInt(config.clk_config.clockPllHighPerformanceEnable);
	clkCfg["clock_pll_power_mode"] = config.clk_config.clockPllPowerMode;
	clkCfg["processor_clock_divider"] = config.clk_config.processorClockDivider;

	configJson["clk_cfg"] = clkCfg;

	QJsonDocument doc(configJson);
	return doc.toJson(QJsonDocument::Compact);
}

bool ProfileCliManager::writeConfigToTempFile(const QString &filename, const RadioConfig &config)
{
	QString jsonConfig = createConfigJson(config);

	QFile file(filename);
	if(!file.open(QIODevice::WriteOnly)) {
		qWarning(CAT_PROFILECLIMANAGER) << "Failed to write config file:" << filename;
		return false;
	}

	file.write(jsonConfig.toUtf8());
	file.close();

	return true;
}

// CLI Execution
bool ProfileCliManager::executeCli(const QStringList &arguments, QString &output, QString &errorOutput)
{
	QString workingDir = QFileInfo(m_cliPath).absolutePath();

	QProcess process;
	process.setProgram(m_cliPath);
	process.setArguments(arguments);
	process.setWorkingDirectory(workingDir);
	process.start();

	if(!process.waitForStarted(5000)) {
		errorOutput = QString("Failed to start CLI: %1").arg(process.errorString());
		return false;
	}

	if(!process.waitForFinished(CLI_TIMEOUT_MS)) {
		process.kill();
		if(!process.waitForFinished(3000)) { // Give it time to cleanup
			process.terminate();	     // Force terminate if kill didn't work
		}
		errorOutput = "CLI execution timeout";
		return false;
	}

	output = process.readAllStandardOutput();
	errorOutput = process.readAllStandardError();

	if(process.exitCode() != 0) {
		qWarning(CAT_PROFILECLIMANAGER)
			<< "CLI command failed with exit code:" << process.exitCode() << "Error:" << errorOutput;
		return false;
	}

	return true;
}

// File Operations
QByteArray ProfileCliManager::readFileContents(const QString &filename)
{
	QFile file(filename);
	if(!file.open(QIODevice::ReadOnly)) {
		qWarning(CAT_PROFILECLIMANAGER) << "Failed to read file:" << filename;
		return QByteArray();
	}

	QByteArray data = file.readAll();
	file.close();

	return data;
}

bool ProfileCliManager::writeDeviceAttribute(const QString &attribute, const QByteArray &data)
{
	if(!m_device) {
		qWarning(CAT_PROFILECLIMANAGER) << "No device available for writing attribute:" << attribute;
		return false;
	}

	int ret = iio_device_attr_write_raw(m_device, attribute.toLocal8Bit().data(), data.constData(), data.size());

	if(ret > 0) {
		return true;
	} else {
		qWarning(CAT_PROFILECLIMANAGER)
			<< "Failed to write attribute:" << attribute << "expected:" << data.size() << "written:" << ret;
		return false;
	}
}

void ProfileCliManager::cleanupTempFiles(const QStringList &files)
{
	for(const QString &file : files) {
		if(QFile::exists(file)) {
			QFile::remove(file);
		}
	}
}

#include "moc_profileclimanager.cpp"
