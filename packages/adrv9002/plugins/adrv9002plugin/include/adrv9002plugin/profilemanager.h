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

#ifndef PROFILEMANAGER_H
#define PROFILEMANAGER_H

#include "scopy-adrv9002plugin_export.h"
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QString>
#include <gui/widgets/filebrowserwidget.h>
#include <gui/widgets/smallprogressbar.h>
#include <pkg-manager/pkgmanager.h>
#include <pluginbase/statusbarmanager.h>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <functional>
#include <iio.h>

namespace scopy::adrv9002 {

enum ProgressBarState
{
	SUCCESS,
	ERROR,
	BUSY
};

class SCOPY_ADRV9002PLUGIN_EXPORT ProfileManager : public QWidget
{
	Q_OBJECT

public:
	explicit ProfileManager(iio_device *device, QWidget *parent = nullptr);
	~ProfileManager();

	QString title() const;
	void setTitle(const QString &newTitle);

Q_SIGNALS:
	void profileLoaded(const QString &filename);
	void streamLoaded(const QString &filename);
	void profileError(const QString &errorMessage);
	void streamError(const QString &errorMessage);

public Q_SLOTS:
	void refreshStatus();
	void updateDeviceInfo();
	void updateProfileStatus(ProgressBarState status);
	void updateStreamStatus(ProgressBarState status);

private Q_SLOTS:
	void onProfileFileChanged();
	void doOnProfileFileChangedInThread();
	void onStreamFileChanged();
	void updateStatus();

private:
	// Core functionality
	bool loadProfileFromFile(const QString &filename);
	bool loadStreamFromFile(const QString &filename);
	QString readDeviceAttribute(const QString &attributeName);
	bool writeDeviceAttribute(const QString &attributeName, const QByteArray &data);

	// Device info panel
	QWidget *createDeviceInfoPanel();
	QString getAttributeValue(const QString &attributeName);

	// Progress bar animation helper for threaded operations
	void executeWithProgress(bool isProfile, std::function<bool()> work);

	// Device communication
	iio_device *m_device;

	// UI components - following FastlockProfilesWidget pattern
	QLabel *m_title;

	// Profile section
	QLabel *m_profileLabel;
	scopy::FileBrowserWidget *m_profileFileBrowser;
	scopy::SmallProgressBar *m_profileProgressBar;

	// Stream section
	QLabel *m_streamLabel;
	scopy::FileBrowserWidget *m_streamFileBrowser;
	scopy::SmallProgressBar *m_streamProgressBar;

	// Device info panel
	QTextEdit *m_deviceInfoText;

	// Current file paths
	QString m_currentProfilePath;
	QString m_currentStreamPath;
};

} // namespace scopy::adrv9002

#endif // PROFILEMANAGER_H
