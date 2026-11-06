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

#include "core/m2kdeviceimpl.h"

#include <component/contextfactory.h>
#include <component/controller.h>

#include <QTest>

#include <atomic>
#include <memory>

using namespace scopy;

class CountingM2kFactory : public component::ContextFactory
{
public:
	component::Context *create(const QString &, const int) override
	{
		++calls;
		return nullptr;
	}

	std::atomic<int> calls{0};
};

class TST_M2kDeviceImpl : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void init();
	void queuedSwapIsAbandonedOnDisconnect();
	void destroyedBeforeTheSwapRunsDoesNotCrash();

private:
	std::shared_ptr<CountingM2kFactory> m_factory;
};

void TST_M2kDeviceImpl::init()
{
	m_factory = std::make_shared<CountingM2kFactory>();
	component::Controller::GetInstance()->registerFactory(component::BackendKind::M2k, m_factory);
}

void TST_M2kDeviceImpl::queuedSwapIsAbandonedOnDisconnect()
{
	M2kDeviceImpl dev(QStringLiteral("not-a-uri"), 1);
	dev.connectDev();
	QVERIFY(dev.hasPendingBackendSwap());
	dev.disconnectDev();
	QTest::qWait(50);
	QCOMPARE(m_factory->calls.load(), 0);
}

void TST_M2kDeviceImpl::destroyedBeforeTheSwapRunsDoesNotCrash()
{
	auto *dev = new M2kDeviceImpl(QStringLiteral("not-a-uri"), 1);
	dev->connectDev();
	QVERIFY(dev->hasPendingBackendSwap());
	delete dev;
	QTest::qWait(50);
	QCOMPARE(m_factory->calls.load(), 0);
}

QTEST_MAIN(TST_M2kDeviceImpl)

#include "tst_m2kdeviceimpl.moc"
