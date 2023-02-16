#ifndef DEVICEPAGEBUILDER_H
#define DEVICEPAGEBUILDER_H

#include <QObject>
#include <scopycore_export.h>

class SCOPYCORE_EXPORT DevicePageBuilder : public QObject
{
	Q_OBJECT
public:
	explicit DevicePageBuilder(QObject *parent = nullptr);

signals:

};

#endif // DEVICEPAGEBUILDER_H
