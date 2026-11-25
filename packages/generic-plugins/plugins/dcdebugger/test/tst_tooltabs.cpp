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

#include "dcdebuggertool.h"
#include "fakestream.h"
#include "log/commandlogview.h"
#include "readpolicy.h"

#include <QSplitter>
#include <QTabBar>
#include <QTabWidget>
#include <QTreeView>
#include <QtTest>

#include <component/commandmonitor.h>
#include <component/connection.h>
#include <component/context.h>
#include <component/device.h>
#include <pluginbase/preferences.h>

using namespace scopy;
using namespace scopy::component;
using namespace scopy::dcdebugger;
using namespace scopy::dcdebugger::test;

// The tool's two pages: Explore (tree + node panel) and Trace (command log + code generator).
class TST_ToolTabs : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void initTestCase();
	void init();
	void cleanup();
	void exploreAndTraceTabs();
	void splittersCollapse();
	void goToOriginStaysOnTraceWithoutReads();
	void traceTabHasNoCount();

private:
	Connection *m_conn = nullptr;
	Device *m_dev = nullptr;
	FakeAttributeReader *m_reader = nullptr;
	DCDebuggerTool *m_tool = nullptr;
};

void TST_ToolTabs::initTestCase()
{
	CommandLogView::initPreferences();
	ReadPolicy::initPreferences();
	Preferences::set(ReadPolicy::PREF, QString::fromLatin1(ReadPolicy::ONLY_UNCACHED));
	Preferences::set("general_use_docking_if_available", false);
}

void TST_ToolTabs::init()
{
	m_conn = new Connection("test:0", "test");
	auto *ctx = new Context();
	ctx->setName("ctx");
	ctx->setBackend("iio");
	m_conn->add("iio", ctx);
	m_dev = new Device(ctx);
	m_dev->setId("iio:device0");
	m_dev->setName("dev0");
	makeAttribute(m_dev, "sampling_frequency", "1000", &m_reader);

	m_tool = new DCDebuggerTool(m_conn);
	m_tool->resize(1200, 800);
	m_tool->show();
	QVERIFY(QTest::qWaitForWindowExposed(m_tool));
}

void TST_ToolTabs::cleanup()
{
	delete m_tool;
	m_tool = nullptr;
	delete m_conn;
	m_conn = nullptr;
}

void TST_ToolTabs::exploreAndTraceTabs()
{
	QTabWidget *tabs = m_tool->tabs();
	QVERIFY(tabs);
	QCOMPARE(tabs->count(), 2);
	QCOMPARE(tabs->tabText(DCDebuggerTool::ExploreTab), QStringLiteral("Explore"));
	QCOMPARE(tabs->tabText(DCDebuggerTool::TraceTab), QStringLiteral("Trace"));
	QCOMPARE(tabs->widget(DCDebuggerTool::TraceTab), static_cast<QWidget *>(m_tool->commandLog()));
	QVERIFY(tabs->widget(DCDebuggerTool::ExploreTab)->isAncestorOf(m_tool->findChild<QTreeView *>()));
	QCOMPARE(tabs->currentIndex(), int(DCDebuggerTool::ExploreTab));
	QCOMPARE(tabs->tabPosition(), QTabWidget::South);
	m_tool->showTab(DCDebuggerTool::TraceTab);
	QVERIFY(m_tool->commandLog()->isVisible());
}

void TST_ToolTabs::splittersCollapse()
{
	// Every splitter of the debugger can be dragged fully closed.
	const QList<QSplitter *> splitters = m_tool->findChildren<QSplitter *>();
	QVERIFY(!splitters.isEmpty());
	for(QSplitter *s : splitters) {
		QVERIFY2(s->childrenCollapsible(), qPrintable(s->objectName()));
	}
}

void TST_ToolTabs::goToOriginStaysOnTraceWithoutReads()
{
	m_tool->showTab(DCDebuggerTool::TraceTab);
	QTest::qWait(50);
	const int before = m_reader->reads;
	const CommandOrigin o = CommandMonitor::originOf(m_dev);
	Q_EMIT m_tool->commandLog()->nodeRequested(o.path, o.backendId, 0);
	QCoreApplication::processEvents();

	// Selected in the tree, but the log stays in front and nothing was read.
	QCOMPARE(m_tool->tabs()->currentIndex(), int(DCDebuggerTool::TraceTab));
	auto *tree = m_tool->findChild<QTreeView *>();
	QCOMPARE(tree->currentIndex().data().toString(), QStringLiteral("dev0"));
	QCOMPARE(m_reader->reads, before);

	// Going to Explore shows the panel, still without reading.
	m_tool->showTab(DCDebuggerTool::ExploreTab);
	QTest::qWait(50);
	QCOMPARE(m_reader->reads, before);
}

void TST_ToolTabs::traceTabHasNoCount()
{
	QTabWidget *tabs = m_tool->tabs();
	QVERIFY(tabs);
	QCOMPARE(tabs->tabText(DCDebuggerTool::TraceTab), QStringLiteral("Trace"));
}

QTEST_MAIN(TST_ToolTabs)
#include "tst_tooltabs.moc"
