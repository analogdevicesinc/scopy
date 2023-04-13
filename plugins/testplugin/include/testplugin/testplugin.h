#ifndef TESTPLUGIN_H
#define TESTPLUGIN_H

#define SCOPY_PLUGIN_NAME TestPlugin

#include <QObject>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>
#include "scopytestplugin_export.h"
#include <QLineEdit>

namespace scopy {

class SCOPYTESTPLUGIN_EXPORT TestPlugin : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN;

	// Plugin interface
public:
	friend class TestPlugin_API;
	void initPreferences() override;
	void initMetadata() override;
	void init() override;
	bool compatible(QString m_param) override;
	bool loadPreferencesPage() override;
	bool loadPage() override;
	bool loadIcon() override;
	void loadToolList() override;
	bool loadExtraButtons() override;
	QString about() override;
	QString version() override;
	bool onConnect() override;
	bool onDisconnect() override;
	void cloneExtra(Plugin*) override;
	void saveSettings(QSettings &) override;
	void loadSettings(QSettings &) override;

public Q_SLOTS:
	void messageCallback(QString topic, QString message) override;

private:
	QString m_initText;
	QWidget *tool;
	QLineEdit *edit;
	int renameCnt;
	ApiObject* m_pluginApi;
};

class SCOPYTESTPLUGIN_EXPORT TestPlugin_API : public ApiObject {
	Q_OBJECT
public:
	explicit TestPlugin_API(TestPlugin *p) : ApiObject(p),p(p) {}
	~TestPlugin_API() {}
	TestPlugin* p;

	Q_PROPERTY(QString testText READ testText WRITE setTestText);
	QString testText() const;
	void setTestText(const QString &newTestText);
private:
	QString m_testText;
};

}


#endif // TESTPLUGIN_H
