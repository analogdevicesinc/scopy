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

// Renders the command log pane (log + code generator) of the debugger tool to PNGs, for visual
// review: in the Scopy and Harmonic dark themes, with and without KDDockWidgets. Skipped unless
// DC_SCREENSHOT_DIR names the output folder.

#include "dcdebuggertool.h"
#include "log/codegeneratorpanel.h"
#include "log/commandlogmodel.h"
#include "log/commandlogview.h"

#include <QDir>
#include <QPushButton>
#include <QIcon>
#include <QItemSelectionModel>
#include <QSplitter>
#include <QTreeView>
#include <QtTest>

#include <gui/docking/docksettings.h>
#include <pluginbase/preferences.h>
#include <style.h>

using namespace scopy;
using namespace scopy::dcdebugger;

namespace {

constexpr qint64 T0 = 1'767'225'600'000'000'000LL;

CallRecord call(const QString &fn, const QString &attr, const QString &out, int rc)
{
	CallRecord c;
	c.library = QStringLiteral("libiio");
	c.function = fn;
	c.handle = 0xa000;
	c.args = {CallArg::string("attr", attr), CallArg::outLen("len", 4096)};
	c.rc = rc;
	c.out = out;
	c.outKind = CallArg::String;
	c.outSize = out.size();
	c.durationNs = 12000;
	return c;
}

CommandLogEntry entry(const QString &origin, const QList<CallRecord> &calls, int n)
{
	CommandLogEntry e;
	e.origin = origin;
	e.uri = QStringLiteral("ip:192.168.2.1");
	e.backendId = QStringLiteral("iio");
	e.commandClass = QStringLiteral("scopy::iio::AttrReadCommand");
	e.startNs = T0 + n * 1'500'000LL;
	e.durationNs = 30000;
	e.calls = calls;
	for(const CallRecord &c : calls) {
		if(c.rc < 0) {
			e.error = Error{c.rc, QStringLiteral("Connection timed out")};
		}
	}
	return e;
}

void fill(CommandLogModel *m)
{
	const QString gain = QStringLiteral("ctx/ad9361-phy/voltage0/hardwaregain");
	int n = 0;
	m->append(entry(gain, {call("iio_channel_attr_read", "hardwaregain", "71.000000 dB", 13)}, n++));
	m->append(entry(QStringLiteral("ctx/ad9361-phy/voltage0/sampling_frequency"),
			{call("iio_channel_attr_read", "sampling_frequency", "30720000", 9)}, n++));
	m->append(entry(QStringLiteral("ctx/ad9361-phy/rx_path_rates"),
			{call("iio_device_attr_read", "rx_path_rates", "BBPLL:983040000 ADC:245760000", 31),
			 call("iio_device_attr_read", "trx_rate_governor", "nominal", 8)},
			n++));
	m->append(entry(QStringLiteral("ctx/ad9361-phy/ensm_mode"),
			{call("iio_device_attr_write", "ensm_mode", "", -110)}, n++));
	for(int i = 0; i < 12; ++i) {
		m->append(entry(gain, {call("iio_channel_attr_read", "hardwaregain", "71.000000 dB", 13)}, n++));
	}
	m->append(entry(QStringLiteral("ctx/cf-ad9361-lpc/voltage0/en"), {call("iio_channel_attr_read", "en", "1", 2)},
			n++));
}

void initStyle(const QString &theme)
{
	Style *style = Style::GetInstance();
	style->init(theme, 1.0);
	QIcon::setThemeSearchPaths({":/gui/icons/scopy-" + theme});
}

} // namespace

class TST_LogScreenshots : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void initTestCase();
	void render_data();
	void render();

private:
	QString m_dir;
};

void TST_LogScreenshots::initTestCase()
{
	if(qEnvironmentVariableIsEmpty("DC_SCREENSHOT_DIR")) {
		QSKIP("DC_SCREENSHOT_DIR not set");
	}
	m_dir = QDir(qEnvironmentVariable("DC_SCREENSHOT_DIR")).absolutePath();
	CommandLogView::initPreferences();
	initDockWidgets();
	// Style::init finds the style in "./style" of the build folder.
	QDir::setCurrent(QStringLiteral(DCDEBUGGER_TEST_BUILD_DIR));
}

void TST_LogScreenshots::render_data()
{
	QTest::addColumn<QString>("theme");
	QTest::addColumn<bool>("docking");
	for(const QString &theme : {QStringLiteral("Scopy"), QStringLiteral("Harmonic dark")}) {
		for(bool docking : {false, true}) {
			const QString name = QString(theme).replace(' ', '_') + (docking ? "-kddw" : "-classic");
			QTest::newRow(qPrintable(name)) << theme << docking;
		}
	}
}

void TST_LogScreenshots::render()
{
	QFETCH(QString, theme);
	QFETCH(bool, docking);
	const QString &dir = m_dir;
	QDir().mkpath(dir);
	initStyle(theme);
	Preferences::set("general_use_docking_if_available", docking);

	auto *tool = new DCDebuggerTool(nullptr);
	tool->resize(1600, 1100);
	CommandLogView *log = tool->commandLog();
	fill(log->model());
	tool->show();
	QVERIFY(QTest::qWaitForWindowExposed(tool));
	tool->showTab(DCDebuggerTool::TraceTab);
	log->tree()->expand(log->model()->index(2, 0));
	log->tree()->selectionModel()->select(log->model()->index(1, 0),
					      QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
	log->tree()->selectionModel()->select(log->model()->index(4, 0),
					      QItemSelectionModel::Select | QItemSelectionModel::Rows);
	log->showCodeGenerator(true);
	QTest::qWait(300);
	QCoreApplication::processEvents();

	const QString tag = QString(QTest::currentDataTag());
	qInfo("SIZES %s generate=%dx%d pause=%dx%d copy=%dx%d save=%dx%d", qPrintable(tag),
	      log->generateButton()->width(), log->generateButton()->height(), log->pauseButton()->width(),
	      log->pauseButton()->height(), log->codeGenerator()->copyButton()->width(),
	      log->codeGenerator()->copyButton()->height(), log->codeGenerator()->saveButton()->width(),
	      log->codeGenerator()->saveButton()->height());
	QVERIFY(tool->grab().save(dir + "/tool-" + tag + ".png"));
	QVERIFY(tool->commandLog()->grab().save(dir + "/log-" + tag + ".png"));
	delete tool;
}

QTEST_MAIN(TST_LogScreenshots)
#include "tst_logscreenshots.moc"
