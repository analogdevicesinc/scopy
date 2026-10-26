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

#ifndef FAKECTXBACKEND_H
#define FAKECTXBACKEND_H

#include <component/connectionprovider.h>
#include <component/contextbackend.h>
#include <component/devicemonitor.h>

#include <memory>

namespace scopy {

// Test transport registered at runtime from FakeBackendPlugin::init(). Opens hardware-free
// connections for "fake:<n>" uris and reports them through a polling monitor. "fake:hotplug"
// appears and disappears every HOTPLUG_PERIOD_S seconds to exercise found/lost.
class FakeProvider : public component::ConnectionProvider
{
public:
	// Also the device category, so plugins declare "category": ["fake"].
	static constexpr const char *KIND = "fake";
	static constexpr const char *HOTPLUG_URI = "fake:hotplug";
	static constexpr int HOTPLUG_PERIOD_S = 10;

	explicit FakeProvider(int deviceCount);
	~FakeProvider() override;

	component::ConnectionKind kind() const override { return KIND; }
	component::Connection *open(const QString &uri) override;
	QVector<component::ScanResult> scan(const QString &filter = QString()) override;
	component::DeviceMonitor *monitor() override;
	QString uriHint() const override { return "fake:0, fake:hotplug"; }

private:
	QVector<component::ScanResult> available() const;

	int m_deviceCount;
	std::unique_ptr<component::DeviceMonitor> m_monitor;
};

// Test view over a FakeProvider connection; plugins reach it with "category": ["fake"].
class FakeCtxBackend : public component::ContextBackend
{
public:
	static constexpr const char *ID = "fake";

	component::BackendId id() const override { return ID; }
	component::ConnectionKind connectionKind() const override { return FakeProvider::KIND; }
	bool compatible(const component::Connection &c) const override { return c.kind() == FakeProvider::KIND; }
	component::Context *create(component::Connection &c) override;
};

} // namespace scopy

#endif // FAKECTXBACKEND_H
