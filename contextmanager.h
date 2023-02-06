#ifndef CONTEXTMANAGER_H
#define CONTEXTMANAGER_H

#include <QObject>
#include <QMap>
#include <memory>
#include <iio.h>
#include <mutex>

class ContextRefCounter {
public:
	ContextRefCounter(QString uri);
	~ContextRefCounter();
	QString uri;
	struct iio_context* ctx = nullptr;
	int refcnt = 0;
};

class ContextManager : public QObject
{
Q_OBJECT
protected:
	ContextManager(QObject *parent = nullptr);
	~ContextManager();
	QString name;
public:

	ContextManager(ContextManager &other) = delete;
	void operator=(const ContextManager &) = delete;

	static ContextManager *GetInstance();
	struct iio_context* open(QString uri);
	void close(QString uri);

private:
	static ContextManager * pinstance_;
	static std::mutex mutex_;
	QMap<QString, ContextRefCounter*> map;

};

#endif // CONTEXTMANAGER_H
