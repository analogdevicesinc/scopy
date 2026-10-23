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

#include "fakectxbackend.h"

#include <QDateTime>
#include <QStringList>

#include <algorithm>
#include <functional>

#include <component/attribute.h>
#include <component/context.h>
#include <component/device.h>

using namespace scopy;
using namespace scopy::component;

namespace {
// Read capability without I/O: each read returns value() and updates the cached value.
class FakeAttributeReader : public AttributeReader
{
public:
	explicit FakeAttributeReader(std::function<QString()> value, QObject *parent = nullptr)
		: AttributeReader(parent)
		, m_value(std::move(value))
	{}

	QCoro::Task<CommandResponse<QByteArray>> readAsync(size_t) override
	{
		Result<QByteArray> r(m_value().toUtf8());
		Q_EMIT readSucceeded(r);
		co_return CommandResponse<QByteArray>(QUuid::createUuid(), r);
	}

private:
	std::function<QString()> m_value;
};

Attribute *addAttribute(QObject *parent, const QString &name, std::function<QString()> value)
{
	auto *attr = new Attribute(parent);
	attr->setName(name);
	attr->addReadCapability(new FakeAttributeReader(std::move(value)));
	return attr;
}
} // namespace

FakeCtxBackend::FakeCtxBackend(int deviceCount)
	: m_deviceCount(deviceCount)
{}

FakeCtxBackend::~FakeCtxBackend() = default;

Context *FakeCtxBackend::create(const QString &uri, const int)
{
	const QVector<ScanResult> devices = available();
	if(std::none_of(devices.cbegin(), devices.cend(), [&uri](const ScanResult &r) { return r.uri == uri; })) {
		return nullptr;
	}

	auto *ctx = new Context;
	ctx->setUri(uri);
	ctx->setName("fake");
	ctx->setDescription("Fake device " + uri);
	addAttribute(ctx, "hw_model", [uri]() { return "Fake device (" + uri + ")"; });
	addAttribute(ctx, "backend", []() { return QString(ID); });

	auto *dev = new Device(ctx);
	dev->setId("fakedev");
	dev->setName("fakedev");
	auto counter = std::make_shared<int>(0);
	addAttribute(dev, "counter", [counter]() { return QString::number(++*counter); });
	addAttribute(dev, "uptime", []() { return QDateTime::currentDateTime().toString(Qt::ISODate); });
	return ctx;
}

QVector<ScanResult> FakeCtxBackend::scan(const QString &) { return available(); }

QVector<ScanResult> FakeCtxBackend::available() const
{
	QVector<ScanResult> results;
	for(int i = 0; i < m_deviceCount; i++) {
		const QString uri = QString("fake:%1").arg(i);
		results.append({uri, "Fake device " + uri, ID});
	}
	if((QDateTime::currentSecsSinceEpoch() / HOTPLUG_PERIOD_S) % 2 == 0) {
		results.append({HOTPLUG_URI, "Fake hotplug device", ID});
	}
	return results;
}

DeviceMonitor *FakeCtxBackend::monitor()
{
	if(!m_monitor) {
		m_monitor = std::make_unique<PollingDeviceMonitor>(this, QString());
	}
	return m_monitor.get();
}
