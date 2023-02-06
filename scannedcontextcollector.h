#ifndef SCANNEDCONTEXTCOLLECTOR_H
#define SCANNEDCONTEXTCOLLECTOR_H

#include <QObject>
#include <QSet>
class ScannedContextCollector : public QObject
{
	Q_OBJECT
public:
	explicit ScannedContextCollector(QObject *parent = nullptr);
	~ScannedContextCollector();

public Q_SLOTS:
	void update(QStringList uris);
	void clearCache();
Q_SIGNALS:
	void newDevice(QString uri);
	void deleteDevice(QString uri);
private:
	QSet<QString> uris;

};

#endif // SCANNEDCONTEXTCOLLECTOR_H
