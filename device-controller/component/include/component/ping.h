/*
 * Copyright (c) 2026 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include "core/result.h"

#include <QDateTime>
#include <QObject>
#include <QTimer>

#include <algorithm>

#include <qcoro/qcorotask.h>
#include "component/component_export.h"

namespace scopy::component {

// Reachability capability (Pattern A base). A single async method: the timer
// fires checkReachableAsync() (leaf dispatches the ping through the executor) and
// the leaf reports the outcome via reachabilityChecked(bool), so the GUI thread
// never blocks in a nested event loop. connectionLost() is emitted once, on first
// failure. Callers needing a synchronous one-off wrap it: QCoro::waitFor(p->checkReachableAsync()).
//
// The monitoring state and period are runtime-controllable (start / stop / interval) and
// observable (monitoringChanged / intervalChanged); the outcome of the last check is kept
// (hasResult / lastReachable / lastCheckedAt / isLost, notified by resultChanged).
class COMPONENT_EXPORT Ping : public QObject
{
	Q_OBJECT
	Q_PROPERTY(bool monitoring READ isMonitoring WRITE setMonitoring NOTIFY monitoringChanged)
	Q_PROPERTY(int intervalMs READ intervalMs WRITE setIntervalMs NOTIFY intervalChanged)
	Q_PROPERTY(bool hasResult READ hasResult NOTIFY resultChanged)
	Q_PROPERTY(bool lastReachable READ lastReachable NOTIFY resultChanged)
	Q_PROPERTY(QDateTime lastCheckedAt READ lastCheckedAt NOTIFY resultChanged)
	Q_PROPERTY(bool lost READ isLost NOTIFY resultChanged)
public:
	static constexpr int DEFAULT_INTERVAL_MS = 2000;
	static constexpr int MIN_INTERVAL_MS = 100;

	explicit Ping(QObject *parent = nullptr)
		: QObject(parent)
	{
		m_timer.setSingleShot(false);
		connect(&m_timer, &QTimer::timeout, this, [this]() { checkReachableAsync(); });
		connect(this, &Ping::reachabilityChecked, this, &Ping::onReachabilityChecked);
	}
	~Ping() override = default;

	Q_INVOKABLE virtual QCoro::Task<CommandResponse<void>> checkReachableAsync() = 0;

	// Starts (or restarts) the periodic check with `intervalMs`, clearing the lost state.
	void startMonitoring(int intervalMs = DEFAULT_INTERVAL_MS)
	{
		const bool wasRunning = isMonitoring();
		const bool wasLost = m_lost;
		m_lost = false;
		storeInterval(intervalMs);
		m_timer.start(m_intervalMs);
		if(wasLost) {
			Q_EMIT resultChanged();
		}
		if(!wasRunning) {
			Q_EMIT monitoringChanged(true);
		}
	}
	void stopMonitoring()
	{
		const bool wasRunning = isMonitoring();
		m_timer.stop();
		if(wasRunning) {
			Q_EMIT monitoringChanged(false);
		}
	}
	bool isMonitoring() const { return m_timer.isActive(); }
	// Starts with the current interval, or stops.
	void setMonitoring(bool on)
	{
		if(on) {
			startMonitoring(m_intervalMs);
		} else {
			stopMonitoring();
		}
	}

	int intervalMs() const { return m_intervalMs; }
	// Restarts the running timer with the new period; while stopped only stores it.
	void setIntervalMs(int intervalMs)
	{
		if(!storeInterval(intervalMs)) {
			return;
		}
		if(isMonitoring()) {
			m_timer.start(m_intervalMs);
		}
	}

	bool hasResult() const { return m_hasResult; }
	bool lastReachable() const { return m_lastReachable; }
	QDateTime lastCheckedAt() const { return m_lastCheckedAt; }
	bool isLost() const { return m_lost; }

Q_SIGNALS:
	void reachabilityChecked(bool reachable);
	void connectionLost();
	void monitoringChanged(bool monitoring);
	void intervalChanged(int intervalMs);
	void resultChanged();

private Q_SLOTS:
	void onReachabilityChecked(bool reachable)
	{
		m_hasResult = true;
		m_lastReachable = reachable;
		m_lastCheckedAt = QDateTime::currentDateTime();
		if(m_lost || reachable) {
			Q_EMIT resultChanged();
			return;
		}
		m_lost = true;
		const bool wasRunning = isMonitoring();
		m_timer.stop();
		// State first: connectionLost() may tear the connection down.
		Q_EMIT resultChanged();
		if(wasRunning) {
			Q_EMIT monitoringChanged(false);
		}
		Q_EMIT connectionLost();
	}

private:
	// Returns true if the (clamped) interval changed.
	bool storeInterval(int intervalMs)
	{
		intervalMs = std::max(intervalMs, MIN_INTERVAL_MS);
		if(intervalMs == m_intervalMs) {
			return false;
		}
		m_intervalMs = intervalMs;
		Q_EMIT intervalChanged(m_intervalMs);
		return true;
	}

	QTimer m_timer;
	int m_intervalMs = DEFAULT_INTERVAL_MS;
	bool m_lost = false;
	bool m_hasResult = false;
	bool m_lastReachable = false;
	QDateTime m_lastCheckedAt;
};

} // namespace scopy::component
