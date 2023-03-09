#ifndef DEVICEICONBUILDER_H
#define DEVICEICONBUILDER_H

#include <QObject>
#include <QAbstractButton>
#include <deviceicon.h>

namespace adiscope {
class DeviceIconBuilder : public QObject
{
	Q_OBJECT
public:
	explicit DeviceIconBuilder(QObject *parent = nullptr);
	~DeviceIconBuilder();
	DeviceIcon* build(QString name, QString id, QWidget *icon, QWidget *parent);

signals:

};
}
#endif // DEVICEICONBUILDER_H
