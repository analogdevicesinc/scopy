#ifndef SCANNEDIIOCONTEXTCOLLECTOR_H
#define SCANNEDIIOCONTEXTCOLLECTOR_H

#include <QObject>
#include <QSet>
class ScannedIIOContextCollector : public QObject
{
	Q_OBJECT
public:
	explicit ScannedIIOContextCollector(QObject *parent = nullptr);
	~ScannedIIOContextCollector();

public Q_SLOTS:
	void update(QStringList uris);
	void clearCache();
Q_SIGNALS:
	void foundDevice(QString uri);
	void lostDevice(QString uri);
private:
	QSet<QString> uris;

};

#endif // SCANNEDIIOCONTEXTCOLLECTOR_H
