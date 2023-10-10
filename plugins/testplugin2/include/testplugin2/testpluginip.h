#ifndef TESTPLUGINIP_H
#define TESTPLUGINIP_H

#define SCOPY_PLUGIN_NAME TestPluginIp

#include "pluginbase/pluginbase.h"
#include "scopy-testplugin2_export.h"

#include <QObject>

#include <pluginbase/plugin.h>

namespace scopy {
class SCOPY_TESTPLUGIN2_EXPORT TestPluginIp : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN

	// Plugin interface
public:
	void postload() override;
	bool loadIcon() override;
	bool loadPage() override;
	void loadToolList() override;
	bool compatible(QString m_param, QString category) override;
	void unload() override;
	bool onConnect() override;
	bool onDisconnect() override;
	void initMetadata() override;
	void saveSettings(QSettings &) override;
	void loadSettings(QSettings &) override;

private:
	QWidget *m_tool;
};
} // namespace scopy
#endif // TESTPLUGINIP_H
