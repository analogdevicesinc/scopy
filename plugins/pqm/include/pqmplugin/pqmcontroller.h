#ifndef PQMCONTROLLER_H
#define PQMCONTROLLER_H

#include <QObject>
#include <iioutil/cyclicaltask.h>
#include <iioutil/iiopingtask.h>

namespace scopy::pqm {
class PqmController : public QObject
{
	Q_OBJECT
public:
	PqmController(QString uri, QObject *parent = nullptr);
	~PqmController();

	void startPingTask(iio_context *ctx);
	void stopPingTask();

Q_SIGNALS:
	void pingSuccess();
	void pingFailed();

private:
	QString m_uri;
	IIOPingTask *m_pingTask;
	CyclicalTask *m_pingTimer;
};
} // namespace scopy::pqm

#endif // PQMCONTROLLER_H
