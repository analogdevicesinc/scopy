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

#include "component/devicemonitor.h"

#include <QLoggingCategory>
#include <utility>

Q_LOGGING_CATEGORY(CAT_DEVICEMONITOR, "DeviceMonitor")

using namespace scopy::component;

DeviceMonitor::DeviceMonitor(QObject *parent)
	: QObject(parent)
{}

DeviceMonitor::~DeviceMonitor() = default;

void DeviceMonitor::hold(const QString &uri)
{
	if(m_known.contains(uri)) {
		m_held.insert(uri);
	}
}

void DeviceMonitor::release(const QString &uri) { m_held.remove(uri); }

void DeviceMonitor::forget(const QString &uri) { m_known.remove(uri); }

void DeviceMonitor::clear() { m_known.clear(); }

void DeviceMonitor::publish(const QVector<ScanResult> &results)
{
	QMap<QString, ScanResult> updated;
	for(const ScanResult &r : results) {
		updated.insert(r.uri, r);
	}
	for(const QString &uri : std::as_const(m_held)) {
		if(!updated.contains(uri) && m_known.contains(uri)) {
			updated.insert(uri, m_known.value(uri));
		}
	}

	const QMap<QString, ScanResult> previous = std::exchange(m_known, updated);
	for(auto it = updated.cbegin(); it != updated.cend(); ++it) {
		if(!previous.contains(it.key())) {
			qInfo(CAT_DEVICEMONITOR) << "new device found:" << it.key();
			Q_EMIT found(it.value());
		}
	}
	for(auto it = previous.cbegin(); it != previous.cend(); ++it) {
		if(!updated.contains(it.key())) {
			qInfo(CAT_DEVICEMONITOR) << "device lost:" << it.key();
			Q_EMIT lost(it.value());
		}
	}
}

PollingDeviceMonitor::PollingDeviceMonitor(ContextBackend *backend, QString filter, QObject *parent)
	: DeviceMonitor(parent)
	, m_backend(backend)
	, m_filter(std::move(filter))
	, m_timer(new QTimer(this))
{
	m_pool.setMaxThreadCount(1);
	connect(m_timer, &QTimer::timeout, this, &PollingDeviceMonitor::scanNow);
}

PollingDeviceMonitor::~PollingDeviceMonitor()
{
	// Queued results posted to a destroyed object are discarded by Qt.
	stop();
}

void PollingDeviceMonitor::start(int periodMs)
{
	if(m_timer->isActive()) {
		return;
	}
	scanNow();
	m_timer->start(periodMs);
}

void PollingDeviceMonitor::stop()
{
	m_timer->stop();
	// Like the former CyclicalTask: no scan keeps running once stopped (e.g. while a USB device is claimed).
	m_pool.waitForDone();
}

void PollingDeviceMonitor::scanBlocking()
{
	m_pool.waitForDone();
	publish(m_backend->scan(m_filter));
}

void PollingDeviceMonitor::scanNow()
{
	if(m_busy) {
		return;
	}
	m_busy = true;
	m_pool.start([this]() {
		QVector<ScanResult> results = m_backend->scan(m_filter);
		QMetaObject::invokeMethod(
			this,
			[this, results]() {
				m_busy = false;
				publish(results);
			},
			Qt::QueuedConnection);
	});
}
