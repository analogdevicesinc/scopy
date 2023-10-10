#ifndef SWIOTIDENTIFYTASK_H
#define SWIOTIDENTIFYTASK_H

#include <QThread>
namespace scopy::swiot {
class SwiotIdentifyTask : public QThread
{
public:
	SwiotIdentifyTask(QString uri, QObject *parent = nullptr);
	~SwiotIdentifyTask();
	void run() override;

private:
	QString m_uri;
};
} // namespace scopy::swiot
#endif // SWIOTIDENTIFYTASK_H
