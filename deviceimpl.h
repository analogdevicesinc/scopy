#ifndef DEVICEIMPL_H
#define DEVICEIMPL_H

#include <QObject>
#include <device.h>
#include <QPushButton>
#include <QCheckBox>

namespace adiscope {

class DeviceImpl : public QObject, public Device
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
	QList<ToolMenuEntry> toolList() override;


public Q_SLOTS:
	void connectDev() override;
	void disconnectDev() override;
//	void forgetDev() override;
Q_SIGNALS:
	void toolListChanged() override;
	void connected() override;
	void disconnected() override;
private:
	QString m_uri;
	QWidget *m_icon;
	QWidget *m_page;
	QPushButton *connbtn;
	QPushButton *discbtn;
	QCheckBox *extraToolchkbox;
};
}

#endif // DEVICEIMPL_H
