#ifndef PLUGINBASE_H
#define PLUGINBASE_H
#include "plugin.h"
#include "scopypluginbase_export.h"
#include <QObject>


namespace adiscope {
class SCOPYPLUGINBASE_EXPORT PluginBase : public Plugin {

public:
	virtual ~PluginBase() {}

	virtual void setUri(QString uri) override;
	virtual void initMetadata() override;
	virtual void setMetadata(QJsonObject obj) override;
	virtual void preload() override;
	virtual void postload() override;
	virtual bool loadIcon() override;
	virtual bool loadPage() override;
	virtual void loadToolList() override;

	virtual QString uri() override;
	virtual QString name() override;
	virtual QWidget* icon() override;
	virtual QWidget* page() override;
	virtual QList<ToolMenuEntry*> toolList() override;
	virtual QJsonObject metadata() override;
	virtual QString about() override;

	virtual void showPageCallback() override;
	virtual void hidePageCallback() override;


	virtual void loadMetadata(QString data);



protected:
	QString m_uri;
	QString m_name;
	QWidget *m_page;
	QWidget *m_icon;
	QList<ToolMenuEntry*> m_toolList;
	QJsonObject m_metadata;
};
}

#define xstr(a) str(a)
#define str(a) #a

#define SCOPY_PLUGIN \
	Q_PLUGIN_METADATA(IID ScopyPlugin_iid)\
	Q_INTERFACES(adiscope::Plugin)\
public:\
	virtual ~SCOPY_PLUGIN_NAME () override {qDebug()<<"dtor";}\
	SCOPY_PLUGIN_NAME* clone() override { \
		SCOPY_PLUGIN_NAME* ret = new SCOPY_PLUGIN_NAME(); \
		/* copy metadata from this object to the next one */\
		ret->m_name = xstr(SCOPY_PLUGIN_NAME);\
		ret->setMetadata(metadata()); \
		return ret;\
	} \
	QString name() override { return  m_name;}\
\
Q_SIGNALS:\
	virtual void disconnectDevice() override;\
	virtual void restartDevice() override;\
	virtual void toolListChanged() override;\
	virtual void requestTool(QString) override;\
private:\

#define SCOPY_PLUGIN_ICON(x) m_icon = new QLabel(""); m_icon->setStyleSheet("border-image: url(" x ");");
#define SCOPY_NEW_TOOLMENUENTRY(x, y) new ToolMenuEntry(QUuid::createUuid().toString(),x,y,this)


#endif // PLUGINBASE_H
