#ifndef SWIOTPINGTASK_H
#define SWIOTPINGTASK_H

#include "scopy-swiot_export.h"

#include <iio.h>

#include <QThread>

#include <iioutil/command.h>
#include <iioutil/connection.h>

namespace scopy::swiot {
class SwiotPingTask : public QThread
{
	Q_OBJECT
public:
	SwiotPingTask(Connection *conn, QObject *parent = nullptr);
	~SwiotPingTask();
	virtual void run() override;
Q_SIGNALS:
	void pingSuccess();
	void pingFailed();

protected:
	QString m_uri;
	Connection *c;
	bool enabled;

private Q_SLOTS:
	void getTriggerCommandFinished(scopy::Command *cmd);
};
} // namespace scopy::swiot
#endif // SWIOTPINGTASK_H
