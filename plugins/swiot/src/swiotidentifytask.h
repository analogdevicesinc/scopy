#ifndef SWIOTIDENTIFYTASK_H
#define SWIOTIDENTIFYTASK_H

#include <QThread>
namespace scopy::swiot {
class SwiotIdentifyTask : public QThread {
public:
	SwiotIdentifyTask(QString uri, QObject *parent = nullptr);
	~SwiotIdentifyTask();
	void run() override;
Q_SIGNALS:
	void hasPerformedIdentification();
private:
	QString m_uri;
};
}
#endif // SWIOTIDENTIFYTASK_H
