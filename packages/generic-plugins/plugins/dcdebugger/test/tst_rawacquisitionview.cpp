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

#include "acquisition/rawacquisitionview.h"
#include "fakestream.h"
#include "streamutils.h"

#include <QCheckBox>
#include <QPushButton>
#include <QTableWidget>
#include <QtTest>

#include <component/attributewriter.h>
#include <menuonoffswitch.h>
#include <menuspinbox.h>
#include <plotaxis.h>
#include <plotchannel.h>
#include <plotwidget.h>
#include <toolbuttons.h>

using namespace scopy;
using namespace scopy::component;
using namespace scopy::dcdebugger;
using namespace scopy::dcdebugger::test;

namespace {
// Writes into the reader's value, then reports success like a real writer.
class FakeAttributeWriter : public AttributeWriter
{
public:
	explicit FakeAttributeWriter(FakeAttributeReader *reader)
		: m_reader(reader)
	{}
	QCoro::Task<CommandResponse<void>> writeAsync(const QString &value) override
	{
		m_reader->value = value;
		writes++;
		Q_EMIT writeSucceeded();
		co_return CommandResponse<void>(QUuid());
	}
	int writes = 0;

private:
	FakeAttributeReader *m_reader;
};
} // namespace

// Simplified acquisition: all channels, raw values against the sample index, fixed X axis.
class TST_RawAcquisitionView : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void init();
	void cleanup();
	void allChannelsEnabled();
	void noMeasurementsTable();
	void xAxisSpansSamples();
	void singlePlotsRawAgainstSampleIndex();
	void yAutoscaleOffKeepsRange();
	void hidingACurveKeepsItAcquired();
	void sampleRateAppliedToEveryChannel();
	void noSampleRateWidgetWithoutAttribute();

private:
	void acquireOnce();

	FakeInputStream *m_stream = nullptr;
	RawAcquisitionView *m_view = nullptr;
};

void TST_RawAcquisitionView::init()
{
	m_stream = new FakeInputStream(3);
	m_view = new RawAcquisitionView(m_stream);
}

void TST_RawAcquisitionView::cleanup()
{
	delete m_view;
	m_view = nullptr;
	delete m_stream;
	m_stream = nullptr;
}

void TST_RawAcquisitionView::acquireOnce()
{
	auto *single = m_view->findChild<SingleShotBtn *>();
	QVERIFY(single);
	single->setChecked(true);
	QTRY_VERIFY(!single->isChecked());
}

void TST_RawAcquisitionView::allChannelsEnabled()
{
	QCOMPARE(m_view->acquisition()->enabledChannels(), QList<int>({0, 1, 2}));
	QCOMPARE(int(m_view->plot()->getChannels().size()), 3);
	for(PlotChannel *ch : m_view->plot()->getChannels()) {
		QVERIFY(ch->isEnabled());
	}
}

void TST_RawAcquisitionView::noMeasurementsTable() { QVERIFY(!m_view->findChild<QTableWidget *>()); }

void TST_RawAcquisitionView::xAxisSpansSamples()
{
	QCOMPARE(m_view->plot()->xAxis()->min(), 0.0);
	QCOMPARE(m_view->plot()->xAxis()->max(), 4096.0);
	auto *samples = m_view->findChild<gui::MenuSpinbox *>("rawAcquisitionSamples");
	QVERIFY(samples);
	samples->setValue(64);
	QCOMPARE(m_view->acquisition()->samples(), size_t(64));
	QCOMPARE(m_view->plot()->xAxis()->max(), 64.0);
}

void TST_RawAcquisitionView::singlePlotsRawAgainstSampleIndex()
{
	m_view->findChild<gui::MenuSpinbox *>("rawAcquisitionSamples")->setValue(16);
	acquireOnce();
	QCOMPARE(m_stream->refills, 1);
	PlotChannel *ch1 = m_view->plot()->getChannels().at(1);
	QCOMPARE(int(ch1->curve()->dataSize()), 16);
	QCOMPARE(ch1->curve()->sample(0), QPointF(0, 1000));   // channel 1, sample 0 = 1000
	QCOMPARE(ch1->curve()->sample(15), QPointF(15, 1015)); // unscaled
	// X stays fixed on the buffer, Y autoscaled around the data.
	QCOMPARE(m_view->plot()->xAxis()->max(), 16.0);
	QVERIFY(m_view->plot()->yAxis()->min() < 0 && m_view->plot()->yAxis()->max() > 2015);

	const QString csv = m_view->csvData();
	QVERIFY(csv.startsWith("sample,voltage0,voltage1,voltage2\n0,0,1000,2000\n"));
}

void TST_RawAcquisitionView::yAutoscaleOffKeepsRange()
{
	auto *autoscale = m_view->findChild<MenuOnOffSwitch *>("rawAcquisitionYAutoscale");
	QVERIFY(autoscale);
	autoscale->onOffswitch()->setChecked(false);
	m_view->plot()->yAxis()->setInterval(-5, 5);
	acquireOnce();
	QCOMPARE(m_view->plot()->yAxis()->min(), -5.0);
	QCOMPARE(m_view->plot()->yAxis()->max(), 5.0);
}

void TST_RawAcquisitionView::hidingACurveKeepsItAcquired()
{
	auto *box = m_view->findChild<QCheckBox *>("rawAcquisitionCurve:voltage1");
	QVERIFY(box);
	box->setChecked(false);
	QVERIFY(!m_view->plot()->getChannels().at(1)->isEnabled());
	QVERIFY(m_view->plot()->getChannels().at(0)->isEnabled());
	QCOMPARE(m_view->acquisition()->enabledChannels(), QList<int>({0, 1, 2}));
	acquireOnce();
	QCOMPARE(m_stream->enabledChannels(), QList<int>({0, 1, 2}));
	box->setChecked(true);
	QVERIFY(m_view->plot()->getChannels().at(1)->isEnabled());
}

void TST_RawAcquisitionView::sampleRateAppliedToEveryChannel()
{
	FakeInputStream stream(3);
	QList<FakeAttributeReader *> readers;
	QList<FakeAttributeWriter *> writers;
	for(Channel *ch : stream.channels()) {
		FakeAttributeReader *r = nullptr;
		Attribute *a = makeAttribute(ch, "sampling_frequency", "100", &r);
		auto *w = new FakeAttributeWriter(r);
		a->addWriteCapability(w);
		readers.append(r);
		writers.append(w);
	}
	QCOMPARE(memberSampleRates(&stream, false).size(), 3);

	RawAcquisitionView view(&stream);
	Attribute *first = memberSampleRates(&stream, false).first();
	first->writeCapability()->writeAsync("2000");
	QTRY_COMPARE(readers[1]->value, QStringLiteral("2000"));
	QTRY_COMPARE(readers[2]->value, QStringLiteral("2000"));
	QCOMPARE(writers[1]->writes, 1);
}

void TST_RawAcquisitionView::noSampleRateWidgetWithoutAttribute()
{
	QVERIFY(memberSampleRates(m_stream, false).isEmpty());
	QVERIFY(!m_view->findChild<QWidget *>("rawAcquisitionSampleRate"));
}

QTEST_MAIN(TST_RawAcquisitionView)
#include "tst_rawacquisitionview.moc"
