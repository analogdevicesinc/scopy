#ifndef IIOCONTEXTSCANNER_H
#define IIOCONTEXTSCANNER_H

#include <QObject>
#include <QTimer>
#include <iio.h>

namespace adiscope {

class IIOContextScanner : public QObject
{
	Q_OBJECT
public:
	IIOContextScanner(QObject *parent = nullptr);
	~IIOContextScanner();
	void startScan(int period = 5000, bool now = true);
	void stopScan();
	void setScanParams(QString s);

Q_SIGNALS:
	void scanFinished(QStringList);

public Q_SLOTS:
	void scan();
private:
	QTimer *t;
	QString scanParams;
};
}
#endif // IIOCONTEXTSCANNER_H
