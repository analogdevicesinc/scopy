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
#include <component/device.h>
#include <component/attribute.h>
#include <component/navigation.h>
#include <qcorotask.h>

Q_LOGGING_CATEGORY(CAT_PROFILEMANAGER, "ProfileManager")

using namespace scopy::adrv9002;
using namespace scopy;

ProfileManager::ProfileManager(component::Device *device, QWidget *parent)
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
	mainLayout->setContentsMargins(0, 0, 0, 0);

	// Title
	m_title = new QLabel("Profile & Stream Configuration", this);
	Style::setStyle(m_title, style::properties::label::menuMedium);
	mainLayout->addWidget(m_title);

	// Create horizontal layout for Profile controls (left) and Device Info (right)
	QHBoxLayout *contentLayout = new QHBoxLayout();
	contentLayout->setContentsMargins(0, 0, 0, 0);
	contentLayout->setSpacing(15);
	mainLayout->addLayout(contentLayout);

	// Left side: Profile and Stream controls
	QWidget *profileControlsWidget = new QWidget();
	QVBoxLayout *leftLayout = new QVBoxLayout(profileControlsWidget);
	leftLayout->setContentsMargins(0, 0, 0, 0);

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
	profileLayout->setContentsMargins(0, 0, 0, 0);

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
	streamLayout->setContentsMargins(0, 0, 0, 0);
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

QString ProfileManager::getAttributeValue(const QString &attributeName, size_t bytes)
{
	if(!m_device) {
		return "Device not available";
	}

	component::Attribute *a = component::attributeByName(m_device, attributeName);
	if(!a || !a->readCapability()) {
		return QString("Error reading %1").arg(attributeName);
	}

	return componentRead(attributeName, bytes);
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

QCoro::Task<void> ProfileManager::onProfileFileChanged()
{
	if(!m_profileFileBrowser || !m_profileFileBrowser->lineEdit()) {
		co_return;
	}

	QString filename = m_profileFileBrowser->lineEdit()->text();
	if(filename.isEmpty() || filename == "(None)") {
		co_return;
	}

	// Non-blocking load on the main thread with progress indication
	updateProfileStatus(ProgressBarState::BUSY);
	bool success = co_await loadProfileFromFile(filename);
	updateProfileStatus(success ? ProgressBarState::SUCCESS : ProgressBarState::ERROR);

	QLineEdit *profileEdit = m_profileFileBrowser->lineEdit();
	if(success) {
		m_currentProfilePath = filename;
		Q_EMIT profileLoaded(filename);
		updateStatus();

		QFileInfo fileInfo(filename);
		profileEdit->setToolTip(QString("Profile loaded successfully: %1").arg(fileInfo.fileName()));
		scopy::StatusBarManager::pushMessage(QString("Profile loaded: %1").arg(fileInfo.fileName()), 3000);
		qInfo(CAT_PROFILEMANAGER) << "Profile loaded successfully:" << filename;
	} else {
		QString errorMsg = QString("Failed to load profile from: %1").arg(filename);
		Q_EMIT profileError(errorMsg);
		qWarning(CAT_PROFILEMANAGER) << errorMsg;
		scopy::StatusBarManager::pushMessage(errorMsg, 5000);
		profileEdit->setToolTip(errorMsg);
	}
}

QCoro::Task<void> ProfileManager::onStreamFileChanged()
{
	if(!m_streamFileBrowser || !m_streamFileBrowser->lineEdit()) {
		co_return;
	}

	QString filename = m_streamFileBrowser->lineEdit()->text();
	if(filename.isEmpty() || filename == "(None)") {
		co_return;
	}

	// Non-blocking load on the main thread with progress indication
	updateStreamStatus(ProgressBarState::BUSY);
	bool success = co_await loadStreamFromFile(filename);
	updateStreamStatus(success ? ProgressBarState::SUCCESS : ProgressBarState::ERROR);

	QLineEdit *streamEdit = m_streamFileBrowser->lineEdit();
	if(success) {
		m_currentStreamPath = filename;
		Q_EMIT streamLoaded(filename);
		updateStatus();

		QFileInfo fileInfo(filename);
		streamEdit->setToolTip(QString("Stream loaded successfully: %1").arg(fileInfo.fileName()));
		scopy::StatusBarManager::pushMessage(QString("Stream loaded: %1").arg(fileInfo.fileName()), 3000);
		qInfo(CAT_PROFILEMANAGER) << "Stream loaded successfully:" << filename;
	} else {
		QString errorMsg = QString("Failed to load stream from: %1").arg(filename);
		Q_EMIT streamError(errorMsg);
		qWarning(CAT_PROFILEMANAGER) << errorMsg;
		scopy::StatusBarManager::pushMessage(errorMsg, 5000);
		streamEdit->setToolTip(errorMsg);
	}
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

QCoro::Task<bool> ProfileManager::loadProfileFromFile(QString filename)
{
	if(!m_device) {
		qWarning(CAT_PROFILEMANAGER) << "No device available for profile loading";
		co_return false;
	}

	// Read file content
	QFile file(filename);
	if(!file.open(QIODevice::ReadOnly)) {
		qWarning(CAT_PROFILEMANAGER) << "Failed to open profile file:" << filename;
		co_return false;
	}

	QByteArray profileData = file.readAll();
	file.close();

	if(profileData.isEmpty()) {
		qWarning(CAT_PROFILEMANAGER) << "Profile file is empty:" << filename;
		co_return false;
	}

	// Write to device attribute
	bool success = co_await writeDeviceAttribute("profile_config", profileData);
	if(success) {
		qDebug(CAT_PROFILEMANAGER) << "Profile data written to device successfully";
	} else {
		qWarning(CAT_PROFILEMANAGER) << "Failed to write profile data to device";
	}

	co_return success;
}

QCoro::Task<bool> ProfileManager::loadStreamFromFile(QString filename)
{
	if(!m_device) {
		qWarning(CAT_PROFILEMANAGER) << "No device available for stream loading";
		co_return false;
	}

	// Read file content - ensure binary mode for .stream files
	QFile file(filename);
	if(!file.open(QIODevice::ReadOnly)) {
		qWarning(CAT_PROFILEMANAGER) << "Failed to open stream file:" << filename;
		co_return false;
	}

	QByteArray streamData = file.readAll();
	file.close();

	if(streamData.isEmpty()) {
		qWarning(CAT_PROFILEMANAGER) << "Stream file is empty:" << filename;
		co_return false;
	}

	// Write to device attribute
	bool success = co_await writeDeviceAttribute("stream_config", streamData);
	if(success) {
		qDebug(CAT_PROFILEMANAGER) << "Stream data written to device successfully";
	} else {
		qWarning(CAT_PROFILEMANAGER) << "Failed to write stream data to device";
	}

	co_return success;
}

QString ProfileManager::readDeviceAttribute(const QString &attributeName, size_t bytes)
{
	if(!m_device) {
		return QString();
	}

	return componentRead(attributeName, bytes);
}

QCoro::Task<bool> ProfileManager::writeDeviceAttribute(QString attributeName, QByteArray data)
{
	if(!m_device) {
		qWarning(CAT_PROFILEMANAGER) << "No device available for writing attribute:" << attributeName;
		co_return false;
	}

	bool success = co_await componentWrite(attributeName, data);
	if(success) {
		qDebug(CAT_PROFILEMANAGER)
			<< "Successfully wrote" << data.size() << "bytes to attribute:" << attributeName;
	} else {
		qWarning(CAT_PROFILEMANAGER) << "Failed to write attribute:" << attributeName;
	}
	co_return success;
}

QString ProfileManager::componentRead(const QString &attributeName, size_t bytes)
{
	component::Attribute *a = component::attributeByName(m_device, attributeName);
	if(a && a->readCapability()) {
		auto res = QCoro::waitFor(a->readCapability()->readAsync(bytes));
		if(res) {
			return a->cachedValue().trimmed();
		}
	}

	qDebug(CAT_PROFILEMANAGER) << "Failed to read device attribute:" << attributeName;
	return QString();
}

QCoro::Task<bool> ProfileManager::componentWrite(QString attributeName, QByteArray data)
{
	component::Attribute *a = component::attributeByName(m_device, attributeName);
	if(!a || !a->writeCapability())
		co_return false;

	auto res = co_await a->writeCapability()->writeAsync(QString::fromUtf8(data));
	co_return(bool) res;
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
