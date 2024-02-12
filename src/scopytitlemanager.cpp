#include "scopytitlemanager.h"

#include <QApplication>
#include <QDebug>

#define TITLE_SEPARATOR " - "

using namespace adiscope;

ScopyTitleManager *ScopyTitleManager::pinstance_{nullptr};

ScopyTitleManager::ScopyTitleManager(QObject *parent) : QObject(parent) {
	qDebug() << "ctor";
}

ScopyTitleManager::~ScopyTitleManager() { qDebug() << "dtor"; }

void ScopyTitleManager::buildTitle()
{
	QString result;
	bool addSeparator = false;

	if (!m_title.isEmpty()) {
		result = m_title;
		addSeparator = true;
	}

	if (!m_version.isEmpty()) {
		if (addSeparator) {
			result += TITLE_SEPARATOR;
		}

		result += m_version;
		addSeparator = true;
	}

	if (!m_hash.isEmpty()) {
		if (addSeparator) {
			result += TITLE_SEPARATOR;
		}

		result += m_hash;
		addSeparator = true;
	}

	if (!m_filename.isEmpty()) {
		if (addSeparator) {
			result += TITLE_SEPARATOR;
		}

		result += m_filename;
		addSeparator = true;
	}

	m_currentTitle = result;
	auto instance = ScopyTitleManager::GetInstance();
	if (instance->m_mainWindow) {
		instance->m_mainWindow->setWindowTitle(instance->m_currentTitle);
	} else {
		qWarning() << "Cannot set title, no mainWidget was specified";
	}
}

ScopyTitleManager *ScopyTitleManager::GetInstance() {
	if (pinstance_ == nullptr) {
		pinstance_ = new ScopyTitleManager(
					QApplication::instance()); // singleton has the app as parent
	} else {
		qDebug() << "Singleton instance already created.";
	}
	return pinstance_;
}

void ScopyTitleManager::setApplicationName(QString title) {
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

void ScopyTitleManager::setGitHash(QString hash) {
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

QString ScopyTitleManager::getCurrentTitle()
{
	return ScopyTitleManager::GetInstance()->m_currentTitle;
}

void ScopyTitleManager::setMainWindow(QWidget *window)
{
	ScopyTitleManager::GetInstance()->m_mainWindow = window;
}

