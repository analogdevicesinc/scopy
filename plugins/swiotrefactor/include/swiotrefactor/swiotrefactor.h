#ifndef SWIOTREFACTORPLUGIN_H
#define SWIOTREFACTORPLUGIN_H

#define SCOPY_PLUGIN_NAME SWIOTREFACTORPlugin

#define AD74413R_TME_ID "swiotad74413r"
#define CONFIG_TME_ID "swiotconfig"
#define FAULTS_TME_ID "swiotfaults"
#define MAX14906_TME_ID "swiotmax14906"

#include "scopy-swiotrefactor_export.h"
#include "swiotcontroller.h"
#include "swiotruntime.h"
#include "swiotinfopage.h"
#include <QObject>
#include <QPushButton>
#include <tutorialbuilder.h>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>

namespace scopy::swiotrefactor {
class SCOPY_SWIOTREFACTOR_EXPORT SWIOTREFACTORPlugin : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN;

public:
	void preload() override;
	bool loadPage() override;
	bool loadExtraButtons() override;
	bool loadIcon() override;
	void loadToolList() override;
	void unload() override;
	bool compatible(QString param, QString category) override;
	void initMetadata() override;
	QString description() override;

public Q_SLOTS:
	bool onConnect() override;
	bool onDisconnect() override;
	void onIsRuntimeCtxChanged(bool isRuntimeCtx);

	void startTutorial();
	void abortTutorial();

	void startAd74413rTutorial();
	void startMax14906Tutorial();
	void startFaultsTutorial();

	void powerSupplyStatus(bool ps);

private Q_SLOTS:
	void setCtxMode(QString mode);
	void onModeAttributeChanged(QString mode);

private:
	void switchCtx();
	void setupToolList();
	void createStatusContainer();
	QPushButton *m_btnIdentify;
	QPushButton *m_btnTutorial;
	QWidget *m_statusContainer;
	SwiotInfoPage *m_infoPage;

	QWidget *config;
	QWidget *adtool;
	QWidget *faults;
	QWidget *maxtool;

	SwiotController *m_swiotController;
	SwiotRuntime *m_runtime;

	gui::TutorialBuilder *m_ad74413rTutorial;
	gui::TutorialBuilder *m_max14906Tutorial;
	gui::TutorialBuilder *m_faultsTutorial;

	bool m_isRuntime;
	bool m_switchCmd = false;
	QString m_ctxMode;
};
} // namespace scopy::swiotrefactor
#endif // SWIOTREFACTORPLUGIN_H
