#ifndef IIOCONTEXTSCANNERTHREAD_H
#define IIOCONTEXTSCANNERTHREAD_H

#include <QObject>
#include <QThread>


namespace adiscope {

class IIOContextScannerThread : public QThread
{
	Q_OBJECT
public:
	IIOContextScannerThread(QObject *parent = nullptr);
	~IIOContextScannerThread();
	void setScanParams(QString s);

Q_SIGNALS:
	void scanFinished(QStringList);

protected:
	void run() override;
	QString scanParams = "";
};
}

#endif // IIOCONTEXTSCANNERTHREAD_H
