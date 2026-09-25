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

#ifndef PROFILECLIMANAGER_H
#define PROFILECLIMANAGER_H

#include "scopy-adrv9002plugin_export.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QFileInfo>
#include <QStandardPaths>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QLoggingCategory>
#include <qcorotask.h>
#include <profilegeneratortypes.h>

Q_DECLARE_LOGGING_CATEGORY(CAT_PROFILECLIMANAGER)

namespace scopy {
namespace component {
class Device;
class Channel;
} // namespace component
} // namespace scopy

namespace scopy::adrv9002 {

class SCOPY_ADRV9002PLUGIN_EXPORT ProfileCliManager : public QObject
{
	Q_OBJECT

public:
	explicit ProfileCliManager(component::Device *device, QObject *parent = nullptr);
	~ProfileCliManager();

	// CLI detection and validation
	bool isCliAvailable() const;
	QString getCliVersion() const;
	QString getCliPath() const;

	// Profile operations following iio-oscilloscope pattern (main-thread coroutines)
	QCoro::Task<void> saveProfileToFile(QString filename, RadioConfig config);
	QCoro::Task<void> saveStreamToFile(QString filename, RadioConfig config);
	QCoro::Task<void> loadProfileToDevice(RadioConfig config);

	// Configuration preview for debug display
	QString generateConfigPreview(const RadioConfig &config);

Q_SIGNALS:
	void operationProgress(const QString &message);
	void operationError(const QString &error);

private:
	// CLI management (based on iio-oscilloscope profile_gen_cli_get_cmd)
	bool detectCli();
	bool validateCliVersion();

	// Config operations (simple JSON config, not full profile)
	bool writeConfigToTempFile(const QString &filename, const RadioConfig &config);
	QString createConfigJson(const RadioConfig &config);

	// CLI execution (non-blocking coroutine over QProcess)
	QCoro::Task<bool> executeCli(QStringList arguments, QString *output);

	// File operations
	QByteArray readFileContents(const QString &filename);
	QCoro::Task<bool> writeDeviceAttribute(QString attribute, QByteArray data);

	// Cleanup helpers
	void cleanupTempFiles(const QStringList &files);

	// Member variables
	component::Device *m_device;
	bool m_cliAvailable;
	QString m_cliPath;
	QString m_cliVersion;

	// CLI constants
	static const QString CLI_NAME;
	static const int CLI_TIMEOUT_MS;
};

} // namespace scopy::adrv9002

#endif // PROFILECLIMANAGER_H
