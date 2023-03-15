#ifndef M2KIDENTIFYTASK_H
#define M2KIDENTIFYTASK_H

#include <QThread>
namespace adiscope::m2k {
class M2kIdentifyTask : public QThread {
public:
	M2kIdentifyTask(QString uri, QObject *parent = nullptr);
	~M2kIdentifyTask();
	void run() override;
private:
	QString m_uri;
};
}
#endif // M2KIDENTIFYTASK_H
