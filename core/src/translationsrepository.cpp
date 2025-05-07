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

#include "translationsrepository.h"

#include "common/scopyconfig.h"
#include "pkgmanager.h"

#include <QApplication>
#include <QDir>
#include <QLoggingCategory>
#include <QTranslator>

#include <pluginbase/preferences.h>

Q_LOGGING_CATEGORY(CAT_TRANSLATIONREPOSITORY, "ScopyTranslations");

using namespace scopy;

TranslationsRepository *TranslationsRepository::pinstance_{nullptr};

TranslationsRepository::TranslationsRepository(QObject *parent)
	: QObject(parent)
{}

TranslationsRepository::~TranslationsRepository() {}

TranslationsRepository *TranslationsRepository::GetInstance()
{
	if(pinstance_ == nullptr) {
		pinstance_ = new TranslationsRepository(QApplication::instance()); // singleton has the app as parent
	}
	return pinstance_;
}

QString TranslationsRepository::getTranslationsPath()
{
	// Check the local plugins folder first
	QDir pathDir(config::localTranslationFolderPath());
	if(pathDir.exists()) {
		return config::localTranslationFolderPath();
	}

	return config::defaultTranslationFolderPath();
}

QStringList TranslationsRepository::getLanguages()
{
	QDir directory(TranslationsRepository::getTranslationsPath());
	QFileInfoList languagesFileInfo = PkgManager::listFilesInfo(
		QStringList() << scopy::config::pkgTranslationsDir(), QStringList() << "*.qm");
	QStringList languages;
	std::transform(languagesFileInfo.begin(), languagesFileInfo.end(), std::back_inserter(languages),
		       [](const QFileInfo &fi) { return fi.fileName(); });
	languages.append(directory.entryList(QStringList() << "*.qm", QDir::Files));
	languages.replaceInStrings(".qm", "");
	languages.sort();
	for(const QString &lang : qAsConst(languages)) {
		if(lang.contains("_"))
			languages.removeOne(lang);
	}

	// no languages found
	if(languages.empty()) {
		languages.append("default");
	}

	return languages;
}

void TranslationsRepository::loadTranslations(QString language)
{
	if(language == "default") {
		qDebug(CAT_TRANSLATIONREPOSITORY) << "No languages loaded (default)";
		return;
	}

	QList<QTranslator *> translatorList = QList<QTranslator *>();
	QDir directory(TranslationsRepository::getTranslationsPath());
	QFileInfoList languages = directory.entryInfoList(QStringList() << "*.qm", QDir::Files);
	languages.append(PkgManager::listFilesInfo(QStringList() << scopy::config::pkgTranslationsDir(),
						   QStringList() << "*.qm"));

	for(const QFileInfo &lang : languages) {
		if(lang.fileName().endsWith("_" + language + ".qm") || lang.fileName() == language + ".qm") {
			translatorList.append(new QTranslator());
			translatorList.last()->load(lang.filePath());
			QApplication::installTranslator(translatorList.last());

			qDebug(CAT_TRANSLATIONREPOSITORY) << "Loaded:" << lang.fileName();
		}
	}
}

#include "moc_translationsrepository.cpp"
