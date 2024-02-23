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
	static Connection *open(struct iio_context *ctx);
	static Connection *open(QString uri);
	static void close(struct iio_context *ctx);
	static void close(Connection *conn);
	static void close(QString uri);
	static void closeAll(struct iio_context *ctx);
	static void closeAll(Connection *conn);
	static void closeAll(QString uri);

private:
	Connection *_open(struct iio_context *ctx);
	Connection *_open(QString uri);
	void _close(struct iio_context *ctx);
	void _close(Connection *conn);
	void _close(QString uri);
	void _closeAll(struct iio_context *ctx);
	void _closeAll(Connection *conn);
	void _closeAll(QString uri);
	void _closeAndRemove(QString uri);
	static ConnectionProvider *pinstance_;
	static std::mutex mutex_;
	QMap<QString, Connection *> map;
};
} // namespace scopy

#endif // CONNECTIONPROVIDER_H
