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

#include "fakestream.h"
#include "generation/streamgeneration.h"
#include "generation/waveform.h"

#include <QSignalSpy>
#include <QtTest>

#include <cmath>

using namespace scopy;
using namespace scopy::component;
using namespace scopy::dcdebugger;
using namespace scopy::dcdebugger::test;

class TST_StreamGeneration : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void snapKeepsWholePeriods();
	void waveformsHitTheirLevels();
	void defaultsArePhasedPairs();
	void cyclicPushesOnceWithEncodedTones();
	void cyclicChangeReopens();
	void streamingPushesInALoop();
	void disabledChannelsAreNotOpened();
};

void TST_StreamGeneration::snapKeepsWholePeriods()
{
	QCOMPARE(snapFrequency(1000, 100e3, 8192), std::round(1000 * 8192 / 100e3) * 100e3 / 8192);
	QCOMPARE(snapFrequency(1, 100e3, 1000), 100.0);		       // at least one period
	QCOMPARE(snapFrequency(1e9, 100e3, 1000), 499 * 100e3 / 1000); // below Nyquist: one period under it
	QCOMPARE(snapFrequency(1e9, 100e3, 999), 499 * 100e3 / 999);   // odd: the last whole period under it
	QCOMPARE(snapFrequency(1000, 0, 1000), 1000.0);		       // unknown rate: unchanged
	QCOMPARE(snapFrequency(0, 1e3, 100), 0.0);
}

void TST_StreamGeneration::waveformsHitTheirLevels()
{
	ToneSpec t;
	t.frequency = 1;
	t.amplitude = 0.5;
	t.offset = 0.25;
	t.waveform = Waveform::Sine;
	QVector<double> v = generateTone(t, 4, 4); // quarter periods: 0, 90, 180, 270 degrees
	QVERIFY(std::abs(v[0] - 0.25) < 1e-12);
	QVERIFY(std::abs(v[1] - 0.75) < 1e-12);
	QVERIFY(std::abs(v[3] + 0.25) < 1e-12);
	t.waveform = Waveform::Triangle;
	v = generateTone(t, 4, 4);
	QVERIFY(std::abs(v[1] - 0.75) < 1e-12);
	t.waveform = Waveform::Square;
	v = generateTone(t, 4, 4);
	QCOMPARE(v[0], 0.75);
	QCOMPARE(v[2], -0.25);
	t.phase = 90;
	t.waveform = Waveform::Sine;
	QVERIFY(std::abs(generateTone(t, 1, 4)[0] - 0.75) < 1e-12);
	t.waveform = Waveform::Constant;
	QCOMPARE(generateTone(t, 3, 4), QVector<double>({0.25, 0.25, 0.25}));
	t.offset = 0.9; // clamped to full scale
	t.waveform = Waveform::Square;
	QCOMPARE(generateTone(t, 1, 4)[0], 1.0);
}

void TST_StreamGeneration::defaultsArePhasedPairs()
{
	FakeOutputStream stream(2);
	StreamGeneration gen(&stream);
	QCOMPARE(gen.tones().size(), 2);
	QCOMPARE(gen.tones().value("voltage0").phase, 0.0);
	QCOMPARE(gen.tones().value("voltage1").phase, 90.0);
	QVERIFY(gen.cyclic());
}

void TST_StreamGeneration::cyclicPushesOnceWithEncodedTones()
{
	FakeOutputStream stream(2);
	StreamGeneration gen(&stream);
	gen.setSamples(8);
	gen.setSampleRate(8);
	QHash<QString, ToneSpec> tones = gen.tones();
	tones["voltage0"].waveform = Waveform::Constant;
	tones["voltage0"].offset = 0.5;
	tones["voltage1"].frequency = 1; // one period in the buffer, phase 90: starts at the top
	tones["voltage1"].amplitude = 1;
	gen.setTones(tones);

	QSignalSpy pushed(&gen, &StreamGeneration::pushed);
	gen.start();
	QTRY_COMPARE(pushed.size(), 1);
	QTest::qWait(20);
	QCOMPARE(stream.pushes, 1); // cyclic: replayed by the device
	QVERIFY(stream.lastPushCyclic);
	QCOMPARE(stream.lastPush.size(), 16);
	QCOMPARE(stream.lastPush[0], int16_t(16384)); // voltage0, interleaved first
	QCOMPARE(stream.lastPush[1], int16_t(32767)); // voltage1 at +full scale
	QCOMPARE(stream.lastPush[2 * 4 + 1], int16_t(-32767));
	gen.stop();
	QTRY_VERIFY(!gen.isRunning());
	QVERIFY(!stream.isOpen());
}

void TST_StreamGeneration::cyclicChangeReopens()
{
	FakeOutputStream stream(1);
	StreamGeneration gen(&stream);
	gen.setSamples(16);
	gen.start();
	QTRY_COMPARE(stream.pushes, 1);
	QHash<QString, ToneSpec> tones = gen.tones();
	tones["voltage0"].amplitude = 0.1;
	gen.setTones(tones);
	QTRY_COMPARE(stream.pushes, 2);
	QCOMPARE(stream.opens, 2);
	gen.shutdown();
	QVERIFY(!stream.isOpen());
}

void TST_StreamGeneration::streamingPushesInALoop()
{
	FakeOutputStream stream(1, false);
	StreamGeneration gen(&stream);
	QVERIFY(!gen.cyclic());
	gen.setSamples(16);
	gen.start();
	QTRY_VERIFY(stream.pushes >= 3);
	QVERIFY(!stream.lastPushCyclic);
	QCOMPARE(stream.opens, 1); // same buffer refilled, not reopened
	gen.stop();
	QTRY_VERIFY(!gen.isRunning());
}

void TST_StreamGeneration::disabledChannelsAreNotOpened()
{
	FakeOutputStream stream(3);
	StreamGeneration gen(&stream);
	QHash<QString, ToneSpec> tones = gen.tones();
	tones["voltage1"].enabled = false;
	gen.setTones(tones);
	gen.start();
	QTRY_COMPARE(stream.pushes, 1);
	QCOMPARE(stream.enabledChannels(), QList<int>({0, 2}));
	QCOMPARE(stream.writeFormat().channels.size(), 2);

	for(auto &t : tones) {
		t.enabled = false;
	}
	gen.stop();
	QTRY_VERIFY(!gen.isRunning());
	gen.setTones(tones);
	QSignalSpy errors(&gen, &StreamGeneration::errorOccurred);
	gen.start();
	QTRY_COMPARE(errors.size(), 1);
	QVERIFY(!gen.isRunning());
}

QTEST_GUILESS_MAIN(TST_StreamGeneration)
#include "tst_streamgeneration.moc"
