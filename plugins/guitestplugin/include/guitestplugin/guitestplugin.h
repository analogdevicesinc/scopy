#ifndef TESTPLUGIN_H
#define TESTPLUGIN_H

#define SCOPY_PLUGIN_NAME TestPlugin

#include "scopy-guitestplugin_export.h"

#include <QLineEdit>
#include <QObject>

#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>

namespace scopy {

class SCOPY_GUITESTPLUGIN_EXPORT TestPlugin : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN;

	// Plugin interface
public:
	friend class TestPlugin_API;
	void initMetadata() override;
	bool compatible(QString m_param, QString category) override;
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

} // namespace scopy

#endif // TESTPLUGIN_H
