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

// Drives the Spectrum Analyzer end to end against a real board or iio-emu:
//   iio-emu adalm2000 &
//   M2K_URI=ip:127.0.0.1 ./scopy-adalm2000_test_spectrumhw
// Skipped when M2K_URI is unset, so a plain ctest run never waits on a network timeout.

#include "m2kspectrumcontroller.h"
#include "m2kanaloginsource.h"
#include "m2kcomponentbuilder.h"
#include "m2kcontext.h"
#include "m2kcontextfactory.h"
#include "spectrumsweep.h"

#include <core/acq_engine/acquisitionengine.h>
#include <core/acq_engine/datastore.h>
#include <core/acq_engine/snapshotsource.h>
#include <core/acq_ui/acqinstrument.h>
#include <core/acq_ui/acqplot.h>
#include <core/acq_ui/filesourceblock.h>

#include <component/attribute.h>
#include <component/attributereader.h>
#include <component/channel.h>
#include <component/controller.h>
#include <component/device.h>
#include <component/navigation.h>

#include <QDir>
#include <QTemporaryFile>
#include <QTest>

#include <gui/plotaxis.h>
#include <gui/waterfallplotwidget.h>
#include <pluginbase/preferences.h>
#include <qwt_axis.h>

#include <memory>

using namespace scopy;
using namespace scopy::adalm2000;
using scopy::acq::DataKey;

class TST_SpectrumHw : public QObject
{
	Q_OBJECT

private:
	component::ContextHandle m_handle;
	M2kContext *m_ctx = nullptr;
	QString m_uri;

	static int defaultFft() { return spec::fftSizes().at(spec::DEFAULT_FFT_INDEX); }

	static void runOnce(M2kSpectrumController &tool, int ch)
	{
		tool.ui()->store()->clear();
		tool.ui()->run();
		QTRY_VERIFY_WITH_TIMEOUT(
			tool.ui()->store()->latestFloat(M2kSpectrumController::spectrumKey(ch)).has_value(), 10000);
		tool.ui()->stop();
		QTRY_VERIFY_WITH_TIMEOUT(!tool.ui()->engine()->isRunning(), 5000);
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

	void realSpectraOnBothChannels()
	{
		M2kSpectrumController tool(nullptr, m_ctx, m_uri);
		tool.init(m_ctx);
		runOnce(tool, 1);
		const int bins = spec::bins(defaultFft());
		for(int ch = 0; ch < 2; ++ch) {
			const QVector<float> mag =
				*tool.ui()->store()->latestFloat(M2kSpectrumController::spectrumKey(ch));
			const QVector<float> f = *tool.ui()->store()->latestFloat(M2kSpectrumController::freqKey(ch));
			QCOMPARE(mag.size(), bins);
			QCOMPARE(f.size(), bins);
			// Real FFT: 0 .. fs/2, with fs the lowest rate reaching the default stop.
			QCOMPARE(f.first(), 0.0f);
			QVERIFY(qFuzzyCompare(f.last(),
					      static_cast<float>(spec::sampleRateFor(spec::DEFAULT_STOP_HZ) / 2)));
		}
	}

	void waterfallIsOffByDefault()
	{
		M2kSpectrumController tool(nullptr, m_ctx, m_uri);
		tool.init(m_ctx);
		tool.ui()->show();
		auto *wf = tool.ui()->findChild<WaterfallPlotWidget *>();
		QVERIFY(wf);
		QVERIFY(!wf->isVisible());
		runOnce(tool, 0);
		QVERIFY(tool.ui()->store()->depth(M2kSpectrumController::spectrumKey(0)) <= 2);
	}

	void startConfiguresAnalogIn()
	{
		auto *dev =
			m_ctx->findChild<component::Device *>(QStringLiteral("analog-in"), Qt::FindDirectChildrenOnly);
		auto attr = [dev](int ch, const char *name) {
			QObject *node = ch < 0 ? static_cast<QObject *>(dev)
					       : component::channelById(dev, M2kAnalogInSource::channelId(ch), false);
			return component::attributeByName(node, QString::fromLatin1(name));
		};
		auto read = [](component::Attribute *a) {
			const auto value = QCoro::waitFor(a->readCapability()->readAsync());
			return value ? QString::fromUtf8(value.value()).trimmed() : QString();
		};
		auto write = [](component::Attribute *a, const QString &v) {
			return bool(QCoro::waitFor(a->writeCapability()->writeAsync(v)));
		};

		// Left the way another tool would leave them: a trigger that never fires and the low range
		QVERIFY(attr(-1, "trigger_source") && attr(0, "trigger_mode") && attr(0, "range"));
		QVERIFY(write(attr(-1, "trigger_source"), QStringLiteral("CHANNEL_2")));
		for(int ch = 0; ch < 2; ++ch) {
			QVERIFY(write(attr(ch, "trigger_mode"), QStringLiteral("EXTERNAL")));
			QVERIFY(write(attr(ch, "range"), QLatin1String(PLUS_2_5V)));
		}

		M2kSpectrumController tool(nullptr, m_ctx, m_uri);
		tool.init(m_ctx);
		// Gets data only if start freed the trigger
		runOnce(tool, 0);

		// The trigger is put back, the range is the tool's own
		QCOMPARE(read(attr(-1, "trigger_source")), QStringLiteral("CHANNEL_2"));
		for(int ch = 0; ch < 2; ++ch) {
			QCOMPARE(read(attr(ch, "trigger_mode")), QStringLiteral("EXTERNAL"));
			QCOMPARE(read(attr(ch, "range")), QLatin1String(PLUS_25V));
		}
	}

	void waterfallXAxisFollowsTheSpan()
	{
		M2kSpectrumController tool(nullptr, m_ctx, m_uri);
		tool.init(m_ctx);
		Adalm2000Spectrum_API api(&tool);
		api.setWaterfall(true);
		api.setLogFreq(true);
		tool.ui()->run();
		QTRY_VERIFY_WITH_TIMEOUT(
			tool.ui()->store()->latestFloat(M2kSpectrumController::spectrumKey(0)).has_value(), 10000);

		// Changed while running: the FFT's bin range moves only once data at the new rate
		// arrives, which is when it used to take the waterfall axis back to 0..fs/2
		api.setStartFreq(1e3);
		api.setStopFreq(1e6);
		const double nyquist = spec::sampleRateFor(1e6) / 2.0;
		QTRY_VERIFY_WITH_TIMEOUT(
			qFuzzyCompare(tool.ui()->store()->latestFloat(M2kSpectrumController::freqKey(0))->constLast(),
				      static_cast<float>(nyquist)),
			10000);
		QTest::qWait(300);
		tool.ui()->stop();
		QTRY_VERIFY_WITH_TIMEOUT(!tool.ui()->engine()->isRunning(), 5000);

		auto *wf = tool.ui()->findChild<WaterfallPlotWidget *>();
		QVERIFY(wf);
		QVERIFY(wf->xAxis()->logScale());
		QCOMPARE(wf->xAxis()->max(), 1e6);
		QVERIFY(wf->xAxis()->min() > 0.0);
	}

	void importedFileIsAReference()
	{
		M2kSpectrumController tool(nullptr, m_ctx, m_uri);
		tool.init(m_ctx);
		QTemporaryFile csv(QDir::tempPath() + QStringLiteral("/spectrumXXXXXX.csv"));
		QVERIFY(csv.open());
		csv.write("Frequency (Hz),CH1 (dBFS)\n0,-100\n1000,-50\n2000,-80\n");
		csv.flush();

		auto *file = tool.ui()->engine()->findChild<adc::FileSourceBlock *>();
		auto *plot = tool.ui()->findChild<adc::AcqPlot *>();
		QVERIFY(file && plot);
		const int live = plot->channels().size();
		const int slot = file->addSlot();
		QVERIFY(file->setSlotFile(slot, csv.fileName()));
		QCOMPARE(plot->channels().size(), live + 1);
		const DataKey key = DataKey::raw(file->id(), file->enabledChannels().first());
		QCOMPARE(*tool.ui()->store()->latestFloat(key), (QVector<float>{-100, -50, -80}));

		file->removeSlot(slot);
		QCOMPARE(plot->channels().size(), live);
	}

	void channelToggleReachesTheSource()
	{
		M2kSpectrumController tool(nullptr, m_ctx, m_uri);
		tool.init(m_ctx);
		tool.setChannelEnabled(1, false);
		runOnce(tool, 0);
		QVERIFY(!tool.ui()->store()->latestFloat(
			DataKey::raw(QStringLiteral("m2k"), QStringLiteral("voltage1"))));
		QVERIFY(!tool.ui()->store()->latestFloat(M2kSpectrumController::spectrumKey(1)));
	}

	void snapshotIsAnEngineReference()
	{
		M2kSpectrumController tool(nullptr, m_ctx, m_uri);
		tool.init(m_ctx);
		runOnce(tool, 0);
		tool.snapshot(0);
		const DataKey ref = acq::SnapshotSource::keyFor(QStringLiteral("ref"), QStringLiteral("ref0"));
		const QVector<float> live = *tool.ui()->store()->latestFloat(M2kSpectrumController::spectrumKey(0));
		QCOMPARE(*tool.ui()->store()->latestFloat(ref), live);
		runOnce(tool, 0);
		QCOMPARE(*tool.ui()->store()->latestFloat(ref), live);
	}
};

QTEST_MAIN(TST_SpectrumHw)
#include "tst_spectrumhw.moc"
