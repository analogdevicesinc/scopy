#ifndef SCANBUTTONCONTROLLER_H
#define SCANBUTTONCONTROLLER_H

#include <QObject>
#include <iiocontextscanner.h>
#include <QPushButton>

namespace adiscope {
class ScanButtonController : public QObject
{
	Q_OBJECT
public:
	explicit ScanButtonController(IIOContextScanner *cs , QPushButton *btn, QObject *parent);
	~ScanButtonController();

public Q_SLOTS:
	void enableScan(bool b);
	void startScan();
	void stopScan();
private:
	QPushButton *btn;
	IIOContextScanner *cs;
	QMetaObject::Connection conn;

};
}

#endif // SCANBUTTONCONTROLLER_H
