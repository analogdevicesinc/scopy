#ifndef SWIOTPINGTASK_H
#define SWIOTPINGTASK_H

#include <iio.h>

#include <QThread>

#include "command.h"
#include "connection.h"
#include "pingtask.h"

namespace scopy {
class SCOPY_IIOUTIL_EXPORT CmdQPingTask : public PingTask
{
	Q_OBJECT
public:
	CmdQPingTask(Connection *conn, QObject *parent = nullptr);
	CmdQPingTask(Connection *conn, QString pingDevice, QObject *parent = nullptr);
	~CmdQPingTask();
	void run() override;
	bool ping() override;

protected:
	QString m_pingDevice = "";
	Connection *c;

private Q_SLOTS:
	void getTriggerCommandFinished(scopy::Command *cmd);
};
} // namespace scopy
#endif // SWIOTPINGTASK_H
