#ifndef DATALOGGERPLUGIN_H
#define DATALOGGERPLUGIN_H

#define SCOPY_PLUGIN_NAME DataLoggerPlugin

#include "dataacquisitionmanager.hpp"
#include "datamonitortool.h"
#include "datamonitor/readabledatamonitormodel.hpp"
#include "scopy-dataloggerplugin_export.h"
#include <QObject>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>

namespace scopy::datamonitor {

class SCOPY_DATALOGGERPLUGIN_EXPORT DataLoggerPlugin : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN;

public:
	bool compatible(QString m_param, QString category) override;
	bool loadPage() override;
	bool loadIcon() override;
	void loadToolList() override;
	void unload() override;
	void initMetadata() override;
	void initPreferences() override;
	bool loadPreferencesPage() override;
	QString description() override;

public Q_SLOTS:
	bool onConnect() override;
	bool onDisconnect() override;

	void addNewTool();
	void removeTool(QString toolId);
	void toggleRunState(bool toggled);

private:
	QList<ReadableDataMonitorModel *> dmmList;
	DataAcquisitionManager *m_dataAcquisitionManager;
	bool isRunning = false;
};
} // namespace scopy::datamonitor
#endif // DATALOGGERPLUGIN_H
