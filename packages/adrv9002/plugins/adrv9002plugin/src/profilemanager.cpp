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
{
	// Create main layout following FastlockProfilesWidget pattern
	QVBoxLayout *layout = new QVBoxLayout(this);

	// Title
	m_title = new QLabel("Profile & Stream Configuration", this);
	layout->addWidget(m_title);

	// Get embedded resources directory
	QFileInfoList profiles = scopy::PkgManager::listFilesInfo(QStringList() << "adrv9002-profiles");
	QString defaultDir =
		(profiles.size() > 0) ? profiles.first().absolutePath() : scopy::PkgManager::packagesPath();

	// Profile section
	m_profileLabel = new QLabel("Load Profile:", this);
	layout->addWidget(m_profileLabel);

	m_profileFileBrowser = new scopy::FileBrowserWidget(scopy::FileBrowserWidget::OPEN_FILE, this);
	m_profileFileBrowser->setFilter("Profile files (*.json)");
	m_profileFileBrowser->setBaseDirectory(defaultDir);
	m_profileFileBrowser->lineEdit()->setReadOnly(true);
	m_profileFileBrowser->lineEdit()->setPlaceholderText("(None)");
	layout->addWidget(m_profileFileBrowser);

	// Stream section
	m_streamLabel = new QLabel("Load Stream:", this);
	layout->addWidget(m_streamLabel);

	m_streamFileBrowser = new scopy::FileBrowserWidget(scopy::FileBrowserWidget::OPEN_FILE, this);
	m_streamFileBrowser->setFilter("Stream files (*.stream)");
	m_streamFileBrowser->setBaseDirectory(defaultDir);
	m_streamFileBrowser->lineEdit()->setReadOnly(true);
	m_streamFileBrowser->lineEdit()->setPlaceholderText("(None)");
	layout->addWidget(m_streamFileBrowser);

	// Connect FileBrowserWidget signals
	connect(m_profileFileBrowser->lineEdit(), &QLineEdit::textChanged, this, &ProfileManager::onProfileFileChanged);
	connect(m_streamFileBrowser->lineEdit(), &QLineEdit::textChanged, this, &ProfileManager::onStreamFileChanged);

	// Initial status update
	updateStatus();
}

ProfileManager::~ProfileManager() {}

QString ProfileManager::title() const { return m_title->text(); }

void ProfileManager::setTitle(const QString &newTitle) { m_title->setText(newTitle); }

void ProfileManager::refreshStatus() { updateStatus(); }

void ProfileManager::onProfileFileChanged()
{
	if(!m_profileFileBrowser)
		return;

	QString filename = m_profileFileBrowser->lineEdit()->text();
	if(filename.isEmpty() || filename == "(None)") {
		return;
	}

	qDebug(CAT_PROFILEMANAGER) << "Loading profile from:" << filename;

	if(loadProfileFromFile(filename)) {
		m_currentProfilePath = filename;
		updateStatus();
		Q_EMIT profileLoaded(filename);
		qInfo(CAT_PROFILEMANAGER) << "Profile loaded successfully:" << filename;
	} else {
		QString errorMsg = QString("Failed to load profile from: %1").arg(filename);
		Q_EMIT profileError(errorMsg);
		qWarning(CAT_PROFILEMANAGER) << errorMsg;
		scopy::StatusBarManager::pushMessage(errorMsg, 5000);
	}
}

void ProfileManager::onStreamFileChanged()
{
	if(!m_streamFileBrowser)
		return;

	QString filename = m_streamFileBrowser->lineEdit()->text();
	if(filename.isEmpty() || filename == "(None)") {
		return;
	}

	qDebug(CAT_PROFILEMANAGER) << "Loading stream from:" << filename;

	if(loadStreamFromFile(filename)) {
		m_currentStreamPath = filename;
		updateStatus();
		Q_EMIT streamLoaded(filename);
		qInfo(CAT_PROFILEMANAGER) << "Stream loaded successfully:" << filename;
	} else {
		QString errorMsg = QString("Failed to load stream from: %1").arg(filename);
		Q_EMIT streamError(errorMsg);
		qWarning(CAT_PROFILEMANAGER) << errorMsg;
		scopy::StatusBarManager::pushMessage(errorMsg, 5000);
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
				profileEdit->setStyleSheet("color: gray;");
			}
		} else {
			// Show current file path if available, otherwise show status
			if(!m_currentProfilePath.isEmpty()) {
				QFileInfo fileInfo(m_currentProfilePath);
				// Don't change text if user selected a file, just update styling
				if(profileEdit->text().isEmpty()) {
					profileEdit->setText(fileInfo.fileName());
				}
				profileEdit->setStyleSheet("color: black;");
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
				streamEdit->setStyleSheet("color: gray;");
			}
		} else {
			// Show current file path if available, otherwise show status
			if(!m_currentStreamPath.isEmpty()) {
				QFileInfo fileInfo(m_currentStreamPath);
				// Don't change text if user selected a file, just update styling
				if(streamEdit->text().isEmpty()) {
					streamEdit->setText(fileInfo.fileName());
				}
				streamEdit->setStyleSheet("color: black;");
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

	// Read file content
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

	int ret =
		iio_device_attr_write_raw(m_device, attributeName.toLocal8Bit().data(), data.constData(), data.size());

	if(ret == data.size()) {
		qDebug(CAT_PROFILEMANAGER)
			<< "Successfully wrote" << data.size() << "bytes to attribute:" << attributeName;
		return true;
	} else {
		qWarning(CAT_PROFILEMANAGER) << "Failed to write attribute:" << attributeName
					     << "expected:" << data.size() << "written:" << ret;
		return false;
	}
}
