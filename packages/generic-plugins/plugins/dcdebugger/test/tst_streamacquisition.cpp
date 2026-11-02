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

#include "acquisition/streamacquisition.h"

#include "fakestream.h"

#include <QtTest>

using namespace scopy;
using namespace scopy::component;
using namespace scopy::dcdebugger;
using namespace scopy::dcdebugger::test;

class TST_StreamAcquisition : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void defaultsToEveryElement();
	void decodeFollowsEnabledElements();
	void singleCapturesOneFrameAndCloses();
	void continuousRunsUntilStopped();
	void samplesChangeReopensTheStream();
	void noChannelIsAnError();
	void findEdgeRisingAndFalling();
};

void TST_StreamAcquisition::defaultsToEveryElement()
{
	FakeInputStream stream(3);
	StreamAcquisition acq(&stream);
	QCOMPARE(acq.scanElements().size(), 3);
	QCOMPARE(acq.enabledChannels(), QList<int>({0, 1, 2}));
}

void TST_StreamAcquisition::decodeFollowsEnabledElements()
{
	FakeInputStream stream(3);
	StreamAcquisition acq(&stream);
	acq.setEnabledChannels({2, 0});
	acq.setSamples(8);
	QSignalSpy frames(&acq, &StreamAcquisition::frameReady);
	acq.start(true);
	QTRY_COMPARE(frames.size(), 1);
	const auto frame = frames.first().first().value<AcquiredFrame>();
	QCOMPARE(frame.ids, QStringList({"voltage0", "voltage2"}));
	QCOMPARE(frame.data[0].size(), 8);
	QCOMPARE(frame.data[0][3], 3.0);
	QCOMPARE(frame.data[1][5], 2005.0);
}

void TST_StreamAcquisition::singleCapturesOneFrameAndCloses()
{
	FakeInputStream stream(2);
	StreamAcquisition acq(&stream);
	QSignalSpy running(&acq, &StreamAcquisition::runningChanged);
	acq.start(true);
	QTRY_COMPARE(running.size(), 2);
	QCOMPARE(running.last().first().toBool(), false);
	QCOMPARE(stream.refills, 1);
	QVERIFY(!stream.isOpen());
}

void TST_StreamAcquisition::continuousRunsUntilStopped()
{
	FakeInputStream stream(1);
	StreamAcquisition acq(&stream);
	QSignalSpy frames(&acq, &StreamAcquisition::frameReady);
	acq.start(false);
	QTRY_VERIFY(frames.size() >= 3);
	acq.stop();
	QTRY_VERIFY(!acq.isRunning());
	QVERIFY(!stream.isOpen());
}

void TST_StreamAcquisition::samplesChangeReopensTheStream()
{
	FakeInputStream stream(1);
	StreamAcquisition acq(&stream);
	acq.setSamples(16);
	QSignalSpy frames(&acq, &StreamAcquisition::frameReady);
	acq.start(false);
	QTRY_VERIFY(frames.size() >= 1);
	acq.setSamples(32);
	QTRY_VERIFY(stream.samplesCount() == 32);
	QTRY_VERIFY(frames.last().first().value<AcquiredFrame>().data[0].size() == 32);
	QVERIFY(stream.opens >= 2);
	acq.shutdown();
	QVERIFY(!stream.isOpen());
	QVERIFY(!acq.isRunning());
}

void TST_StreamAcquisition::noChannelIsAnError()
{
	FakeInputStream stream(2);
	StreamAcquisition acq(&stream);
	acq.setEnabledChannels({});
	QSignalSpy errors(&acq, &StreamAcquisition::errorOccurred);
	acq.start(false);
	QCOMPARE(errors.size(), 1);
	QVERIFY(!acq.isRunning());
	QCOMPARE(stream.opens, 0);
}

void TST_StreamAcquisition::findEdgeRisingAndFalling()
{
	const QVector<double> d = {0, 1, 2, 3, 2, 1, 0, 1, 2};
	QCOMPARE(findEdge(d, 1.5, true), 2);
	QCOMPARE(findEdge(d, 1.5, false), 5);
	QCOMPARE(findEdge(d, 1.5, true, 3), 8);
	QCOMPARE(findEdge(d, 1.5, true, 3, 8), -1);
	QCOMPARE(findEdge(d, 10, true), -1);

	// Noise around the level: hysteresis keeps it from firing or re-arming.
	const QVector<double> noisy = {0, 0.9, 1.1, 0.9, 1.1, 0.9, 2, 3, 2, 0};
	QCOMPARE(findEdge(noisy, 1.0, true), 2);
	QCOMPARE(findEdge(noisy, 1.0, true, 1, -1, 0.5), -1); // starts above level - h: never armed
	QCOMPARE(findEdge(noisy, 1.0, true, 0, -1, 0.5), 2);  // armed at 0, first crossing at 2
	const QVector<double> rearm = {0, 1.2, 0.95, 1.2, 0.2, 1.5};
	QCOMPARE(findEdge(rearm, 1.0, true, 2, -1, 0.5), 5); // 0.95 does not re-arm, 0.2 does
}

QTEST_GUILESS_MAIN(TST_StreamAcquisition)
#include "tst_streamacquisition.moc"
