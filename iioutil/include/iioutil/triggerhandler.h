#ifndef SCOPY_TRIGGERHANDLER_H
#define SCOPY_TRIGGERHANDLER_H

#include <iio.h>
#include <QObject>
#include <QList>
#include "scopy-iioutil_export.h"

namespace scopy {
class SCOPY_IIOUTIL_EXPORT TriggerHandler : public QObject
{
	Q_OBJECT
public:
	TriggerHandler() = default;
	~TriggerHandler() = default;

	virtual QList<QString> getAvailableTriggerNames() = 0;
	virtual QString getTriggerFromDevice(QString device) = 0;
	virtual struct iio_device *getTriggerWithName(QString trigger) = 0;

public Q_SLOTS:
	virtual void setTrigger(QString deviceName, QString triggerName) = 0;
	virtual void removeTrigger(QString deviceName) = 0;
};
} // namespace scopy

#endif // SCOPY_TRIGGERHANDLER_H
