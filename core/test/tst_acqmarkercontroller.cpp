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
#include <core/acq_engine/datastore.h>
#include <core/acq_ui/acqaxis.h>
#include <core/acq_ui/acqcurvechannel.h>
#include <core/acq_ui/acqmarkercontroller.h>
#include <core/acq_ui/acqplot.h>
#include <core/acq_ui/acqplotmanager.h>
#include <gui/instrumenttemplate.h>

#include <QTest>

using namespace scopy;
using namespace scopy::adc;
using scopy::acq::DataKey;
using Step = AcqMarkerController::PeakStep;

class TST_AcqMarkerController : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void fixedMarkerStepsBetweenPeaks();
};

void TST_AcqMarkerController::fixedMarkerStepsBetweenPeaks()
{
	const QVector<float> y{0, 1, 5, 1, 2, 9, 2, 3, 7, 3, 0};
	QVector<float> x;
	for(int i = 0; i < y.size(); ++i) {
		x.append(i);
	}
	const DataKey yKey = DataKey::raw("dev", "mag");
	const DataKey xKey = DataKey::raw("dev", "hz");
	scopy::acq::DataStore store;
	store.write(yKey, y);
	store.write(xKey, x);

	InstrumentTemplate shell;
	AcqPlotManager mgr(&store, nullptr, &shell);
	AcqPlot *plot = mgr.addPlot("P", AcqPlotKind::Basic);
	auto *curve = qobject_cast<AcqCurveChannel *>(mgr.addChannel(plot, scopy::acq::ReprKind::Curve, yKey, xKey));
	QVERIFY(curve);
	curve->xAxis()->setInterval(0, y.size() - 1);
	curve->pull(y.size());

	AcqMarkerController mc(curve);
	mc.bind();
	mc.setEnabled(true);
	mc.setMarkerType(PlotMarkerController::MC_FIXED);
	auto markerX = [&mc]() { return mc.markerInfo().first().peak.x; };

	QVERIFY(mc.stepToPeak(0, Step::Highest));
	QCOMPARE(markerX(), 5.0);
	QVERIFY(mc.stepToPeak(0, Step::Right));
	QCOMPARE(markerX(), 8.0);
	QVERIFY(!mc.stepToPeak(0, Step::Right));
	QVERIFY(mc.stepToPeak(0, Step::Higher));
	QCOMPARE(markerX(), 5.0);
	QVERIFY(mc.stepToPeak(0, Step::Lower));
	QCOMPARE(markerX(), 8.0);
	QVERIFY(mc.stepToPeak(0, Step::Left));
	QCOMPARE(markerX(), 5.0);
	QVERIFY(mc.stepToPeak(0, Step::Left));
	QCOMPARE(markerX(), 2.0);

	mc.moveFixedMarker(1, 8.0);
	QCOMPARE(mc.markerInfo().at(1).peak.x, 8.0);
	QCOMPARE(mc.markerInfo().at(1).peak.y, 7.0);
}

QTEST_MAIN(TST_AcqMarkerController)
#include "tst_acqmarkercontroller.moc"
