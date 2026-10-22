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

#ifndef DEVICESCANNER_H
#define DEVICESCANNER_H

#include "scopy-core_export.h"

#include <QMap>
#include <QObject>
#include <QPointer>

#include <component/contextbackend.h>
#include <component/devicemonitor.h>

namespace scopy {

// Aggregates the auto-scan monitors of every registered backend. Monitors of backends registered
// later (e.g. by a package) are picked up through Controller::backendRegistered and started if a
// scan is running. found/lost carry the id of the backend that reported the uri, which is also the
// device category.
class SCOPY_CORE_EXPORT DeviceScanner : public QObject
{
	Q_OBJECT
public:
	explicit DeviceScanner(QObject *parent = nullptr);
	~DeviceScanner() override;

	void start(int periodMs);
	void stop();
	bool isRunning() const;
	void scanNow();
	// One scan per monitor on the calling thread; found/lost are emitted before it returns.
	void scanBlocking();

	// Broadcast to every monitor; a monitor ignores uris it did not report.
	void hold(const QString &uri);
	void release(const QString &uri);
	void forget(const QString &uri);

	QList<component::BackendId> backends() const;

Q_SIGNALS:
	void found(const QString &category, const QString &uri);
	void lost(const QString &category, const QString &uri);

private:
	void attach(const component::BackendId &id);
	void detach(const component::BackendId &id);
	QList<component::DeviceMonitor *> monitors() const;

	QMap<component::BackendId, QPointer<component::DeviceMonitor>> m_monitors;
	bool m_running = false;
	int m_periodMs = 0;
};

} // namespace scopy

#endif // DEVICESCANNER_H
