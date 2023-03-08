#ifndef IIOSCANTASK_H
#define IIOSCANTASK_H

#include <QThread>
#include "scopyiioutil_export.h"

namespace adiscope {
/**
 * @brief The IIOScanTask class
 * IIOScanTask - scans for IIO context and emits a scanFinished signal
 */
class SCOPYIIOUTIL_EXPORT IIOScanTask : public QThread {
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
