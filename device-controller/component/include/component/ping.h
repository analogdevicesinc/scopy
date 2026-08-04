#pragma once

#include "core/result.h"

#include <QObject>
#include <QTimer>

#include <qcoro/qcorotask.h>

namespace scopy::component {

// Reachability capability (Pattern A base). A single async method: the timer
// fires checkReachableAsync() (leaf dispatches the ping through the executor) and
// the leaf reports the outcome via reachabilityChecked(bool), so the GUI thread
// never blocks in a nested event loop. connectionLost() is emitted once, on first
// failure. Callers needing a synchronous one-off wrap it: QCoro::waitFor(p->checkReachableAsync()).
class Ping : public QObject
{
	Q_OBJECT
public:
	explicit Ping(QObject *parent = nullptr)
		: QObject(parent)
	{
		m_timer.setSingleShot(false);
		connect(&m_timer, &QTimer::timeout, this, [this]() { checkReachableAsync(); });
		connect(this, &Ping::reachabilityChecked, this, &Ping::onReachabilityChecked);
	}
	~Ping() override = default;

	Q_INVOKABLE virtual QCoro::Task<CommandResponse<void>> checkReachableAsync() = 0;

	void startMonitoring(int intervalMs)
	{
		m_lost = false;
		m_timer.start(intervalMs);
	}
	void stopMonitoring() { m_timer.stop(); }
	bool isMonitoring() const { return m_timer.isActive(); }

Q_SIGNALS:
	void reachabilityChecked(bool reachable);
	void connectionLost();

private Q_SLOTS:
	void onReachabilityChecked(bool reachable)
	{
		if(m_lost || reachable) {
			return;
		}
		m_lost = true;
		m_timer.stop();
		Q_EMIT connectionLost();
	}

private:
	QTimer m_timer;
	bool m_lost = false;
};

} // namespace scopy::component
