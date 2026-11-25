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
#include "generation/rawgeneratorview.h"

#include <QComboBox>
#include <QtTest>

#include <menuspinbox.h>
#include <plotaxis.h>
#include <plotchannel.h>
#include <plotwidget.h>
#include <verticalchannelmanager.h>

using namespace scopy;
using namespace scopy::component;
using namespace scopy::dcdebugger;
using namespace scopy::dcdebugger::test;

// Raw generator: per-channel tones in codes and periods per buffer, every setting in the menu.
class TST_RawGeneratorView : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void init();
	void cleanup();
	void rawToneInCodes();
	void everySettingInTheMenu();
	void previewSpansTheBuffer();
	void periodsCappedBelowNyquist();
	void pushesRawCodesPerChannel();
	void codesClampedToTheChannelRange();

private:
	gui::MenuSpinbox *spin(const QString &name) const { return m_view->findChild<gui::MenuSpinbox *>(name); }
	QComboBox *combo(const QString &name) const { return m_view->findChild<QComboBox *>(name); }
	int16_t pushed(int channel, int sample) const
	{
		return m_stream->lastPush.value(sample * m_stream->enabledChannels().size() + channel);
	}

	FakeOutputStream *m_stream = nullptr;
	RawGeneratorView *m_view = nullptr;
};

void TST_RawGeneratorView::init()
{
	m_stream = new FakeOutputStream(2);
	m_view = new RawGeneratorView(m_stream);
}

void TST_RawGeneratorView::cleanup()
{
	delete m_view;
	m_view = nullptr;
	delete m_stream;
	m_stream = nullptr;
}

void TST_RawGeneratorView::rawToneInCodes()
{
	ToneSpec t;
	t.waveform = Waveform::Sine;
	t.frequency = 1; // one period
	t.amplitude = 1000;
	t.offset = 10;
	const QVector<double> v = generateRawTone(t, 4);
	QCOMPARE(v.size(), 4);
	QCOMPARE(qRound(v[0]), 10);
	QCOMPARE(qRound(v[1]), 1010);
	QCOMPARE(qRound(v[3]), -990);
	QCOMPARE(maxPeriods(8192), 4095);
}

void TST_RawGeneratorView::everySettingInTheMenu()
{
	QVERIFY(m_view->generation()->rawMode());
	QVERIFY(!m_view->findChild<VerticalChannelManager *>()); // no channel list
	for(const QString id : {"voltage0", "voltage1"}) {
		QVERIFY(m_view->findChild<QWidget *>("rawGeneratorChannel:" + id));
		QVERIFY(combo("rawGeneratorWaveform:" + id));
		for(const QString field : {"Periods", "Amplitude", "Offset", "Phase"}) {
			QVERIFY2(spin("rawGenerator" + field + ":" + id), qPrintable(field));
		}
	}
	QCOMPARE(spin("rawGeneratorPeriods:voltage0")->value(), double(RawGeneratorView::DefaultPeriods));
	QCOMPARE(spin("rawGeneratorAmplitude:voltage1")->value(), RawGeneratorView::DefaultAmplitude);
	QCOMPARE(spin("rawGeneratorPhase:voltage1")->value(), 90.0); // I/Q default kept
}

void TST_RawGeneratorView::previewSpansTheBuffer()
{
	const double samples = double(m_view->generation()->samples());
	QCOMPARE(m_view->plot()->xAxis()->min(), 0.0);
	QCOMPARE(m_view->plot()->xAxis()->max(), samples);
	PlotChannel *ch0 = m_view->plot()->getChannels().at(0);
	QCOMPARE(int(ch0->curve()->dataSize()), int(samples));
	QVERIFY(m_view->plot()->yAxis()->max() > RawGeneratorView::DefaultAmplitude);
}

void TST_RawGeneratorView::periodsCappedBelowNyquist()
{
	spin("rawGeneratorSamples")->setValue(16);
	spin("rawGeneratorPeriods:voltage0")->setValue(100);
	m_view->apply();
	QCOMPARE(m_view->generation()->tones().value("voltage0").frequency, double(maxPeriods(16)));
	QCOMPARE(spin("rawGeneratorPeriods:voltage0")->value(), double(maxPeriods(16)));
}

void TST_RawGeneratorView::pushesRawCodesPerChannel()
{
	spin("rawGeneratorSamples")->setValue(16);
	combo("rawGeneratorWaveform:voltage0")->setCurrentIndex(int(Waveform::Constant));
	spin("rawGeneratorOffset:voltage0")->setValue(500);
	spin("rawGeneratorPeriods:voltage1")->setValue(1);
	spin("rawGeneratorPhase:voltage1")->setValue(0);
	spin("rawGeneratorAmplitude:voltage1")->setValue(1000);
	m_view->apply();
	m_view->generation()->start();
	QTRY_VERIFY(m_stream->pushes >= 1);
	QCOMPARE(m_stream->enabledChannels(), QList<int>({0, 1}));
	for(int s = 0; s < 16; ++s) {
		QCOMPARE(pushed(0, s), int16_t(500));
	}
	QCOMPARE(pushed(1, 0), int16_t(0));
	QCOMPARE(pushed(1, 4), int16_t(1000)); // a quarter period: the sine's peak
	QCOMPARE(pushed(1, 12), int16_t(-1000));
	m_view->generation()->stop();
}

void TST_RawGeneratorView::codesClampedToTheChannelRange()
{
	spin("rawGeneratorSamples")->setValue(16);
	combo("rawGeneratorWaveform:voltage0")->setCurrentIndex(int(Waveform::Constant));
	spin("rawGeneratorOffset:voltage0")->setValue(40000); // past a 16-bit signed channel
	m_view->apply();
	m_view->generation()->start();
	QTRY_VERIFY(m_stream->pushes >= 1);
	QCOMPARE(pushed(0, 0), int16_t(32767));
	m_view->generation()->stop();
}

QTEST_MAIN(TST_RawGeneratorView)
#include "tst_rawgeneratorview.moc"
