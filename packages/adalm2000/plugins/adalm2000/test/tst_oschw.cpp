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
 */

// Drives the Oscilloscope end to end against a real board or iio-emu:
//   M2K_URI=usb:3.10.5 ./scopy-adalm2000_test_oschw
// Skipped when M2K_URI is unset, so a plain ctest run never waits on a network timeout.

#include "adalm2000osc_api.h"
#include "m2kcontext.h"
#include "m2kcontextfactory.h"
#include "m2kosccontroller.h"
#include "m2ksource.h"

#include <core/acq_engine/acquisitionengine.h>
#include <core/acq_engine/datastore.h>
#include <core/acq_ui/acqinstrument.h>

#include <component/attribute.h>
#include <component/attributereader.h>
#include <component/channel.h>
#include <component/controller.h>
#include <component/device.h>
#include <component/navigation.h>

#include <QFile>
#include <QSettings>
#include <QTemporaryDir>
#include <QTest>

#include <pluginbase/preferences.h>
#include <qcoro/qcorotask.h>
#include <qwt_axis.h>

#include <cmath>
#include <numeric>
#include <memory>

using namespace scopy;
using namespace scopy::adalm2000;
using scopy::acq::DataKey;

class TST_OscHw : public QObject
{
	Q_OBJECT

private:
	component::ContextHandle m_handle;
	M2kContext *m_ctx = nullptr;
	QString m_uri;

	component::Device *analogIn() const
	{
		return m_ctx->findChild<component::Device *>(QStringLiteral("analog-in"), Qt::FindDirectChildrenOnly);
	}

	component::Channel *channel(int ch) const
	{
		return component::channelById(analogIn(), M2kSource::channelId(M2kSource::Analog, ch), false);
	}

	QString read(QObject *node, const QString &name) const
	{
		component::Attribute *attr = component::attributeByName(node, name);
		if(!attr || !attr->readCapability()) {
			return QString();
		}
		const auto resp = QCoro::waitFor(attr->readCapability()->readAsync());
		return resp ? QString::fromUtf8(resp.value()) : QString();
	}

	static void capture(M2kOscController &tool)
	{
		tool.single();
		QTRY_VERIFY_WITH_TIMEOUT(tool.ui()->engine()->isRunning(), 5000);
		QTRY_VERIFY_WITH_TIMEOUT(!tool.ui()->engine()->isRunning(), 10000);
	}

	static QVector<float> latest(M2kOscController &tool, const DataKey &key)
	{
		return tool.ui()->store()->latestFloat(key).value_or(QVector<float>{});
	}

	static DataKey probeKey(int ch)
	{
		return DataKey::withStage(QStringLiteral("osc"), M2kSource::channelId(M2kSource::Analog, ch),
					  QStringLiteral("probe"));
	}

	static osc::Trigger freeRun()
	{
		osc::Trigger t;
		t.enabled = false;
		return t;
	}

private Q_SLOTS:
	void initTestCase()
	{
		m_uri = qEnvironmentVariable("M2K_URI");
		if(m_uri.isEmpty()) {
			QSKIP("M2K_URI not set");
		}
		Preferences *p = Preferences::GetInstance();
		p->init("general_plot_target_fps", "60");
		p->init("adc_plot_xaxis_label_position", QwtAxis::XBottom);
		p->init("adc_plot_yaxis_label_position", QwtAxis::YLeft);
		component::Controller::GetInstance()->registerBackend(std::make_shared<M2kContextFactory>());
		m_handle = component::Controller::connectCtx(m_uri, M2kContextFactory::ID);
		m_ctx = qobject_cast<M2kContext *>(m_handle.get());
		QVERIFY2(m_ctx, qPrintable(QStringLiteral("could not open ") + m_uri));
	}

	void singleFillsBothChannels()
	{
		M2kOscController tool(nullptr, m_ctx, m_uri);
		tool.init(m_ctx);
		tool.setTrigger(freeRun());
		sweep::Sweep s = tool.sweep();
		s.bufferSize = 4000;
		s.plotWindow = 4000;
		s.delay = -1000;
		tool.setSweep(s);
		capture(tool);
		QCOMPARE(latest(tool, probeKey(0)).size(), 4000);
		QCOMPARE(latest(tool, probeKey(1)).size(), 4000);
		const QVector<float> time = latest(tool, DataKey::raw(QStringLiteral("osc"), QStringLiteral("time")));
		QCOMPARE(time.size(), 4000);
		const double rate = tool.sweep().sampleRate;
		QVERIFY(qAbs(time.first() - (-1000 / rate)) < 1e-9);
		QCOMPARE(read(analogIn(), "trigger_streaming_flag"), QStringLiteral("0"));
	}

	void disabledChannelIsNotWritten()
	{
		M2kOscController tool(nullptr, m_ctx, m_uri);
		tool.init(m_ctx);
		tool.setTrigger(freeRun());
		tool.setChannelEnabled(1, false);
		capture(tool);
		QVERIFY(!latest(tool, probeKey(0)).isEmpty());
		QVERIFY(latest(tool, probeKey(1)).isEmpty());
	}

	void autoTriggerCapturesWithoutAnEdge()
	{
		M2kOscController tool(nullptr, m_ctx, m_uri);
		tool.init(m_ctx);
		osc::Trigger t;
		t.autoMode = true;
		t.level = 20.0;
		tool.setTrigger(t);
		capture(tool);
		QVERIFY(!latest(tool, probeKey(0)).isEmpty());
	}

	void triggerSettingsReachTheHardware()
	{
		M2kOscController tool(nullptr, m_ctx, m_uri);
		tool.init(m_ctx);
		osc::Trigger t;
		t.channel = 1;
		t.condition = 1;
		t.level = 0.5;
		t.hysteresis = 0.1;
		tool.setTrigger(t);
		capture(tool);
		QCOMPARE(read(analogIn(), "trigger_source"), QStringLiteral("CHANNEL_2"));
		QCOMPARE(read(channel(1), "trigger_mode"), QStringLiteral("ANALOG"));
		QCOMPARE(read(channel(0), "trigger_mode"), QStringLiteral("ALWAYS"));
		QCOMPARE(read(channel(1), "trigger_condition"), QStringLiteral("FALLING_EDGE_ANALOG"));
		QVERIFY(qAbs(read(channel(1), "trigger_level").toDouble() - 0.5) < 0.05);
	}

	void analogAndDigitalCombine()
	{
		M2kOscController tool(nullptr, m_ctx, m_uri);
		tool.init(m_ctx);
		osc::Trigger t;
		t.autoMode = true;
		t.digital = osc::DigitalExternal;
		t.logic = osc::LogicAnd;
		tool.setTrigger(t);
		if(tool.trigger().digital != osc::DigitalExternal) {
			QSKIP("no external trigger in");
		}
		capture(tool);
		QCOMPARE(read(channel(0), "trigger_mode"), QStringLiteral("DIGITAL_AND_ANALOG"));
		QCOMPARE(read(analogIn(), "trigger_source"), QStringLiteral("CHANNEL_1"));
	}

	void fractionalProbe()
	{
		M2kOscController tool(nullptr, m_ctx, m_uri);
		tool.init(m_ctx);
		Adalm2000Osc_API api(&tool);
		api.setProbe({0.1, 1.0});
		QCOMPARE(api.probe()[0], 0.1);
	}

	void probeScalesTheData()
	{
		M2kOscController tool(nullptr, m_ctx, m_uri);
		tool.init(m_ctx);
		tool.setTrigger(freeRun());
		tool.setProbe(0, 10.0);
		capture(tool);
		const QVector<float> raw =
			latest(tool, DataKey::raw(QStringLiteral("osc"), M2kSource::channelId(M2kSource::Analog, 0)));
		const QVector<float> scaled = latest(tool, probeKey(0));
		QCOMPARE(raw.size(), scaled.size());
		QVERIFY(!raw.isEmpty());
		QVERIFY(qAbs(scaled[10] - raw[10] * 10.0f) < 1e-4);
	}

	void smallVoltsPerDivPicksTheLowRange()
	{
		M2kOscController tool(nullptr, m_ctx, m_uri);
		tool.init(m_ctx);
		tool.setTrigger(freeRun());
		tool.setVoltsPerDiv(0, 0.1);
		QTest::qWait(400);
		capture(tool);
		QCOMPARE(read(channel(0), "range"), QStringLiteral("+/-2.5V"));
		tool.setVoltsPerDiv(0, 1.0);
		QTest::qWait(400);
		capture(tool);
		QCOMPARE(read(channel(0), "range"), QStringLiteral("+/-25V"));
	}

	void measurementsComeBackFromTheWorker()
	{
		M2kOscController tool(nullptr, m_ctx, m_uri);
		tool.init(m_ctx);
		tool.setTrigger(freeRun());
		tool.setMeasured(0, scopy::acq::Measure::Mean, true);
		tool.setStatsShown(0, scopy::acq::Measure::PeakPeak, true);
		QVERIFY(tool.isMeasured(0, scopy::acq::Measure::Mean));
		capture(tool);
		QTRY_VERIFY_WITH_TIMEOUT(!std::isnan(tool.measurement(0, scopy::acq::Measure::Mean)), 2000);
		QTRY_VERIFY(tool.statistics(0, scopy::acq::Measure::PeakPeak).value("count").toInt() >= 1);
		QVERIFY(std::isnan(tool.measurement(1, scopy::acq::Measure::Mean)));
		tool.setMeasured(0, scopy::acq::Measure::Mean, false);
		QVERIFY(!tool.isMeasured(0, scopy::acq::Measure::Mean));
	}

	void exportsCsv()
	{
		M2kOscController tool(nullptr, m_ctx, m_uri);
		tool.init(m_ctx);
		tool.setTrigger(freeRun());
		capture(tool);
		QTemporaryDir dir;
		const QString path = dir.filePath(QStringLiteral("osc.csv"));
		QVERIFY(tool.exportData(path));
		QFile f(path);
		QVERIFY(f.open(QIODevice::ReadOnly));
		const QStringList lines = QString::fromUtf8(f.readAll()).split('\n', Qt::SkipEmptyParts);
		QCOMPARE(lines.first(), QStringLiteral("Time (s),CH1 (V),CH2 (V)"));
		QCOMPARE(lines.size(), tool.sweep().bufferSize + 1);
	}

	void exportImportRoundTrip()
	{
		M2kOscController tool(nullptr, m_ctx, m_uri);
		tool.init(m_ctx);
		Adalm2000Osc_API api(&tool);
		tool.setTrigger(freeRun());
		capture(tool);
		QTemporaryDir dir;
		const QString path = dir.filePath(QStringLiteral("osc.txt"));
		QVERIFY(tool.exportData(path));
		QFile f(path);
		QVERIFY(f.open(QIODevice::ReadOnly));
		QCOMPARE(QString::fromUtf8(f.readLine()).trimmed(), QStringLiteral("Time (s)\tCH1 (V)\tCH2 (V)"));
		f.close();
		QCOMPARE(api.importReferences(path), 2);
		QCOMPARE(api.traceCount(), 4);
		QVERIFY(api.traceName(2).startsWith(QStringLiteral("osc-")));
	}

	void fftAndXYSources()
	{
		M2kOscController tool(nullptr, m_ctx, m_uri);
		tool.init(m_ctx);
		Adalm2000Osc_API api(&tool);
		tool.setTrigger(freeRun());
		api.setFftEnabled(true);
		api.addMath(QStringLiteral("X1 * 2"));
		api.setXYEnabled(true);
		api.setXYSources({0, 2});
		QCOMPARE(api.xySources(), (QList<int>{0, 2}));
		capture(tool);
		QCOMPARE(api.fft(0).size(), api.bufferSize() / 2 + 1);
		QCOMPARE(api.fftFrequencies(0).size(), api.fft(0).size());
		QCOMPARE(api.samples(0).size(), api.bufferSize());
	}

	void slowTimeBaseStreams()
	{
		M2kOscController tool(nullptr, m_ctx, m_uri);
		tool.init(m_ctx);
		Adalm2000Osc_API api(&tool);
		api.setTriggerEnabled(false);
		api.setTimeBase(0.1);
		QVERIFY(tool.sweep().plotWindow > tool.sweep().bufferSize);
		capture(tool);
		const QList<double> t = api.time();
		QCOMPARE(t.size(), api.bufferSize());
		const sweep::Sweep sw = tool.sweep();
		QVERIFY(qAbs(t.first() - (sw.plotWindow - sw.bufferSize) / sw.sampleRate) < 1e-6);
		api.setTimeBase(1e-3);
		QCOMPARE(tool.sweep().plotWindow, tool.sweep().bufferSize);
	}

	void scriptsSeeTheTool()
	{
		M2kOscController tool(nullptr, m_ctx, m_uri);
		tool.init(m_ctx);
		Adalm2000Osc_API api(&tool);
		api.setTriggerEnabled(false);
		api.setTimeBase(1e-3);
		QVERIFY(qAbs(api.timeBase() - 1e-3) < 1e-9);
		api.setMeasurements({QStringLiteral("0:rms")});
		QVERIFY(tool.isMeasured(0, scopy::acq::Measure::Rms));
		api.single();
		QTRY_VERIFY_WITH_TIMEOUT(api.running(), 5000);
		QTRY_VERIFY_WITH_TIMEOUT(!api.running(), 10000);
		QCOMPARE(api.samples(0).size(), api.bufferSize());
		QCOMPARE(api.time().size(), api.bufferSize());
		QTRY_VERIFY(!std::isnan(api.measurement(0, QStringLiteral("rms"))));
		QVERIFY(std::isnan(api.measurement(0, QStringLiteral("nope"))));
	}

	void mathAndReferences()
	{
		M2kOscController tool(nullptr, m_ctx, m_uri);
		tool.init(m_ctx);
		Adalm2000Osc_API api(&tool);
		api.setTriggerEnabled(false);
		QCOMPARE(api.addMath(QStringLiteral("X1 * 2")), 2);
		api.setMeasurements({QStringLiteral("2:mean")});
		capture(tool);
		const QList<double> ch1 = api.samples(0);
		const QVector<float> math = latest(tool, DataKey::raw(QStringLiteral("math"), QStringLiteral("math1")));
		QCOMPARE(math.size(), ch1.size());
		QVERIFY(qAbs(math[5] - 2 * ch1[5]) < 1e-4);
		QTRY_VERIFY(!std::isnan(api.measurement(2, QStringLiteral("mean"))));

		QCOMPARE(api.snapshot(0), 3);
		QCOMPARE(api.traceCount(), 4);
		QVERIFY(api.traceName(3).startsWith(QStringLiteral("CH1-REF")));
		capture(tool);
		QCOMPARE(api.math(), QStringList{QStringLiteral("X1 * 2")});
		api.removeTrace(3);
		api.removeTrace(2);
		QCOMPARE(api.traceCount(), 2);
		QVERIFY(api.measurements().isEmpty());
		capture(tool);
	}

	void histogramCountsEverySample()
	{
		M2kOscController tool(nullptr, m_ctx, m_uri);
		tool.init(m_ctx);
		Adalm2000Osc_API api(&tool);
		api.setTriggerEnabled(false);
		api.setVoltsPerDiv({10.0, 10.0});
		api.setHistogramEnabled(true);
		api.setXYEnabled(true);
		capture(tool);
		const QList<double> counts = api.histogram(0);
		QCOMPARE(counts.size(), 256);
		QCOMPARE(std::accumulate(counts.begin(), counts.end(), 0.0), double(api.bufferSize()));
		api.setHistogramEnabled(false);
		api.setXYEnabled(false);
	}

	void autosetEndsTriggered()
	{
		M2kOscController tool(nullptr, m_ctx, m_uri);
		tool.init(m_ctx);
		Adalm2000Osc_API api(&tool);
		api.setTriggerEnabled(false);
		api.autoset();
		QVERIFY(api.isAutosetting());
		QTRY_VERIFY_WITH_TIMEOUT(!api.isAutosetting(), 20000);
		QVERIFY(api.triggerEnabled());
		QVERIFY(api.triggerAuto());
		QVERIFY(api.voltsPerDiv()[0] >= 10e-3);
		QVERIFY(!api.running());
	}

	void timeBaseSamplesRaiseTheRate()
	{
		M2kOscController tool(nullptr, m_ctx, m_uri);
		tool.init(m_ctx);
		Adalm2000Osc_API api(&tool);
		api.setTimeBase(1e-3);
		const double rate = api.sampleRate();
		api.setTimeBaseSamples(1 << 16);
		QCOMPARE(api.timeBaseSamples(), 1 << 16);
		QVERIFY(api.sampleRate() > rate);
		QVERIFY(api.bufferSize() <= 1 << 16);
		QCOMPARE(api.timeBase(), 1e-3);
		api.setTimeBase(2e-3);
		QVERIFY(api.bufferSize() > 8192);
	}

	void settingsRoundTrip()
	{
		QTemporaryDir dir;
		QSettings s(dir.filePath("osc.ini"), QSettings::IniFormat);
		{
			M2kOscController tool(nullptr, m_ctx, m_uri);
			tool.init(m_ctx);
			Adalm2000Osc_API api(&tool);
			api.setObjectName("osc");
			api.setTimeBaseSamples(4096);
			api.setBufferSize(2000);
			api.setVoltsPerDiv({0.5, 2.0});
			api.setProbe({10.0, 1.0});
			api.setAcCoupled({false, true});
			api.setEnabled({true, false});
			api.setTriggerChannel(1);
			api.setTriggerLevel(1.5);
			api.setTriggerAuto(false);
			api.addMath(QStringLiteral("X1 - X2"));
			api.setMeasurements(
				{QStringLiteral("0:frequency"), QStringLiteral("1:mean"), QStringLiteral("2:rms")});
			api.setMeasurementStats({QStringLiteral("0:frequency")});
			api.save(s);
		}
		M2kOscController tool(nullptr, m_ctx, m_uri);
		tool.init(m_ctx);
		Adalm2000Osc_API api(&tool);
		api.setObjectName("osc");
		api.load(s);
		QCOMPARE(api.timeBaseSamples(), 4096);
		QCOMPARE(api.bufferSize(), 2000);
		QCOMPARE(api.voltsPerDiv(), (QList<double>{0.5, 2.0}));
		QCOMPARE(api.probe(), (QList<double>{10.0, 1.0}));
		QCOMPARE(api.acCoupled(), (QList<bool>{false, true}));
		QCOMPARE(api.enabled(), (QList<bool>{true, false}));
		QCOMPARE(api.triggerChannel(), 1);
		QCOMPARE(api.triggerLevel(), 1.5);
		QVERIFY(!api.triggerAuto());
		QCOMPARE(api.math(), QStringList{QStringLiteral("X1 - X2")});
		QCOMPARE(api.measurements(),
			 (QStringList{QStringLiteral("0:frequency"), QStringLiteral("1:mean"),
				      QStringLiteral("2:rms")}));
		QCOMPARE(api.measurementStats(), QStringList{QStringLiteral("0:frequency")});
	}
};

QTEST_MAIN(TST_OscHw)
#include "tst_oschw.moc"
