#ifndef DEVICEIMPL_H
#define DEVICEIMPL_H

#include "pluginbase/plugin.h"
#include <QObject>
#include "device.h"
#include <QPushButton>
#include <QCheckBox>
#include "scopy-core_export.h"
#include "pluginmanager.h"

namespace scopy {


class SCOPY_CORE_EXPORT DeviceImpl : public QObject, public Device
{
	Q_OBJECT
public:
	explicit DeviceImpl(QString param, PluginManager *p, QString category = "",QObject *parent = nullptr);
	virtual ~DeviceImpl();

	// Device interface
public:
	QString id() override;
	QString displayName() override;
	QString displayParam() override;
	QString category() override;
	QString param() override;
	QWidget *icon() override;
	QWidget *page() override;
	QList<ToolMenuEntry*> toolList() override;
	virtual void init() override;
	virtual void preload() override;
	virtual void loadPlugins() override;
	virtual void unloadPlugins() override;

	QList<Plugin *> plugins() const;


public Q_SLOTS:
	virtual void connectDev() override;
	virtual void disconnectDev() override;
	virtual void showPage() override;
	virtual void hidePage() override;
	virtual void save(QSettings &) override;
	virtual void load(QSettings &) override;
//	void forgetDev() override;
	void onConnectionFailed();
Q_SIGNALS:
	void toolListChanged() override;
	void connected() override;
	void disconnected() override;
	void requestedRestart() override;
	void requestTool(QString) override;
	void connectionFailed();
protected:
	void removeDisabledPlugins();
	void loadName();
	void loadIcons();
	void loadPages();
	void loadToolList();

protected:
	PluginManager *p;
	QList<Plugin*> m_plugins;
	QList<Plugin*> m_connectedPlugins;
	QString m_id;
	QString m_category;
	QString m_displayName;
	QString m_displayParam;
	QString m_param;
	QWidget *m_icon;
	QWidget *m_page;
	QPushButton *connbtn, *discbtn;

private:
	void loadWarningBadge();
	bool eventFilter(QObject *obj, QEvent *event) override;
	QPushButton *m_warningBtn;

};
}

#endif // DEVICEIMPL_H
