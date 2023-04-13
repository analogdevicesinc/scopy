#include <QTest>
#include <QList>
#include "core/pluginrepository.h"
#include <QLibrary>

using namespace scopy;

class TST_PluginRepository : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void loadLibs();

private:
	QStringList libs;
};


#define NONPLUGIN_LIBRARY_LOCATION "../libscopycore.so"
#define PLUGIN_LOCATION "../../plugins/plugins"


void TST_PluginRepository::loadLibs()
{
	PluginRepository *p = new PluginRepository(this);
	PluginManager *pm = p->getPluginManager();
	QVERIFY(pm->metadata().isEmpty());
	p->init(PLUGIN_LOCATION);
//	QVERIFY(!pm->metadata().isEmpty());


	delete p;
}


QTEST_MAIN(TST_PluginRepository)

#include "tst_pluginrepository.moc"
