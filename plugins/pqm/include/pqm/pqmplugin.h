#ifndef PQMPLUGIN_H
#define PQMPLUGIN_H

#define SCOPY_PLUGIN_NAME PQMPlugin

#include "scopy-pqm_export.h"

#include <QObject>
#include <acquisitionmanager.h>
#include <gui/infopage.h>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>

namespace scopy::pqm {
class SCOPY_PQM_EXPORT PQMPlugin : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN;

public:
	void preload() override;
	bool compatible(QString m_param, QString category) override;
	bool loadPage() override;
	bool loadIcon() override;
	void loadToolList() override;
	void unload() override;
	void initMetadata() override;
	QString description() override;

public Q_SLOTS:
	bool onConnect() override;
	bool onDisconnect() override;
	void startPingTask() override;
	void stopPingTask() override;
	void onPausePingTask(bool pause) override;

private:
	void clearPingTask();

	InfoPage *m_infoPage;
	AcquisitionManager *m_acqManager;
};
} // namespace scopy::pqm

#endif // PQMPLUGIN_H
