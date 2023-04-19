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
	void initMetadata() override;
	bool compatible(QString m_param) override;
	bool loadPage() override;
	bool loadIcon() override;
	void loadToolList() override;
	bool onConnect() override;
	bool onDisconnect() override;

private:
	QString m_initText;
	QWidget *tool;
	QLineEdit *edit;
};



}


#endif // TESTPLUGIN_H
