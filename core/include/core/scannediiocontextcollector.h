#ifndef SCANNEDIIOCONTEXTCOLLECTOR_H
#define SCANNEDIIOCONTEXTCOLLECTOR_H

#include "device.h"
#include "scopy-core_export.h"

#include <QObject>
#include <QSet>

namespace scopy {
class SCOPY_CORE_EXPORT ScannedIIOContextCollector : public QObject
{
	Q_OBJECT
public:
	explicit ScannedIIOContextCollector(QObject *parent = nullptr);
	~ScannedIIOContextCollector();

public Q_SLOTS:
	void update(QStringList uris);
	void clearCache();
	void lock(QString, Device *);
	void unlock(QString, Device *);
	void removeDevice(QString id, Device *d);
Q_SIGNALS:
	void foundDevice(QString cat, QString uri);
	void lostDevice(QString cat, QString uri);

private:
	QSet<QString> uris;
	QSet<QString> lockedUris;
};
} // namespace scopy

#endif // SCANNEDIIOCONTEXTCOLLECTOR_H
