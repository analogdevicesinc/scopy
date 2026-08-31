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

#include "backendfixture.h"

#include "iioutil/iscanops.h"

using namespace scopy;
using namespace scopy::iio;

class TestScanOps : public QObject
{
	Q_OBJECT
private slots:
	// scan() must be callable on every backend and return a (possibly empty)
	// list without crashing — no live context needed.
	void scanDoesNotCrash();
	void scanDoesNotCrash_data() { dctest::addBackendRows(); }
};

void TestScanOps::scanDoesNotCrash()
{
	DC_REQUIRE_BACKEND();
	const QVector<ScanResult> results = b->scanOps()->scan();
	// Each result, if any, must carry a non-empty URI.
	for(const ScanResult &r : results) {
		QVERIFY(!r.uri.isEmpty());
	}
}

QTEST_MAIN(TestScanOps)
#include "tst_scanops.moc"
