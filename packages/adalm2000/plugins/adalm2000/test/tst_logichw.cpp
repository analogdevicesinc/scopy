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

// Drives the Logic Analyzer end to end against a real board or iio-emu:
//   M2K_URI=usb:3.10.5 ./scopy-adalm2000_test_logichw
// Skipped when M2K_URI is unset, so a plain ctest run never waits on a network timeout.

#include "adalm2000logic_api.h"
#include "m2kcontext.h"
#include "m2kcontextfactory.h"
#include "m2klogiccontroller.h"
#include "m2klogicsource.h"

#include <core/acq_engine/acquisitionengine.h>
#include <core/acq_engine/datastore.h>
#include <core/acq_ui/acqinstrument.h>
#include <core/decoder/decodermanager.h>
#include <core/decoder/decoderpanel.h>

#include <component/attribute.h>
#include <component/attributereader.h>
#include <component/channel.h>
#include <component/device.h>
#include <component/navigation.h>

#include <QFile>
#include <QSettings>
#include <QTemporaryDir>
#include <QTest>

#include <pluginbase/preferences.h>
#include <qcoro/qcorotask.h>
#include <qwt_axis.h>

#include <memory>

using namespace scopy;
using namespace scopy::adalm2000;

class TST_LogicHw : public QObject
{
	Q_OBJECT

private:
	std::unique_ptr<M2kContext> m_ctx;
	QString m_uri;

	component::Device *digital() const
	{
		return m_ctx->findChild<component::Device *>(QStringLiteral("digital"), Qt::FindDirectChildrenOnly);
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

	static void capture(M2kLogicController &tool)
	{
		tool.single();
		QTRY_VERIFY_WITH_TIMEOUT(tool.ui()->engine()->isRunning(), 5000);
		QTRY_VERIFY_WITH_TIMEOUT(!tool.ui()->engine()->isRunning(), 10000);
	}

	static int samples(M2kLogicController &tool, int pin, int n)
	{
		return tool.ui()
			->store()
			->windowAs<QVector<quint8>>(scopy::acq::DataKey::raw("logic", M2kLogicSource::channelId(pin)),
						    n)
			.size();
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
		M2kContextFactory factory;
		m_ctx.reset(qobject_cast<M2kContext *>(factory.create(m_uri, 4)));
		QVERIFY2(m_ctx, qPrintable(QStringLiteral("could not open ") + m_uri));
	}

	void oneShotFillsEveryChannel()
	{
		M2kLogicController tool(nullptr, m_ctx.get(), m_uri);
		tool.init(m_ctx.get());
		Adalm2000Logic_API api(&tool);
		api.setSampleRate(1e6);
		api.setBufferSize(1000);
		capture(tool);
		QVERIFY(qAbs(api.sampleRate() - 1e6) < 1.0);
		QCOMPARE(read(digital(), "trigger_streaming_flag"), QStringLiteral("0"));
		for(int pin = 0; pin < M2kLogicController::CHANNELS; ++pin) {
			QCOMPARE(samples(tool, pin, 1000), 1000);
		}
	}

	void disabledChannelIsNotWritten()
	{
		M2kLogicController tool(nullptr, m_ctx.get(), m_uri);
		tool.init(m_ctx.get());
		tool.setChannelEnabled(3, false);
		capture(tool);
		QCOMPARE(samples(tool, 3, 1000), 0);
		QCOMPARE(samples(tool, 4, 1000), 1000);
	}

	void wideWindowStreams()
	{
		M2kLogicController tool(nullptr, m_ctx.get(), m_uri);
		tool.init(m_ctx.get());
		Adalm2000Logic_API api(&tool);
		api.setBufferSize(1000);
		api.setPlotWindow(5000);
		capture(tool);
		QCOMPARE(read(digital(), "trigger_streaming_flag"), QStringLiteral("1"));
		QCOMPARE(samples(tool, 0, 5000), 5000);
	}

	void triggerSettingsReachTheHardware()
	{
		M2kLogicController tool(nullptr, m_ctx.get(), m_uri);
		tool.init(m_ctx.get());
		Adalm2000Logic_API api(&tool);
		api.setTriggerLogic(1);
		api.setDelay(-100);
		tool.setTriggerCondition(2, 5);
		QTRY_COMPARE(
			read(component::channelById(digital(), QStringLiteral("voltage2"), false), "trigger_condition"),
			QStringLiteral("ANY_EDGE_DIGITAL"));
		QTRY_COMPARE(read(digital(), "trigger_mode"), QStringLiteral("DIO_AND"));
		QTRY_COMPARE(read(digital(), "trigger_delay"), QStringLiteral("-100"));
		tool.setTriggerCondition(2, 0);
		QTRY_COMPARE(
			read(component::channelById(digital(), QStringLiteral("voltage2"), false), "trigger_condition"),
			QStringLiteral("NO_TRIGGER_DIGITAL"));
		api.setTriggerLogic(0);
		api.setDelay(0);
	}

	void exportsCsvAndVcd()
	{
		M2kLogicController tool(nullptr, m_ctx.get(), m_uri);
		tool.init(m_ctx.get());
		capture(tool);
		QTemporaryDir dir;
		const QString csv = dir.filePath("out.csv");
		const QString vcd = dir.filePath("out.vcd");
		QVERIFY(tool.exportData(csv));
		QVERIFY(tool.exportData(vcd));
		QFile c(csv);
		QVERIFY(c.open(QIODevice::ReadOnly));
		QVERIFY(c.readLine().startsWith("Time (s),DIO 0,DIO 1"));
		QFile v(vcd);
		QVERIFY(v.open(QIODevice::ReadOnly));
		const QByteArray text = v.readAll();
		QVERIFY(text.contains("$var wire 1 ! DIO_0 $end"));
		QVERIFY(text.contains("$enddefinitions $end\n#0 "));
	}

	void decoderRunsOnThePins()
	{
		M2kLogicController tool(nullptr, m_ctx.get(), m_uri);
		tool.init(m_ctx.get());
		Adalm2000Logic_API api(&tool);
		const QString uid = api.addDecoder(QStringLiteral("uart"));
		if(uid.isEmpty()) {
			QSKIP("no uart decoder (sigrok-cli missing?)");
		}
		QVERIFY(api.setDecoderChannel(uid, QStringLiteral("rx"), 0));
		QVERIFY(api.setDecoderOption(uid, QStringLiteral("baudrate"), QStringLiteral("9600")));
		const QStringList saved = api.decoders();
		QCOMPARE(saved.size(), 1);
		QVERIFY(saved.first().contains(QStringLiteral("\"channels\":{\"rx\":0}")));
		QVERIFY(saved.first().contains(QStringLiteral("\"baudrate\":\"9600\"")));

		capture(tool);
		const scopy::decoder::DecoderInstance *d = tool.decoders()->manager()->find(uid);
		QVERIFY(d && !d->outKeys.isEmpty());
		QVERIFY(tool.ui()->store()->contains(d->outKeys.first()));

		api.setDecoders(saved);
		QCOMPARE(api.decoders(), saved);
		api.setDecoders({});
		QVERIFY(api.decoders().isEmpty());
	}

	void settingsRoundTrip()
	{
		QTemporaryDir dir;
		QSettings s(dir.filePath("logic.ini"), QSettings::IniFormat);
		{
			M2kLogicController tool(nullptr, m_ctx.get(), m_uri);
			tool.init(m_ctx.get());
			Adalm2000Logic_API api(&tool);
			api.setObjectName("logic");
			api.setBufferSize(2000);
			api.setPlotWindow(4000);
			tool.setChannelName(1, QStringLiteral("CLK"));
			tool.setChannelEnabled(7, false);
			tool.setTriggerCondition(1, 1);
			api.save(s);
			tool.setTriggerCondition(1, 0);
		}
		M2kLogicController tool(nullptr, m_ctx.get(), m_uri);
		tool.init(m_ctx.get());
		Adalm2000Logic_API api(&tool);
		api.setObjectName("logic");
		api.load(s);
		QCOMPARE(api.bufferSize(), 2000);
		QCOMPARE(api.plotWindow(), 4000);
		QCOMPARE(tool.channelName(1), QStringLiteral("CLK"));
		QVERIFY(!tool.isChannelEnabled(7));
		QCOMPARE(tool.triggerCondition(1), 1);
		tool.setTriggerCondition(1, 0);
	}
};

QTEST_MAIN(TST_LogicHw)
#include "tst_logichw.moc"
