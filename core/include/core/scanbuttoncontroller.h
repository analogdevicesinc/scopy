#ifndef SCANBUTTONCONTROLLER_H
#define SCANBUTTONCONTROLLER_H

#include <QObject>
#include <QPushButton>
#include "scopy-core_export.h"
#include "iioutil/cyclicaltask.h"

namespace scopy {
class SCOPY_CORE_EXPORT ScanButtonController : public QObject
{
	Q_OBJECT
public:
	explicit ScanButtonController(CyclicalTask *cs , QPushButton *btn, QObject *parent);
	~ScanButtonController();

public Q_SLOTS:
	void enableScan(bool b);
	void startScan();
	void stopScan();
private:
	QPushButton *btn;
	CyclicalTask *cs;
	QMetaObject::Connection conn;

};
}

#endif // SCANBUTTONCONTROLLER_H
