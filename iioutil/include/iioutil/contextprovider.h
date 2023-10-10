#ifndef CONTEXTPROVIDER_H
#define CONTEXTPROVIDER_H

#include "scopy-iioutil_export.h"

#include <iio.h>

#include <QMap>
#include <QObject>

#include <memory>
#include <mutex>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT ContextRefCounter
{
public:
	ContextRefCounter(QString uri);
	~ContextRefCounter();
	QString uri;
	struct iio_context *ctx = nullptr;
	int refcnt = 0;
};

class SCOPY_IIOUTIL_EXPORT ContextProvider : public QObject
{
	Q_OBJECT
protected:
	ContextProvider(QObject *parent = nullptr);
	~ContextProvider();
	QString name;

public:
	ContextProvider(ContextProvider &other) = delete;
	void operator=(const ContextProvider &) = delete;

	static ContextProvider *GetInstance();
	struct iio_context *open(QString uri);
	void close(QString uri);

private:
	static ContextProvider *pinstance_;
	static std::mutex mutex_;
	QMap<QString, ContextRefCounter *> map;
};
} // namespace scopy

#endif // CONTEXTPROVIDER_H
