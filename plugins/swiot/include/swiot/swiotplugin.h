#ifndef SWIOTPLUGIN_H
#define SWIOTPLUGIN_H

#define SCOPY_PLUGIN_NAME SWIOTPlugin

#include <pluginbase/plugin.h>
#include "pluginbase/pluginbase.h"
#include "scopyswiot_export.h"
#include "QLabel"
#include "iioutil/cyclicaltask.h"
#include "iioutil/iiopingtask.h"


namespace Ui {
class SWIOTInfoPage;
class SWIOTConfigTool;
class SWIOTRuntime;
}

namespace adiscope {
class SCOPYSWIOT_EXPORT SWIOTPlugin : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN;

public:
	bool loadPage() override;
	bool loadIcon() override;
	void loadToolList() override;
	void unload() override;
	bool compatible(QString param) override;
	void initMetadata() override;
public Q_SLOTS:
	bool onConnect() override;
	bool onDisconnect() override;

private:
	Ui::SWIOTInfoPage *infoui;
	Ui::SWIOTConfigTool *configui;
	Ui::SWIOTRuntime *rungui;
	QWidget *config ;
	QWidget *runtime;

	IIOPingTask *ping;
	CyclicalTask *cs;

};
}

#endif // SWIOTPLUGIN_H
