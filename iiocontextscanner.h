#ifndef IIOCONTEXTSCANNER_H
#define IIOCONTEXTSCANNER_H

#include <QObject>
#include <QTimer>
#include <iio.h>
#include "iiocontextscannerthread.h"

namespace adiscope {

class IIOContextScanner : public QObject
{
	Q_OBJECT
public:
	IIOContextScanner(QObject *parent = nullptr);
	~IIOContextScanner();
	void startScan(int period = 5000, QString scanParams = "");
	void stopScan();
	static int scan(QStringList *ctxs, QString scanParams);

Q_SIGNALS:
	void scanFinished(QStringList);

private Q_SLOTS:
	void startScanThread();
private:
	QTimer *t;
	bool enabled = false;
	IIOContextScannerThread* scannerThread;
	QString scanParams;
	QMetaObject::Connection scanFinishedSignal;
	const int THREAD_FINISH_TIMEOUT = 30000;
};
}
#endif // IIOCONTEXTSCANNER_H
