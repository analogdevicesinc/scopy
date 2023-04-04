#ifndef SWIOTPLUGIN_H
#define SWIOTPLUGIN_H

#define SCOPY_PLUGIN_NAME SWIOTPlugin

#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>
#include "scopyswiot_export.h"
#include "QLabel"
#include "iioutil/cyclicaltask.h"
#include "iioutil/iiopingtask.h"

#include "ui_swiotInfoPage.h"
#include "ui_swiotconfig.h"
#include "ui_swiotruntime.h"
#include "ui_swiotmax14906.h"
#include "ui_swiotfaults.h"

namespace Ui {
class SWIOTInfoPage;
class SwiotRuntimeUI;
class SwiotFaults;
class Max14906Tool; // TODO: rename this class
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
	Ui::ConfigMenu *configui;
	Ui::SwiotRuntimeUI *rungui;
        Ui::SwiotFaults *faultsgui;
        Ui::Max14906Tool *maxgui;

	QWidget *config ;
	QWidget *runtime;
        QWidget *faults;
        QWidget *maxtool;

	IIOPingTask *ping;
	CyclicalTask *cs;

};
}

#endif // SWIOTPLUGIN_H
