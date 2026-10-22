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

#include "component/contextbackend.h"
#include "component/component_export.h"

#include <QMap>
#include <QObject>
#include <QSet>
#include <QThreadPool>
#include <QTimer>

namespace scopy::component {

// Auto-scan source. Diffs consecutive scan results and emits found/lost per URI.
// Held URIs (e.g. a connected USB device, which a scan no longer reports once claimed) are
// never reported as lost.
class COMPONENT_EXPORT DeviceMonitor : public QObject
{
	Q_OBJECT
public:
	explicit DeviceMonitor(QObject *parent = nullptr);
	~DeviceMonitor() override;

	virtual void start(int periodMs) = 0;
	virtual void stop() = 0;
	// One scan, outside the periodic schedule.
	virtual void scanNow() = 0;
	// One scan on the calling thread; found/lost are emitted before it returns.
	virtual void scanBlocking() = 0;

	void hold(const QString &uri);
	void release(const QString &uri);
	// Drop uri from the known set, so the next scan reports it as found again.
	void forget(const QString &uri);
	void clear();

Q_SIGNALS:
	void found(const scopy::component::ScanResult &result);
	void lost(const scopy::component::ScanResult &result);

protected:
	// Called on the monitor's thread with a complete scan result.
	void publish(const QVector<ScanResult> &results);

private:
	QMap<QString, ScanResult> m_known;
	QSet<QString> m_held;
};

// Polls ContextBackend::scan(filter) on a worker thread, one scan at a time.
class COMPONENT_EXPORT PollingDeviceMonitor : public DeviceMonitor
{
	Q_OBJECT
public:
	PollingDeviceMonitor(ContextBackend *backend, QString filter, QObject *parent = nullptr);
	~PollingDeviceMonitor() override;

	void start(int periodMs) override;
	void stop() override;
	void scanNow() override;
	void scanBlocking() override;

private:
	ContextBackend *m_backend;
	QString m_filter;
	QTimer *m_timer;
	QThreadPool m_pool;
	bool m_busy = false;
};

} // namespace scopy::component
