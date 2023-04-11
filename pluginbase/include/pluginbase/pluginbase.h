#ifndef PLUGINBASE_H
#define PLUGINBASE_H
#include "plugin.h"
#include "scopypluginbase_export.h"
#include <QObject>
#include "apiobject.h"


namespace adiscope {
/**
 * @brief The PluginBase class
 * PluginBase is used to provide default implementation to the plugin interface
 */
class SCOPYPLUGINBASE_EXPORT PluginBase : public Plugin {

public:
	virtual ~PluginBase() {}

	virtual void setParam(QString param) override;
	virtual void initMetadata() override;
	virtual void setMetadata(QJsonObject obj) override;
	virtual void initPreferences() override;
	virtual void init() override;
	virtual void deinit() override;
	virtual void preload() override;
	virtual void postload() override;

	virtual void loadApi() override;
	virtual bool loadIcon() override;
	virtual bool loadPage() override;
	virtual void loadToolList() override;
	virtual bool loadPreferencesPage() override;
	virtual bool loadExtraButtons() override;

	virtual void saveSettings() override;
	virtual void loadSettings() override;
	virtual void saveSettings(QSettings&) override;
	virtual void loadSettings(QSettings&) override;

	virtual void unload() override;

	virtual ApiObject* api() override;
	virtual QString param() override;
	virtual QString name() override;
	virtual QString displayName() override;
	virtual QString displayDescription() override;
	virtual QWidget* icon() override;
	virtual QWidget* page() override;
	virtual QWidget* preferencesPage() override;
	virtual QList<QAbstractButton*> extraButtons() override;
	virtual QList<ToolMenuEntry*> toolList() override;
	virtual QJsonObject metadata() override;
	virtual QString about() override;
	virtual QString version() override;
	virtual void loadMetadata(QString data);
	virtual void cloneExtra(Plugin*) override;

public Q_SLOTS:
	virtual void showPageCallback() override;
	virtual void hidePageCallback() override;
	virtual void messageCallback(QString topic, QString message) override;
	virtual void requestTool(QString);

protected:
	QString m_param;
	QString m_name;
	QString m_displayName;
	QString m_displayDescription;
	QWidget *m_page;
	QWidget *m_preferencesPage;
	QWidget *m_icon;
	QList<ToolMenuEntry*> m_toolList;
	QList<QAbstractButton*> m_extraButtons;
	QJsonObject m_metadata;
	ApiObject* m_pluginApi = nullptr;
};
}

#define scopyxstr(a) scopystr(a)
#define scopystr(a) #a

#define SCOPY_PLUGIN \
	Q_PLUGIN_METADATA(IID ScopyPlugin_iid)\
	Q_INTERFACES(adiscope::Plugin)\
public:\
	virtual ~SCOPY_PLUGIN_NAME () override {}\
	SCOPY_PLUGIN_NAME* clone(QObject *parent) override { \
		SCOPY_PLUGIN_NAME* ret = new SCOPY_PLUGIN_NAME(); \
		/* copy metadata from this object to the next one */\
		ret->m_name = scopyxstr(SCOPY_PLUGIN_NAME);\
		ret->setParent(parent);\
		ret->m_displayName = ret->m_name;\
		ret->setMetadata(metadata()); \
		ret->cloneExtra(this);\
		return ret;\
	}\
\
Q_SIGNALS:\
	void disconnectDevice() override;\
	void restartDevice() override;\
	void toolListChanged() override;\
	void requestToolByUuid(QString) override;\
private:\

#define SCOPY_PLUGIN_ICON(x) m_icon = new QLabel(""); m_icon->setStyleSheet("border-image: url(" x ");")
#define SCOPY_NEW_TOOLMENUENTRY(id, name, icon) new ToolMenuEntry(id,name,icon,this->m_name,this)


#endif // PLUGINBASE_H
