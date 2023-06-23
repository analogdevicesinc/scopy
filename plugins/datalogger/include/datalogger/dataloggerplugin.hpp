#ifndef DATALOGGERPLUGIN_H
#define DATALOGGERPLUGIN_H

#include "src/datalogger_api.h"
#define SCOPY_PLUGIN_NAME DataLoggerPlugin

#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>
#include "scopy-datalogger_export.h"
#include <iioutil/contextprovider.h>
#include "iioutil/cyclicaltask.h"
#include "iioutil/iiopingtask.h"

#include <QPushButton>
#include <QMap>
#include <QScrollArea>


Q_DECLARE_LOGGING_CATEGORY(CAT_DATALOGGER);
Q_DECLARE_LOGGING_CATEGORY(CAT_DATALOGGER_TOOL);


namespace Ui {
class DataLoggerInfoPage;
}

namespace libm2k::context {
class Context;
}

namespace scopy {
namespace datalogger {
class SCOPY_DATALOGGER_EXPORT DataLoggerPlugin : public QObject, public PluginBase
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
	DataLogger *tool;
	DataLogger_API *api;
	IIOPingTask *ping;
	CyclicalTask *cs;

	libm2k::context::Context *libm2k_context;
};
}
}

#endif // DATALOGGERPLUGIN_H
