#ifndef PLUGIN_H
#define PLUGIN_H

#include <QString>
#include <QWidget>
#include "toolmenuentry.h"
#include "scopypluginbase_export.h"

namespace adiscope {
class SCOPYPLUGINBASE_EXPORT Plugin : public QObject {
	Q_OBJECT
public:
	Plugin(QObject *parent = nullptr) : QObject(parent) { }
	virtual ~Plugin() {};

	virtual bool load(QString uri) = 0;
	virtual void unload() = 0;

	virtual bool compatible(QString uri) = 0;

	virtual QString uri();
	virtual QString name();
	virtual QWidget* icon();
	virtual QWidget* page();
	virtual QList<ToolMenuEntry*> toolList();
public Q_SLOTS:
	virtual bool connectDev() = 0;
	virtual bool disconnectDev() = 0;
	virtual void showPageCallback();
	virtual void hidePageCallback();
Q_SIGNALS:
	void restartDevice();
	void toolListChanged();
	void requestTool(QString);


protected:
	QString m_uri;
	QString m_name;
	QWidget *m_page;
	QWidget *m_icon;
	QList<ToolMenuEntry*> m_toolList;
};
}

#endif // PLUGIN_H
