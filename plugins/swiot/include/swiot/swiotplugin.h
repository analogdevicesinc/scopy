#ifndef SWIOTPLUGIN_H
#define SWIOTPLUGIN_H

#define SCOPY_PLUGIN_NAME SWIOTPlugin

#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>
#include "scopyswiot_export.h"
#include "QLabel"
#include "iioutil/cyclicaltask.h"
#include "iioutil/iiopingtask.h"
#include "src/swiotcontroller.h"


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
namespace swiot {
class SCOPYSWIOT_EXPORT SWIOTPlugin : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN;

public:
	void preload() override;
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

	QWidget *config ;
	QWidget *runtime;
        QWidget *faults;
        QWidget *maxtool;

//	IIOPingTask *ping;
//	CyclicalTask *cs;

	SwiotController *m_swiotController;

};
}
}

#endif // SWIOTPLUGIN_H
