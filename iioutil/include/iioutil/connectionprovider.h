#ifndef CONNECTIONPROVIDER_H
#define CONNECTIONPROVIDER_H

#include "scopy-iioutil_export.h"
#include "connection.h"

#include <QObject>
#include <QMap>
#include <mutex>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT ConnectionProvider : public QObject
{
	Q_OBJECT
protected:
	ConnectionProvider(QObject *parent = nullptr);
	~ConnectionProvider();

public:
	ConnectionProvider(ConnectionProvider &other) = delete;
	void operator=(const ConnectionProvider &) = delete;

	static ConnectionProvider *GetInstance();
	static Connection *open(QString uri);
	static void close(QString uri);
	static void closeAll(QString uri);

private:
	Connection *_open(QString uri);
	void _close(QString uri);
	void _closeAll(QString uri);
	void _closeAndRemove(QString uri);
	static ConnectionProvider *pinstance_;
	static std::mutex mutex_;
	QMap<QString, Connection *> map;
};
} // namespace scopy

#endif // CONNECTIONPROVIDER_H
