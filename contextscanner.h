#ifndef CONTEXTSCANNER_H
#define CONTEXTSCANNER_H

#include <QObject>
#include <QTimer>
#include <iio.h>

namespace adiscope {

class ContextScanner : public QObject
{
	Q_OBJECT
public:
	ContextScanner(QObject *parent = nullptr);
	~ContextScanner();
	void startScan(int period = 5000);
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
#endif // CONTEXTSCANNER_H
