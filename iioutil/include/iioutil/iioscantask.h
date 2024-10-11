#ifndef IIOSCANTASK_H
#define IIOSCANTASK_H

#include "scopy-iioutil_export.h"

#include <QThread>

namespace scopy {
/**
 * @brief The IIOScanTask class
 * IIOScanTask - scans for IIO context and emits a scanFinished signal
 */
class SCOPY_IIOUTIL_EXPORT IIOScanTask : public QThread
{
	Q_OBJECT
public:
	IIOScanTask(QObject *parent);
	~IIOScanTask();

	virtual void run() override;
	void setScanParams(QString s);
	static int scan(QVector<QPair<QString, QString>> *ctxs, QString scanParams);
	static QVector<QString> getSerialPortsName();

Q_SIGNALS:
	void scanFinished(QVector<QPair<QString, QString>> ctxs);

protected:
	static QString parseDescription(const QString &d);
	QString scanParams = "";
	bool enabled;
};
} // namespace scopy
#endif // IIOSCANTASK_H
