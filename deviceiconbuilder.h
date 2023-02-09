#ifndef DEVICEICONBUILDER_H
#define DEVICEICONBUILDER_H

#include <QObject>
#include <QAbstractButton>

class DeviceIconBuilder : public QObject
{
	Q_OBJECT
public:
	explicit DeviceIconBuilder(QObject *parent = nullptr);
	~DeviceIconBuilder();
	QAbstractButton* build(QString name, QString id, QWidget *icon, QWidget *parent);

signals:

};

#endif // DEVICEICONBUILDER_H
