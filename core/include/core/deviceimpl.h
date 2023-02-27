#ifndef DEVICEIMPL_H
#define DEVICEIMPL_H

#include "pluginbase/plugin.h"
#include <QObject>
#include <device.h>
#include <QPushButton>
#include <QCheckBox>
#include "scopycore_export.h"
#include <pluginmanager.h>

namespace adiscope {


class SCOPYCORE_EXPORT DeviceImpl : public QObject, public Device
{
	Q_OBJECT
public:
	explicit DeviceImpl(QString uri, PluginManager *p, QObject *parent = nullptr);
	virtual ~DeviceImpl();

	// Device interface
public:
	QString name() override;
	QString uri() override;
	QWidget *icon() override;
	QWidget *page() override;
	QList<ToolMenuEntry*> toolList() override;
	virtual void loadCompatiblePlugins() override;
	virtual void loadPlugins() override;
	virtual void unloadPlugins() override;
	void getPlugins();


public Q_SLOTS:
	virtual void connectDev() override;
	virtual void disconnectDev() override;
	virtual void showPage() override;
	virtual void hidePage() override;
	virtual void save(QSettings &) override;
	virtual void load(QSettings &) override;
//	void forgetDev() override;
Q_SIGNALS:
	void toolListChanged() override;
	void connected() override;
	void disconnected() override;
	void requestedRestart() override;
	void requestTool(QString) override;
protected:
	void loadName();
	void loadIcons();
	void loadPages();
	void loadToolList();

protected:
	PluginManager *p;
	QList<Plugin*> plugins;
	QString m_uri;
	QString m_name;
	QWidget *m_icon;
	QWidget *m_page;
	QPushButton *connbtn;
	QPushButton *discbtn;
	QCheckBox *extraToolchkbox;

};
}

#endif // DEVICEIMPL_H
