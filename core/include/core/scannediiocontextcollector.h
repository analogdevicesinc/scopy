#ifndef SCANNEDIIOCONTEXTCOLLECTOR_H
#define SCANNEDIIOCONTEXTCOLLECTOR_H

#include <QObject>
#include <QSet>
#include "device.h"
#include "scopycore_export.h"

namespace adiscope {
class SCOPYCORE_EXPORT ScannedIIOContextCollector : public QObject
{
	Q_OBJECT
public:
	explicit ScannedIIOContextCollector(QObject *parent = nullptr);
	~ScannedIIOContextCollector();

public Q_SLOTS:
	void update(QStringList uris);
	void clearCache();
	void lock(QString, Device*);
	void unlock(QString, Device*);
	void removeDevice(QString id, Device *d);
Q_SIGNALS:
	void foundDevice(QString cat, QString uri);
	void lostDevice(QString cat, QString uri);
private:
	QSet<QString> uris;
	QSet<QString> lockedUris;

};
}

#endif // SCANNEDIIOCONTEXTCOLLECTOR_H
