#ifndef DEVICEIMPL_H
#define DEVICEIMPL_H

#include <QObject>
#include <device.h>

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
	QStringList toolList() override;
private:
	QString m_uri;
	QWidget *m_icon;
	QWidget *m_page;
};

#endif // DEVICEIMPL_H
