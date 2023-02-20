#ifndef DEVICEIMPL_H
#define DEVICEIMPL_H

#include "plugin.h"
#include <QObject>
#include <device.h>
#include <QPushButton>
#include <QCheckBox>
#include <scopycore_export.h>

namespace adiscope {

class SCOPYCORE_EXPORT DeviceImpl : public QObject, public Device
{
	Q_OBJECT
public:
	explicit DeviceImpl(QString uri,QObject *parent = nullptr);
	~DeviceImpl();

	// Device interface
public:
	QString name() override;
	QString uri() override;
	QWidget *icon() override;
	QWidget *page() override;
	QList<ToolMenuEntry*> toolList() override;
	void loadPlugins() override;
	void unloadPlugins() override;
	void getPlugins();


public Q_SLOTS:
	void connectDev() override;
	void disconnectDev() override;
	void showPage() override;
	void hidePage() override;
//	void forgetDev() override;
Q_SIGNALS:
	void toolListChanged() override;
	void connected() override;
	void disconnected() override;
	void requestedRestart() override;
	void requestTool(QString) override;
private:
	void loadName();
	void loadIcons();
	void loadPages();

private:
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
