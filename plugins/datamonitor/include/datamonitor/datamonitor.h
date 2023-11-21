#ifndef DATAMONITORPLUGIN_H
#define DATAMONITORPLUGIN_H

#define SCOPY_PLUGIN_NAME DataMonitorPlugin

#include "iioutil/cyclicaltask.h"
#include "iioutil/iiopingtask.h"
#include "scopy-datamonitor_export.h"

#include <QMap>
#include <QPushButton>
#include <QScrollArea>

#include <iioutil/contextprovider.h>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>

Q_DECLARE_LOGGING_CATEGORY(CAT_DATAMONITOR)
Q_DECLARE_LOGGING_CATEGORY(CAT_DATAMONITOR_TOOL)

namespace Ui {
class DataMonitorInfoPage;
}

namespace libm2k::context {
class Context;
}

namespace scopy {
namespace datamonitor {

class DataMonitorTool;

class SCOPY_DATAMONITOR_EXPORT DataMonitorPlugin : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN;

public:
	bool loadPage() override;
	bool loadIcon() override;
	void loadToolList() override;
	void unload() override;
	bool compatible(QString param, QString cateogory) override;
	void initMetadata() override;
	void saveSettings(QSettings &) override;
	void loadSettings(QSettings &) override;
	QString description() override;
public Q_SLOTS:
	bool onConnect() override;
	bool onDisconnect() override;

private:
	DataMonitorTool *tool;
	IIOPingTask *ping;
	CyclicalTask *cs;

	libm2k::context::Context *libm2k_context;
};
} // namespace datamonitor
} // namespace scopy

#endif // DATAMONITORPLUGIN_H
