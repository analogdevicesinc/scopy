#ifndef SCANBUTTONCONTROLLER_H
#define SCANBUTTONCONTROLLER_H

#include "iioutil/cyclicaltask.h"
#include "scopy-core_export.h"

#include <QCheckBox>
#include <QObject>

namespace scopy {
class SCOPY_CORE_EXPORT ScanButtonController : public QObject
{
	Q_OBJECT
public:
	explicit ScanButtonController(CyclicalTask *cs, QCheckBox *btn, QObject *parent);
	~ScanButtonController();

public Q_SLOTS:
	void enableScan(bool b);
	void startScan();
	void stopScan();

private:
	QCheckBox *btn;
	CyclicalTask *cs;
	QMetaObject::Connection conn;
};
} // namespace scopy

#endif // SCANBUTTONCONTROLLER_H
