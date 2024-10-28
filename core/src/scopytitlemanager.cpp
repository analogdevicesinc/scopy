/*
 * Copyright (c) 2024 Analog Devices Inc.
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
 *
 */

#include "scopytitlemanager.h"

#include <QApplication>
#include <QDebug>
#include <QLoggingCategory>

#define TITLE_SEPARATOR " - "
Q_LOGGING_CATEGORY(CAT_SCOPYTITLEMANAGER, "ScopyTitleManager")

using namespace scopy;

ScopyTitleManager *ScopyTitleManager::pinstance_{nullptr};

ScopyTitleManager::ScopyTitleManager(QObject *parent)
	: QObject(parent)
{
	qDebug(CAT_SCOPYTITLEMANAGER) << "ctor";
}

ScopyTitleManager::~ScopyTitleManager() { qDebug(CAT_SCOPYTITLEMANAGER) << "dtor"; }

void ScopyTitleManager::buildTitle()
{
	QString result;
	bool addSeparator = false;

	if(!m_title.isEmpty()) {
		result = m_title;
		addSeparator = true;
	}

	if(!m_version.isEmpty()) {
		if(addSeparator) {
			result += TITLE_SEPARATOR;
		}

		result += m_version;
		addSeparator = true;
	}

	if(!m_hash.isEmpty()) {
		if(addSeparator) {
			result += TITLE_SEPARATOR;
		}

		result += m_hash;
		addSeparator = true;
	}

	if(!m_filename.isEmpty()) {
		if(addSeparator) {
			result += TITLE_SEPARATOR;
		}

		result += m_filename;
		addSeparator = true;
	}

	m_currentTitle = result;
	auto instance = ScopyTitleManager::GetInstance();
	if(instance->m_mainWindow) {
		instance->m_mainWindow->setWindowTitle(instance->m_currentTitle);
		qDebug(CAT_SCOPYTITLEMANAGER) << "Title was built: " << m_currentTitle;
	} else {
		qWarning(CAT_SCOPYTITLEMANAGER) << "Cannot set title, no mainWidget was specified";
	}
}

ScopyTitleManager *ScopyTitleManager::GetInstance()
{
	if(pinstance_ == nullptr) {
		pinstance_ = new ScopyTitleManager(QApplication::instance()); // singleton has the app as parent
	} else {
		qDebug(CAT_SCOPYTITLEMANAGER) << "Singleton instance already created.";
	}
	return pinstance_;
}

void ScopyTitleManager::setApplicationName(QString title)
{
	auto instance = ScopyTitleManager::GetInstance();
	instance->m_title = title;
	instance->buildTitle();
}

void ScopyTitleManager::clearApplicationName()
{
	auto instance = ScopyTitleManager::GetInstance();
	instance->m_title.clear();
	instance->buildTitle();
}

void ScopyTitleManager::setScopyVersion(QString version)
{
	auto instance = ScopyTitleManager::GetInstance();
	instance->m_version = version;
	instance->buildTitle();
}

void ScopyTitleManager::clearScopyVersion()
{
	auto instance = ScopyTitleManager::GetInstance();
	instance->m_version.clear();
	instance->buildTitle();
}

void ScopyTitleManager::setGitHash(QString hash)
{
	auto instance = ScopyTitleManager::GetInstance();
	instance->m_hash = hash;
	instance->buildTitle();
}

void ScopyTitleManager::clearGitHash()
{
	auto instance = ScopyTitleManager::GetInstance();
	instance->m_hash.clear();
	instance->buildTitle();
}

void ScopyTitleManager::setIniFileName(QString filename)
{
	auto instance = ScopyTitleManager::GetInstance();
	instance->m_filename = filename;
	instance->buildTitle();
}

void ScopyTitleManager::clearIniFileName()
{
	auto instance = ScopyTitleManager::GetInstance();
	instance->m_filename.clear();
	instance->buildTitle();
}

void ScopyTitleManager::clearAll()
{
	auto instance = ScopyTitleManager::GetInstance();
	instance->m_title.clear();
	instance->m_version.clear();
	instance->m_hash.clear();
	instance->m_filename.clear();
}

QString ScopyTitleManager::getCurrentTitle() { return ScopyTitleManager::GetInstance()->m_currentTitle; }

void ScopyTitleManager::setMainWindow(QWidget *window)
{
	ScopyTitleManager::GetInstance()->m_mainWindow = window;
	qDebug(CAT_SCOPYTITLEMANAGER) << "Main window was set.";
}

#include "moc_scopytitlemanager.cpp"
