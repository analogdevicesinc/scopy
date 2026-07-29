#pragma once

#include <QObject>
#include <QTimer>

namespace scopy::component {

// Reachability capability (Pattern A base). Monitoring is async: the timer fires
// checkReachableAsync() (leaf dispatches the ping through the executor) and the
// leaf reports the outcome via reachabilityChecked(bool), so the GUI thread never
// blocks in a nested event loop. connectionLost() is emitted once, on first
// failure. checkReachable() stays for one-off explicit synchronous checks.
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

	virtual bool checkReachable() = 0;
	virtual void checkReachableAsync() = 0;

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
