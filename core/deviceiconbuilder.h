#ifndef DEVICEICONBUILDER_H
#define DEVICEICONBUILDER_H

#include <QObject>
#include <QAbstractButton>
#include <deviceicon.h>
#include <scopycore_export.h>

namespace adiscope {
class SCOPYCORE_EXPORT DeviceIconBuilder : public QObject
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
