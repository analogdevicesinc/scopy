#ifndef PLUGIN_H
#define PLUGIN_H

#include <QString>
#include <QWidget>
#include "toolmenuentry.h"
#include "scopypluginbase_export.h"
#include <QtPlugin>
#include <QSettings>
#include <QAbstractButton>
#include "apiobject.h"

namespace adiscope {
class SCOPYPLUGINBASE_EXPORT Plugin {
public:
	virtual ~Plugin() {}

	virtual void setUri(QString) = 0;
	virtual void preload() = 0;
	virtual void initPreferences() = 0;
	virtual void postload() = 0;
	virtual void loadApi() = 0;
	virtual bool loadIcon() = 0;
	virtual bool loadPage() = 0;
	virtual void loadToolList() = 0;
	virtual bool loadPreferencesPage() = 0;
	virtual bool loadExtraButtons() = 0;
	virtual void initMetadata() = 0;
	virtual void setMetadata(QJsonObject obj) = 0;

	virtual void saveSettings() = 0;
	virtual void loadSettings() = 0;
	virtual void saveSettings(QSettings&) = 0;
	virtual void loadSettings(QSettings&) = 0;
	virtual ApiObject* api() = 0;

	virtual void unload() = 0;
	virtual bool compatible(QString uri) = 0;

	virtual Plugin* clone() = 0;
	virtual QString uri() = 0;
	virtual QString name() = 0;
	virtual QWidget* icon() = 0;
	virtual QWidget* page() = 0;
	virtual QWidget* preferencesPage() = 0;
	virtual QList<QAbstractButton*> extraButtons() = 0;
	virtual QList<adiscope::ToolMenuEntry*> toolList() = 0;
	virtual QJsonObject metadata() = 0;
	virtual QString about() = 0;
	virtual QString version() = 0;

public Q_SLOTS:
	virtual bool onConnect() = 0;
	virtual bool onDisconnect() = 0;
	virtual void showPageCallback() = 0;
	virtual void hidePageCallback() = 0;
	virtual void messageCallback(QString topic, QString message) = 0;
Q_SIGNALS:
	virtual void disconnectDevice() = 0;
	virtual void restartDevice() = 0;
	virtual void toolListChanged() = 0;
	virtual void requestTool(QString) = 0;
};
}

#define ScopyPlugin_iid "org.adi.Scopy.Plugins.Pluginbase/0.1"
Q_DECLARE_INTERFACE(adiscope::Plugin, ScopyPlugin_iid)

#endif // PLUGIN_H
