#ifndef SWIOTIDENTIFYTASK_H
#define SWIOTIDENTIFYTASK_H

#include <QThread>
#include <iioutil/connection.h>

namespace scopy::swiot {
class SwiotIdentifyTask : public QThread
{
public:
	SwiotIdentifyTask(QString uri, QObject *parent = nullptr);
	~SwiotIdentifyTask();
	void run() override;

private:
	QString m_uri;
	Connection *m_conn;
};
} // namespace scopy::swiot
#endif // SWIOTIDENTIFYTASK_H
