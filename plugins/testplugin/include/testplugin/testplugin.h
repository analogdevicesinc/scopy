#ifndef TESTPLUGIN_H
#define TESTPLUGIN_H

#define SCOPY_PLUGIN_NAME TestPlugin
#define SCOPY_PLUGIN_PRIO 1

#include <QObject>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>
#include "scopytestplugin_export.h"

namespace adiscope {
class SCOPYTESTPLUGIN_EXPORT TestPlugin : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN;
public:
	void initMetadata() override;
	bool compatible(QString uri) override;
	void postload() override;
	bool loadPage() override;
	bool loadIcon() override;
	void loadToolList() override;
	void unload() override;

	bool onConnect() override;
	bool onDisconnect() override;

	// Plugin interface
};
}

#endif // TESTPLUGIN_H
