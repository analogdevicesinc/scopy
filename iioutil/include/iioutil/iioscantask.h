#ifndef IIOSCANTASK_H
#define IIOSCANTASK_H

#include <QThread>
#include "scopyiioutil_export.h"
#include "task.h"

namespace adiscope {
class SCOPYIIOUTIL_EXPORT IIOScanTask : public Task {
	Q_OBJECT
public:
	IIOScanTask(QObject *parent);
	~IIOScanTask();

	virtual void run() override;
	void setScanParams(QString s);
	static int scan(QStringList *ctxs, QString scanParams);

Q_SIGNALS:
	void scanFinished(QStringList);

protected:
	QString scanParams = "";
	bool enabled;

};
}
#endif // IIOSCANTASK_H
