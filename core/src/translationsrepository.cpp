#include "translationsrepository.h"

#include "common/scopyconfig.h"

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
	QStringList languages = directory.entryList(QStringList() << "*.qm", QDir::Files).replaceInStrings(".qm", "");
	for(const QString &lang : languages) {
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
