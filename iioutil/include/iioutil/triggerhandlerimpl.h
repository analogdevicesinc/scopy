#ifndef SCOPY_TRIGGERHANDLERIMPL_H
#define SCOPY_TRIGGERHANDLERIMPL_H

#include "triggerhandler.h"
#include "scopy-iioutil_export.h"
#include <QObject>
#include <QMap>

#define NO_TRIGGER_NAME "None"

namespace scopy {
class SCOPY_IIOUTIL_EXPORT TriggerHandlerImpl : public TriggerHandler
{
	Q_OBJECT
public:
	explicit TriggerHandlerImpl(struct iio_context *context, QObject *parent = nullptr);
	~TriggerHandlerImpl() override;

	QList<QString> getAvailableTriggerNames() override;
	QString getTriggerFromDevice(QString device) override;
	struct iio_device *getTriggerWithName(QString trigger) override;

public Q_SLOTS:
	void setTrigger(QString deviceName, QString triggerName) override;
	void removeTrigger(QString deviceName) override;

private:
	void init();
	QString getMatchingTriggerForDevice(const QString &deviceName);
	void applyMatchingTriggers();

	struct iio_context *m_context;
	QMap<QString, QString> m_connections; // <device, trigger>
	QMap<QString, struct iio_device *> m_triggers;
};
} // namespace scopy

#endif // SCOPY_TRIGGERHANDLERIMPL_H
