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

#include <profilemanager.h>

#include <QLoggingCategory>
#include <QFileInfo>
#include <style.h>
#include <stylehelper.h>
#include <smallprogressbar.h>
#include <QtConcurrent>
#include <QThread>

Q_LOGGING_CATEGORY(CAT_PROFILEMANAGER, "ProfileManager")

using namespace scopy::adrv9002;
using namespace scopy;

ProfileManager::ProfileManager(iio_device *device, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
	, m_title(nullptr)
	, m_profileLabel(nullptr)
	, m_profileFileBrowser(nullptr)
	, m_streamLabel(nullptr)
	, m_streamFileBrowser(nullptr)
	, m_deviceInfoText(nullptr)
{
	// Create main layout with horizontal split like iio-oscilloscope
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(0);

	// Title
	m_title = new QLabel("Profile & Stream Configuration", this);
	Style::setStyle(m_title, style::properties::label::menuMedium);
	mainLayout->addWidget(m_title);

	// Create horizontal layout for Profile controls (left) and Device Info (right)
	QHBoxLayout *contentLayout = new QHBoxLayout();
	contentLayout->setMargin(0);
	contentLayout->setSpacing(15);
	mainLayout->addLayout(contentLayout);

	// Left side: Profile and Stream controls
	QWidget *profileControlsWidget = new QWidget();
	QVBoxLayout *leftLayout = new QVBoxLayout(profileControlsWidget);
	leftLayout->setMargin(0);

	// Get embedded resources directory
	QFileInfoList profiles = scopy::PkgManager::listFilesInfo(QStringList() << "adrv9002-profiles");
	QString defaultDir =
		(profiles.size() > 0) ? profiles.first().absolutePath() : scopy::PkgManager::packagesPath();

	// Profile section
	m_profileLabel = new QLabel("Load Profile:", this);
	leftLayout->addWidget(m_profileLabel);

	// Create profile container with progress bar (following data logger pattern)
	QWidget *profileContainer = new QWidget();
	QVBoxLayout *profileLayout = new QVBoxLayout(profileContainer);
	profileLayout->setMargin(0);

	m_profileFileBrowser = new scopy::FileBrowserWidget(scopy::FileBrowserWidget::OPEN_FILE, profileContainer);
	m_profileFileBrowser->setFilter("Profile files (*.json)");
	m_profileFileBrowser->setBaseDirectory(defaultDir);
	m_profileFileBrowser->lineEdit()->setReadOnly(true);
	m_profileFileBrowser->lineEdit()->setPlaceholderText("(None)");

	m_profileProgressBar = new scopy::SmallProgressBar(profileContainer);

	profileLayout->addWidget(m_profileFileBrowser);
	profileLayout->addWidget(m_profileProgressBar);
	leftLayout->addWidget(profileContainer);

	// Stream section
	m_streamLabel = new QLabel("Load Stream:", this);
	leftLayout->addWidget(m_streamLabel);

	// Create stream container with progress bar (following data logger pattern)
	QWidget *streamContainer = new QWidget();
	QVBoxLayout *streamLayout = new QVBoxLayout(streamContainer);
	streamLayout->setMargin(0);
	streamLayout->setSpacing(1);

	m_streamFileBrowser = new scopy::FileBrowserWidget(scopy::FileBrowserWidget::OPEN_FILE, streamContainer);
	m_streamFileBrowser->setFilter("Stream files (*.stream)");
	m_streamFileBrowser->setBaseDirectory(defaultDir);
	m_streamFileBrowser->lineEdit()->setReadOnly(true);
	m_streamFileBrowser->lineEdit()->setPlaceholderText("(None)");

	m_streamProgressBar = new scopy::SmallProgressBar(streamContainer);

	streamLayout->addWidget(m_streamFileBrowser);
	streamLayout->addWidget(m_streamProgressBar);
	leftLayout->addWidget(streamContainer);

	// Add spacer to push profile controls to top
	leftLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	// Right side: Device Information Panel (matching iio-oscilloscope)
	QWidget *deviceInfoPanel = createDeviceInfoPanel();

	// Add both sides to horizontal layout
	contentLayout->addWidget(profileControlsWidget);
	contentLayout->addWidget(deviceInfoPanel);

	// Set proportions: 50% profile controls, 50% device info
	contentLayout->setStretch(0, 50);
	contentLayout->setStretch(1, 50);

	// Connect FileBrowserWidget signals
	connect(m_profileFileBrowser->lineEdit(), &QLineEdit::textChanged, this, &ProfileManager::onProfileFileChanged);
	connect(m_streamFileBrowser->lineEdit(), &QLineEdit::textChanged, this, &ProfileManager::onStreamFileChanged);

	// Initial status update
	updateStatus();
}

ProfileManager::~ProfileManager() {}

QString ProfileManager::title() const { return m_title->text(); }

void ProfileManager::setTitle(const QString &newTitle) { m_title->setText(newTitle); }

QWidget *ProfileManager::createDeviceInfoPanel()
{
	QWidget *panel = new QWidget();
	Style::setStyle(panel, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(panel);
	layout->setContentsMargins(10, 10, 10, 10);
	layout->setSpacing(5);

	QLabel *title = new QLabel("Profile config attribute value: ", panel);
	Style::setStyle(title, style::properties::label::menuSmall);
	layout->addWidget(title);

	// Device info text box - scrollable display showing profile_config
	m_deviceInfoText = new QTextEdit();
	m_deviceInfoText->setReadOnly(true);
	m_deviceInfoText->setFont(QFont("monospace", 9));
	m_deviceInfoText->setMinimumHeight(200);

	layout->addWidget(m_deviceInfoText);

	// Initial update of device info
	updateDeviceInfo();

	return panel;
}

QString ProfileManager::getAttributeValue(const QString &attributeName)
{
	if(!m_device)
		return "Device not available";

	char buffer[8192]; // Large buffer for profile_config
	int ret = iio_device_attr_read(m_device, attributeName.toUtf8().constData(), buffer, sizeof(buffer));

	if(ret < 0) {
		return QString("Error reading %1: %2").arg(attributeName).arg(ret);
	}

	return QString(buffer);
}

void ProfileManager::updateDeviceInfo()
{
	QString profileConfig = getAttributeValue("profile_config");

	if(profileConfig.startsWith("error", Qt::CaseInsensitive) ||
	   profileConfig.startsWith("Device not", Qt::CaseInsensitive)) {
		m_deviceInfoText->setPlainText("Device information unavailable:\n" + profileConfig);
	} else {
		// Display the profile_config content directly (like iio-oscilloscope)
		m_deviceInfoText->setPlainText(profileConfig);
	}
}

void ProfileManager::refreshStatus() { updateStatus(); }

void ProfileManager::onProfileFileChanged()
{
	if(!m_profileFileBrowser || !m_profileFileBrowser->lineEdit()) {
		return;
	}

	QString filename = m_profileFileBrowser->lineEdit()->text();
	if(filename.isEmpty() || filename == "(None)") {
		return;
	}

	// Execute heavy work with progress indication
	executeWithProgress(true, [this, filename]() -> bool {
		// Move existing load logic to worker thread
		bool success = loadProfileFromFile(filename);

		// UI feedback via main thread
		QMetaObject::invokeMethod(
			this,
			[this, success, filename]() {
				if(success) {
					m_currentProfilePath = filename;
					Q_EMIT profileLoaded(filename);
					updateStatus();

					// Additional feedback
					QFileInfo fileInfo(filename);
					QLineEdit *profileEdit = m_profileFileBrowser->lineEdit();
					profileEdit->setToolTip(
						QString("Profile loaded successfully: %1").arg(fileInfo.fileName()));
					scopy::StatusBarManager::pushMessage(
						QString("Profile loaded: %1").arg(fileInfo.fileName()), 3000);
					qInfo(CAT_PROFILEMANAGER) << "Profile loaded successfully:" << filename;
				} else {
					QString errorMsg = QString("Failed to load profile from: %1").arg(filename);
					Q_EMIT profileError(errorMsg);
					qWarning(CAT_PROFILEMANAGER) << errorMsg;
					scopy::StatusBarManager::pushMessage(errorMsg, 5000);

					// Additional feedback
					QLineEdit *profileEdit = m_profileFileBrowser->lineEdit();
					profileEdit->setToolTip(errorMsg);
				}
			},
			Qt::QueuedConnection);

		return success;
	});
}

void ProfileManager::onStreamFileChanged()
{
	if(!m_streamFileBrowser || !m_streamFileBrowser->lineEdit()) {
		return;
	}

	QString filename = m_streamFileBrowser->lineEdit()->text();
	if(filename.isEmpty() || filename == "(None)") {
		return;
	}

	// Execute heavy work with progress indication
	executeWithProgress(false, [this, filename]() -> bool {
		// Move existing load logic to worker thread
		bool success = loadStreamFromFile(filename);

		// UI feedback via main thread
		QMetaObject::invokeMethod(
			this,
			[this, success, filename]() {
				if(success) {
					m_currentStreamPath = filename;
					Q_EMIT streamLoaded(filename);
					updateStatus();

					// Additional feedback
					QFileInfo fileInfo(filename);
					QLineEdit *streamEdit = m_streamFileBrowser->lineEdit();
					streamEdit->setToolTip(
						QString("Stream loaded successfully: %1").arg(fileInfo.fileName()));
					scopy::StatusBarManager::pushMessage(
						QString("Stream loaded: %1").arg(fileInfo.fileName()), 3000);
					qInfo(CAT_PROFILEMANAGER) << "Stream loaded successfully:" << filename;
				} else {
					QString errorMsg = QString("Failed to load stream from: %1").arg(filename);
					Q_EMIT streamError(errorMsg);
					qWarning(CAT_PROFILEMANAGER) << errorMsg;
					scopy::StatusBarManager::pushMessage(errorMsg, 5000);

					// Additional feedback
					QLineEdit *streamEdit = m_streamFileBrowser->lineEdit();
					streamEdit->setToolTip(errorMsg);
				}
			},
			Qt::QueuedConnection);

		return success;
	});
}

void ProfileManager::updateStatus()
{
	// Update profile status in FileBrowserWidget
	if(m_profileFileBrowser) {
		QString status = readDeviceAttribute("profile_config");
		QLineEdit *profileEdit = m_profileFileBrowser->lineEdit();

		if(status.isEmpty() || status == "ERROR") {
			if(profileEdit->text().isEmpty()) {
				profileEdit->setPlaceholderText("(None)");
			}
		} else {
			// Show current file path if available, otherwise show status
			if(!m_currentProfilePath.isEmpty()) {
				QFileInfo fileInfo(m_currentProfilePath);
				// Don't change text if user selected a file, just update styling
				if(profileEdit->text().isEmpty()) {
					profileEdit->setText(fileInfo.fileName());
				}
			}
		}
	}

	// Update stream status in FileBrowserWidget
	if(m_streamFileBrowser) {
		QString status = readDeviceAttribute("stream_config");
		QLineEdit *streamEdit = m_streamFileBrowser->lineEdit();

		if(status.isEmpty() || status == "ERROR") {
			if(streamEdit->text().isEmpty()) {
				streamEdit->setPlaceholderText("(None)");
			}
		} else {
			// Show current file path if available, otherwise show status
			if(!m_currentStreamPath.isEmpty()) {
				QFileInfo fileInfo(m_currentStreamPath);
				// Don't change text if user selected a file, just update styling
				if(streamEdit->text().isEmpty()) {
					streamEdit->setText(fileInfo.fileName());
				}
			}
		}
	}
}

bool ProfileManager::loadProfileFromFile(const QString &filename)
{
	if(!m_device) {
		qWarning(CAT_PROFILEMANAGER) << "No device available for profile loading";
		return false;
	}

	// Read file content
	QFile file(filename);
	if(!file.open(QIODevice::ReadOnly)) {
		qWarning(CAT_PROFILEMANAGER) << "Failed to open profile file:" << filename;
		return false;
	}

	QByteArray profileData = file.readAll();
	file.close();

	if(profileData.isEmpty()) {
		qWarning(CAT_PROFILEMANAGER) << "Profile file is empty:" << filename;
		return false;
	}

	// Write to device attribute
	bool success = writeDeviceAttribute("profile_config", profileData);
	if(success) {
		qDebug(CAT_PROFILEMANAGER) << "Profile data written to device successfully";
	} else {
		qWarning(CAT_PROFILEMANAGER) << "Failed to write profile data to device";
	}

	return success;
}

bool ProfileManager::loadStreamFromFile(const QString &filename)
{
	if(!m_device) {
		qWarning(CAT_PROFILEMANAGER) << "No device available for stream loading";
		return false;
	}

	// Read file content - ensure binary mode for .stream files
	QFile file(filename);
	if(!file.open(QIODevice::ReadOnly)) {
		qWarning(CAT_PROFILEMANAGER) << "Failed to open stream file:" << filename;
		return false;
	}

	QByteArray streamData = file.readAll();
	file.close();

	if(streamData.isEmpty()) {
		qWarning(CAT_PROFILEMANAGER) << "Stream file is empty:" << filename;
		return false;
	}

	// Write to device attribute
	bool success = writeDeviceAttribute("stream_config", streamData);
	if(success) {
		qDebug(CAT_PROFILEMANAGER) << "Stream data written to device successfully";
	} else {
		qWarning(CAT_PROFILEMANAGER) << "Failed to write stream data to device";
	}

	return success;
}

QString ProfileManager::readDeviceAttribute(const QString &attributeName)
{
	if(!m_device) {
		return QString();
	}

	char buffer[1024];
	int ret = iio_device_attr_read(m_device, attributeName.toLocal8Bit().data(), buffer, sizeof(buffer));

	if(ret > 0) {
		return QString::fromLocal8Bit(buffer, ret).trimmed();
	} else {
		qDebug(CAT_PROFILEMANAGER) << "Failed to read device attribute:" << attributeName << "ret:" << ret;
		return QString();
	}
}

bool ProfileManager::writeDeviceAttribute(const QString &attributeName, const QByteArray &data)
{
	if(!m_device) {
		qWarning(CAT_PROFILEMANAGER) << "No device available for writing attribute:" << attributeName;
		return false;
	}

	// Follow the original iio-oscilloscope pattern for error checking
	int ret =
		iio_device_attr_write_raw(m_device, attributeName.toLocal8Bit().data(), data.constData(), data.size());

	if(ret < 0) {
		// Negative return value indicates an error
		qWarning(CAT_PROFILEMANAGER) << "Failed to write attribute:" << attributeName << "error code:" << ret;
		return false;
	} else {
		// Positive return value indicates success (bytes written)
		qDebug(CAT_PROFILEMANAGER) << "Successfully wrote" << ret << "bytes to attribute:" << attributeName
					   << "(expected:" << data.size() << ")";
		return true;
	}
}

void ProfileManager::updateProfileStatus(ProgressBarState status)
{
	if(status == ProgressBarState::SUCCESS) {
		m_profileProgressBar->setBarColor(Style::getAttribute(json::theme::content_success));
	}
	if(status == ProgressBarState::ERROR) {
		m_profileProgressBar->setBarColor(Style::getAttribute(json::theme::content_error));
	}
	if(status == ProgressBarState::BUSY) {
		m_profileProgressBar->startProgress();
		m_profileProgressBar->setBarColor(Style::getAttribute(json::theme::content_busy));
	}
}

void ProfileManager::updateStreamStatus(ProgressBarState status)
{
	if(status == ProgressBarState::SUCCESS) {
		m_streamProgressBar->setBarColor(Style::getAttribute(json::theme::content_success));
	}
	if(status == ProgressBarState::ERROR) {
		m_streamProgressBar->setBarColor(Style::getAttribute(json::theme::content_error));
	}
	if(status == ProgressBarState::BUSY) {
		m_streamProgressBar->startProgress();
		m_streamProgressBar->setBarColor(Style::getAttribute(json::theme::content_busy));
	}
}

void ProfileManager::executeWithProgress(bool isProfile, std::function<bool()> work)
{
	// Set progress bar to BUSY state (equivalent to startAnimation)
	if(isProfile) {
		updateProfileStatus(ProgressBarState::BUSY);
	} else {
		updateStreamStatus(ProgressBarState::BUSY);
	}

	QFutureWatcher<bool> *watcher = new QFutureWatcher<bool>(this);
	connect(
		watcher, &QFutureWatcher<bool>::finished, this,
		[this, watcher, isProfile]() {
			bool success = watcher->result();

			// Set progress bar to final state (equivalent to stopAnimation)
			if(isProfile) {
				updateProfileStatus(success ? ProgressBarState::SUCCESS : ProgressBarState::ERROR);
			} else {
				updateStreamStatus(success ? ProgressBarState::SUCCESS : ProgressBarState::ERROR);
			}

			watcher->deleteLater();
		},
		Qt::QueuedConnection);

	QFuture<bool> future = QtConcurrent::run([work]() { return work(); });
	watcher->setFuture(future);
}
