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

#include <QTest>

using namespace scopy;

class TST_M2kDeviceImpl : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void queuedSwapIsAbandonedOnDisconnect();
	void destroyedBeforeTheSwapRunsDoesNotCrash();
};

void TST_M2kDeviceImpl::queuedSwapIsAbandonedOnDisconnect()
{
	M2kDeviceImpl dev(QStringLiteral("not-a-uri"), 1);
	dev.connectDev();
	dev.disconnectDev();
	QTest::qWait(50);
	QVERIFY(!dev.hasPendingBackendSwap());
}

void TST_M2kDeviceImpl::destroyedBeforeTheSwapRunsDoesNotCrash()
{
	auto *dev = new M2kDeviceImpl(QStringLiteral("not-a-uri"), 1);
	dev->connectDev();
	delete dev;
	QTest::qWait(50);
	QVERIFY(true); // reaching here without a crash is the assertion
}

QTEST_MAIN(TST_M2kDeviceImpl)

#include "tst_m2kdeviceimpl.moc"
