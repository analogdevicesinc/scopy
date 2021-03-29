#ifndef CONTEXTENUMERATOR_HPP
#define CONTEXTENUMERATOR_HPP

#include <QFuture>
#include <QFutureWatcher>
#include <QTimer>
#include <QVector>

namespace scopy {
namespace core {

class ContextEnumerator : public QObject
{
	Q_OBJECT
public:
	ContextEnumerator();

public:
	QStringList searchDevices();
	void start();

Q_SIGNALS:
	void printData(const QStringList& uris);

private Q_SLOTS:
	void search();

private:
	QTimer m_searchTimer;
	QFuture<QStringList> m_future;
	QFutureWatcher<QStringList> m_watcher;
};
} // namespace core
} // namespace scopy

#endif // CONTEXTENUMERATOR_HPP
