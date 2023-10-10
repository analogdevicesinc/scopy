#include "core/translationsrepository.h"

#include <QLibrary>
#include <QList>
#include <QTest>

using namespace scopy;

class TST_TranslationsRepository : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void checkPath();
	void checkGeneratedFiles();
	void loadTranslations();

private:
	QStringList libs;
};

void TST_TranslationsRepository::checkPath()
{
	QDir pathDir(TranslationsRepository::GetInstance()->getTranslationsPath());

	QVERIFY(pathDir.exists());
	QVERIFY(pathDir.entryList().contains("test.qm"));
	QVERIFY(pathDir.entryList().contains("test_test.qm"));
	qDebug() << "Found files:" << pathDir.entryList();
}

void TST_TranslationsRepository::checkGeneratedFiles()
{
	QStringList languages = TranslationsRepository::GetInstance()->getLanguages();
	qDebug() << "Found languages:" << languages;

	QVERIFY(languages.contains("test"));
}

void TST_TranslationsRepository::loadTranslations()
{
	QVERIFY(tr("TEST1") == "TEST1");
	QVERIFY(tr("TEST2") == "TEST2");

	TranslationsRepository::GetInstance()->loadTranslations("test");

	// verifying if test.qm was applied
	QVERIFY(tr("TEST1") == "test1");

	// verifying if test_test.qm was applied
	QVERIFY(tr("TEST2") == "test2");
}

QTEST_MAIN(TST_TranslationsRepository)

#include "tst_translationsrepository.moc"
