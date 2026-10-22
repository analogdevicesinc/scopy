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

#include "devicescanner.h"

#include <QLoggingCategory>

#include <component/controller.h>

Q_LOGGING_CATEGORY(CAT_DEVICESCANNER, "DeviceScanner")

using namespace scopy;

DeviceScanner::DeviceScanner(QObject *parent)
	: QObject(parent)
{
	component::Controller *c = component::Controller::GetInstance();
	connect(c, &component::Controller::backendRegistered, this, &DeviceScanner::attach);
	connect(c, &component::Controller::backendUnregistered, this, &DeviceScanner::detach);
	const QList<component::BackendId> ids = c->backends();
	for(const component::BackendId &id : ids) {
		attach(id);
	}
}

DeviceScanner::~DeviceScanner() { stop(); }

void DeviceScanner::attach(const component::BackendId &id)
{
	if(m_monitors.contains(id)) {
		return;
	}
	component::DeviceMonitor *m = component::Controller::GetInstance()->monitor(id);
	if(!m) {
		return;
	}
	m_monitors.insert(id, m);
	qInfo(CAT_DEVICESCANNER) << "monitoring backend" << id;

	connect(m, &component::DeviceMonitor::found, this,
		[this](const component::ScanResult &r) { Q_EMIT found(r.backend, r.uri); });
	connect(m, &component::DeviceMonitor::lost, this,
		[this](const component::ScanResult &r) { Q_EMIT lost(r.backend, r.uri); });
	if(m_running) {
		m->start(m_periodMs);
	}
}

void DeviceScanner::detach(const component::BackendId &id)
{
	// The monitor belongs to the backend and may already be gone (QPointer).
	QPointer<component::DeviceMonitor> m = m_monitors.take(id);
	if(m) {
		m->stop();
		disconnect(m, nullptr, this, nullptr);
	}
}

QList<component::DeviceMonitor *> DeviceScanner::monitors() const
{
	QList<component::DeviceMonitor *> list;
	for(const QPointer<component::DeviceMonitor> &m : m_monitors) {
		if(m) {
			list.append(m);
		}
	}
	return list;
}

void DeviceScanner::start(int periodMs)
{
	if(m_running) {
		return;
	}
	m_running = true;
	m_periodMs = periodMs;
	for(component::DeviceMonitor *m : monitors()) {
		m->start(periodMs);
	}
}

void DeviceScanner::stop()
{
	if(!m_running) {
		return;
	}
	m_running = false;
	for(component::DeviceMonitor *m : monitors()) {
		m->stop();
	}
}

bool DeviceScanner::isRunning() const { return m_running; }

void DeviceScanner::scanNow()
{
	for(component::DeviceMonitor *m : monitors()) {
		m->scanNow();
	}
}

void DeviceScanner::scanBlocking()
{
	for(component::DeviceMonitor *m : monitors()) {
		m->scanBlocking();
	}
}

void DeviceScanner::hold(const QString &uri)
{
	for(component::DeviceMonitor *m : monitors()) {
		m->hold(uri);
	}
}

void DeviceScanner::release(const QString &uri)
{
	for(component::DeviceMonitor *m : monitors()) {
		m->release(uri);
	}
}

void DeviceScanner::forget(const QString &uri)
{
	for(component::DeviceMonitor *m : monitors()) {
		m->forget(uri);
	}
}

QList<component::BackendId> DeviceScanner::backends() const { return m_monitors.keys(); }

#include "moc_devicescanner.cpp"
