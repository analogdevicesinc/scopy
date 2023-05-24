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

public Q_SLOTS:
	bool onConnect() override;
	bool onDisconnect() override;

private:
	void cleanAfterLastContext();
	void setupToolList();

	SwiotInfoPage* m_infoPage;
//	Ui::SWIOTInfoPage *infoui;

	QWidget *config;
	QWidget *adtool;
    QWidget *faults;
    QWidget *maxtool;

	SwiotController *m_swiotController;
	SwiotRuntime *m_runtime;
	QVector<QString> m_chnlsFunction{"current_out", "high_z", "high_z", "high_z",
					 "diagnostic", "diagnostic", "diagnostic", "diagnostic"};

	//	//Runtime A
//	const QVector<QString> channel_function{"digital_input", "voltage_out", "current_out", "voltage_in",
//							"diagnostic", "diagnostic", "diagnostic", "diagnostic"};
	//	//Runtime B
//		const QVector<QString> channel_function{"current_in_ext", "current_in_loop", "resistance", "digital_input",
//		"diagnostic", "diagnostic", "diagnostic", "diagnostic"};
	//	//Runtime C
//		const QVector<QString> channel_function{"digital_input_loop", "current_in_ext_hart", "current_in_ext_hart", "high_z",
//		"diagnostic", "diagnostic", "diagnostic", "diagnostic"};
	//	//Runtime D
		const QVector<QString> channel_function{"current_out", "high_z", "high_z", "high_z",
		"diagnostic", "diagnostic", "diagnostic", "diagnostic"};

};
}

#endif // SWIOTPLUGIN_H
