#ifndef TESTPLUGINIP_H
#define TESTPLUGINIP_H


#define SCOPY_PLUGIN_NAME TestPluginIp
#define SCOPY_PLUGIN_PRIO 2

#include <QObject>
#include <pluginbase/plugin.h>
#include "pluginbase/pluginbase.h"
#include "scopytestplugin2_export.h"

namespace adiscope {
class SCOPYTESTPLUGIN2_EXPORT TestPluginIp : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN

public:
	void postload() override;
	bool loadIcon() override;
	bool loadPage() override;
	void loadToolList() override;
	bool compatible(QString uri) override;
	void unload() override;
	bool onConnect() override;
	bool onDisconnect() override;
	void initMetadata() override;
private:
	QWidget *m_tool;

};
}
#endif // TESTPLUGINIP_H
