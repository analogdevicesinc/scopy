#ifndef SWIOTPLUGIN_H
#define SWIOTPLUGIN_H
#define SCOPY_PLUGIN_NAME SWIOTPlugin
#define AD74413R_TME_ID "swiotad74413r"
#define CONFIG_TME_ID "swiotconfig"
#define FAULTS_TME_ID "swiotfaults"
#define MAX14906_TME_ID "swiotmax14906"

#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>
#include "scopy-swiot_export.h"
#include "src/runtime/swiotruntime.h"
#include "QLabel"
#include "src/swiotcontroller.h"
#include "src/swiotinfopage.h"
#include <iioutil/cyclicaltask.h>

namespace scopy::swiot {
class SCOPY_SWIOT_EXPORT SWIOTPlugin : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN;

public:
	void preload() override;
	bool loadPage() override;
	bool loadIcon() override;
	void loadToolList() override;
	void unload() override;
	bool compatible(QString param, QString category) override;
	void initMetadata() override;
	QString description() override;

public Q_SLOTS:
	bool onConnect() override;
	bool onDisconnect() override;
	void startCtxSwitch();
	void onCtxSwitched();

private:
	void setupToolList();

	SwiotInfoPage *m_infoPage;
//	Ui::SWIOTInfoPage *infoui;

	QWidget *config;
	QWidget *adtool;
	QWidget *faults;
	QWidget *maxtool;

	SwiotController *m_swiotController;
	SwiotRuntime *m_runtime;

	bool m_isRuntime;
};
}

#endif // SWIOTPLUGIN_H
