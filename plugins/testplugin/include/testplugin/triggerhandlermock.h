#ifndef SCOPY_TRIGGERHANDLERMOCK_H
#define SCOPY_TRIGGERHANDLERMOCK_H

#include <iioutil/triggerhandler.h>
#include <QMap>

namespace scopy {
class TriggerHandlerMock : public TriggerHandler
{
	Q_OBJECT
public:
	explicit TriggerHandlerMock(QObject *parent = nullptr);

	QList<QString> getAvailableTriggerNames() override;
	void setTrigger(QString deviceName, QString triggerName) override;
	void removeTrigger(QString deviceName) override;

	QString getTriggerFromDevice(QString device) override;
	struct iio_device *getTriggerWithName(QString trigger) override;

	QMultiMap<QString, QString> getConnections() { return m_triggerConnections; }

private:
	QList<QString> m_triggerList{"trig1", "trig2", "trig3", "trig1-dev"};
	QMap<QString, QString> m_triggerConnections;
};
} // namespace scopy

#endif // SCOPY_TRIGGERHANDLERMOCK_H
