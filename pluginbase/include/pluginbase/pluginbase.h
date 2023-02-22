#ifndef PLUGINBASE_H
#define PLUGINBASE_H
#include "plugin.h"
#include "scopypluginbase_export.h"
#include <QObject>


namespace adiscope {
class SCOPYPLUGINBASE_EXPORT PluginBase : public Plugin {

public:
	virtual ~PluginBase() {}

	virtual QString uri();
	virtual QString name();
	virtual QWidget* icon();
	virtual QWidget* page();
	virtual QList<ToolMenuEntry*> toolList();
	virtual int priority();

	virtual void showPageCallback();
	virtual void hidePageCallback();

protected:
	QString m_uri;
	QString m_name;
	QWidget *m_page;
	QWidget *m_icon;
	QList<ToolMenuEntry*> m_toolList;
};
}

#define xstr(a) str(a)
#define str(a) #a

#define SCOPY_PLUGIN \
	Q_PLUGIN_METADATA(IID ScopyPlugin_iid)\
	Q_INTERFACES(adiscope::Plugin)\
public:\
	virtual ~SCOPY_PLUGIN_NAME () override {}\
	SCOPY_PLUGIN_NAME* clone() override { return new SCOPY_PLUGIN_NAME();}\
	QString name() override { return xstr(SCOPY_PLUGIN_NAME) ;}\
	int priority() override { return SCOPY_PLUGIN_PRIO;}\
\
Q_SIGNALS:\
	virtual void restartDevice() override;\
	virtual void toolListChanged() override;\
	virtual void requestTool(QString) override;\
private:\

#define SCOPY_PLUGIN_ICON(x) m_icon = new QLabel(""); m_icon->setStyleSheet("border-image: url(" x ");");
#define SCOPY_NEW_TOOLMENUENTRY(x, y) new ToolMenuEntry(QUuid::createUuid().toString(),x,y,this)


#endif // PLUGINBASE_H
