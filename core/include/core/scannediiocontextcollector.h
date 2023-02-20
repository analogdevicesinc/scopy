#ifndef SCANNEDIIOCONTEXTCOLLECTOR_H
#define SCANNEDIIOCONTEXTCOLLECTOR_H

#include <QObject>
#include <QSet>
#include "scopycore_export.h"

class SCOPYCORE_EXPORT ScannedIIOContextCollector : public QObject
{
	Q_OBJECT
public:
	explicit ScannedIIOContextCollector(QObject *parent = nullptr);
	~ScannedIIOContextCollector();

public Q_SLOTS:
	void update(QStringList uris);
	void clearCache();
	void lock(QString);
	void unlock(QString);
Q_SIGNALS:
	void foundDevice(QString uri);
	void lostDevice(QString uri);
private:
	QSet<QString> uris;
	QSet<QString> lockedUris;

};

#endif // SCANNEDIIOCONTEXTCOLLECTOR_H
