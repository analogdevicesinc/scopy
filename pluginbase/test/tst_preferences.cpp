#include <QTest>
#include <QList>
#include <pluginbase/preferences.h>
#include <QDir>

using namespace scopy;

class TST_Preferences : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void initPreferences();
	void getPreferences();

private:
	QSettings *s;
};




const QString filePath = QDir::currentPath()+"Pref.ini";

void TST_Preferences::initPreferences()
{
	Preferences *p = Preferences::GetInstance();
	p->setPreferencesFilename(filePath);
	p->init("test","true");
	QString t = p->get("test").toString();
	QVERIFY("true"==t);
	p->save();
	p->clear();
}

void TST_Preferences::getPreferences()
{
	Preferences *p = Preferences::GetInstance();
	p->setPreferencesFilename(filePath);
	p->load();
	QString t = p->get("test").toString();
	QVERIFY("true"==t);
}


QTEST_MAIN(TST_Preferences)

#include "tst_preferences.moc"
