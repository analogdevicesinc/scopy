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

#include "bottompanehost.h"
#include "dcdebuggertool.h"
#include "log/commandlogview.h"

#include <QAbstractButton>
#include <QLabel>
#include <QSignalSpy>
#include <QSplitter>
#include <QtTest>

#include <component/connection.h>

using namespace scopy;
using namespace scopy::dcdebugger;

namespace {
constexpr const char *LOG = "log";
constexpr const char *OTHER = "other";

// A shown host with a main widget and two panes.
struct Host
{
	QLabel *main = new QLabel("main");
	QLabel *log = new QLabel("log pane");
	QLabel *other = new QLabel("other pane");
	BottomPaneHost host{main};

	Host()
	{
		host.addPane(LOG, "Log", log);
		host.addPane(OTHER, "Other", other);
		host.resize(800, 600);
		host.show();
	}

	int paneSize() const { return host.splitter()->sizes().value(1); }
	// The pane height the splitter can give at most (minus the handle).
	int total() const { return host.splitter()->height() - host.splitter()->handleWidth(); }
};
} // namespace

class TST_BottomPanes : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void startsHidden();
	void tabTogglesAndSwitches();
	void noCloseButton();
	void badgeAndIndicator();
	void duplicateIdRejected();
	void collapseUnchecksTab();
	void toolSplittersCollapse();
};

void TST_BottomPanes::startsHidden()
{
	Host h;
	QVERIFY(QTest::qWaitForWindowExposed(&h.host));
	QVERIFY(!h.host.isPaneOpen());
	QVERIFY(h.host.currentPane().isEmpty());
	QVERIFY(h.host.paneArea()->isHidden());
	QVERIFY(!h.host.tabButton(LOG)->isChecked());
	QVERIFY(!h.host.tabButton(OTHER)->isChecked());
	QVERIFY(h.host.bar()->isVisible());
	QCOMPARE(h.host.pane(LOG), h.log);
	QCOMPARE(h.host.tabButton(LOG)->text(), QStringLiteral("Log"));
}

void TST_BottomPanes::tabTogglesAndSwitches()
{
	Host h;
	QVERIFY(QTest::qWaitForWindowExposed(&h.host));
	QSignalSpy changed(&h.host, &BottomPaneHost::currentPaneChanged);
	QAbstractButton *logTab = h.host.tabButton(LOG);
	QAbstractButton *otherTab = h.host.tabButton(OTHER);

	logTab->click();
	QCOMPARE(h.host.currentPane(), QString(LOG));
	QVERIFY(logTab->isChecked());
	QVERIFY(h.host.paneArea()->isVisible());
	QVERIFY(h.log->isVisible());
	QVERIFY(h.paneSize() > 0);

	otherTab->click(); // another tab switches panes
	QCOMPARE(h.host.currentPane(), QString(OTHER));
	QVERIFY(!logTab->isChecked());
	QVERIFY(otherTab->isChecked());
	QVERIFY(h.other->isVisible());

	otherTab->click(); // a second click hides it
	QVERIFY(!h.host.isPaneOpen());
	QVERIFY(!otherTab->isChecked());
	QVERIFY(h.host.paneArea()->isHidden());
	QCOMPARE(changed.count(), 3);
	QVERIFY(changed.last()[0].toString().isEmpty());

	h.host.togglePane(LOG); // what Alt+1 does
	QVERIFY(logTab->isChecked());
	h.host.hidePane();
	QVERIFY(!logTab->isChecked());
}

void TST_BottomPanes::noCloseButton()
{
	Host h;
	h.host.showPane(LOG);
	// The bar holds the tabs only; the tab itself closes its pane.
	const QList<QAbstractButton *> buttons = h.host.bar()->findChildren<QAbstractButton *>();
	QCOMPARE(buttons.size(), 2);
	QVERIFY(buttons.contains(h.host.tabButton(LOG)));
	QVERIFY(buttons.contains(h.host.tabButton(OTHER)));
	QVERIFY(h.host.paneArea()->findChildren<QAbstractButton *>().isEmpty());
}

void TST_BottomPanes::badgeAndIndicator()
{
	Host h;
	QAbstractButton *tab = h.host.tabButton(LOG);
	h.host.setPaneBadge(LOG, "12");
	QVERIFY(tab->text().startsWith("Log"));
	QVERIFY(tab->text().endsWith("12"));
	h.host.setPaneBadge(LOG, QString());
	QCOMPARE(tab->text(), QStringLiteral("Log"));

	QVERIFY(!h.host.paneIndicator(LOG));
	h.host.setPaneIndicator(LOG, true, "Recording commands");
	QVERIFY(h.host.paneIndicator(LOG));
	QVERIFY(tab->toolTip().contains("Recording commands"));
	h.host.setPaneIndicator(LOG, false);
	QVERIFY(!h.host.paneIndicator(LOG));
	QVERIFY(!tab->toolTip().contains("Recording commands"));
	QVERIFY(!h.host.paneIndicator("missing"));
}

void TST_BottomPanes::duplicateIdRejected()
{
	Host h;
	auto *again = new QLabel("again");
	QVERIFY(!h.host.addPane(LOG, "Log 2", again));
	delete again; // not taken
	QCOMPARE(h.host.pane(LOG), h.log);
	QVERIFY(!h.host.tabButton("missing"));
	h.host.showPane("missing");
	QVERIFY(!h.host.isPaneOpen());
}

void TST_BottomPanes::collapseUnchecksTab()
{
	Host h;
	QVERIFY(QTest::qWaitForWindowExposed(&h.host));
	QVERIFY(h.host.splitter()->childrenCollapsible());
	QAbstractButton *tab = h.host.tabButton(LOG);
	tab->click();
	QVERIFY(h.host.isPaneOpen());

	// A drag to 150 px is remembered.
	h.host.splitter()->setSizes({h.total() - 150, 150});
	h.host.syncFromSplitter();
	QCOMPARE(h.paneSize(), 150);
	QCOMPARE(h.host.paneHeight(), 150);

	// Dragged fully down: the pane closes and its tab is unchecked; the height stays.
	h.host.splitter()->setSizes({h.total(), 0});
	QCOMPARE(h.paneSize(), 0);
	h.host.syncFromSplitter();
	QVERIFY(!h.host.isPaneOpen());
	QVERIFY(!tab->isChecked());
	QVERIFY(h.host.paneArea()->isHidden());
	QCOMPARE(h.host.paneHeight(), 150);

	// Reopening restores the last non-zero height.
	tab->click();
	QVERIFY(h.host.isPaneOpen());
	QVERIFY(tab->isChecked());
	QVERIFY(qAbs(h.paneSize() - 150) <= 2);

	// The main widget may be collapsed too; the pane stays open.
	h.host.splitter()->setSizes({0, h.total()});
	h.host.syncFromSplitter();
	QVERIFY(h.host.isPaneOpen());
	QCOMPARE(h.host.splitter()->sizes().value(0), 0);
}

void TST_BottomPanes::toolSplittersCollapse()
{
	CommandLogView::initPreferences();
	auto *conn = new component::Connection("fake:0", "fake");
	auto *tool = new DCDebuggerTool(conn);
	tool->resize(1200, 800);
	tool->show();
	QVERIFY(QTest::qWaitForWindowExposed(tool));

	// The log is a bottom pane, closed until its tab is clicked.
	BottomPaneHost *panes = tool->bottomPanes();
	QVERIFY(panes);
	QCOMPARE(panes->pane(DCDebuggerTool::LOG_PANE), tool->commandLog());
	QVERIFY(!panes->isPaneOpen());
	panes->tabButton(DCDebuggerTool::LOG_PANE)->click();
	QCOMPARE(panes->currentPane(), QString(DCDebuggerTool::LOG_PANE));

	// Every splitter of the debugger can be dragged fully closed.
	const QList<QSplitter *> splitters = tool->findChildren<QSplitter *>();
	QVERIFY(splitters.size() >= 2);
	for(QSplitter *s : splitters) {
		QVERIFY2(s->childrenCollapsible(), qPrintable(s->objectName()));
	}

	delete tool;
	delete conn;
}

QTEST_MAIN(TST_BottomPanes)
#include "tst_bottompanes.moc"
