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

#include "componenttreemodel.h"
#include "dcdebuggertool.h"
#include "fakestream.h"
#include "log/commandlogview.h"
#include "nodepanel.h"
#include "readpolicy.h"

#include <QAbstractProxyModel>
#include <QLineEdit>
#include <QMainWindow>
#include <QTreeView>
#include <QtTest>

#include <component/attribute.h>
#include <component/commandmonitor.h>
#include <component/connection.h>
#include <component/context.h>
#include <component/device.h>
#include <iiowidget.h>
#include <menusectionwidget.h>
#include <pluginbase/preferences.h>
#include <widgets/animatedrefreshbtn.h>

using namespace scopy;
using namespace scopy::component;
using namespace scopy::dcdebugger;
using namespace scopy::dcdebugger::test;

class TST_NodePanel : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void init();
	void cleanup();
	void generalInfoStartsExpanded();
	void cachedAttributeIsNotReadOnSelection_data();
	void cachedAttributeIsNotReadOnSelection();
	void cachedValueIsShown();
	void uncachedAttributeIsAutoReadOnce();
	void failedAutoReadIsNotRetried();
	void refreshButtonAlwaysReads();
	void readButtonOfTheWidgetReads();
	void everySelectionReadsEachTime();
	void treeSelectionFollowsThePolicy_data();
	void treeSelectionFollowsThePolicy();
	void preferenceChangeAppliesOnNextSelection();
	void logNavigationDoesNotRead_data();
	void logNavigationDoesNotRead();

private:
	static void setMode(const char *mode) { Preferences::set(ReadPolicy::PREF, QString::fromLatin1(mode)); }
	int totalReads() const { return m_ra->reads + m_rb->reads; }
	NodePanel *devicePanel() { return createNodePanel(m_dev, NodeKind::Device, "dev", {}, {}); }
	void showTool();
	void selectNode(QObject *node);
	QObject *currentNode() const;
	QTreeView *tree() const;

	Connection *m_conn = nullptr;
	Context *m_ctx = nullptr;
	Device *m_dev = nullptr;
	// "a" holds a cached value, "b" has none.
	Attribute *m_a = nullptr;
	Attribute *m_b = nullptr;
	FakeAttributeReader *m_ra = nullptr;
	FakeAttributeReader *m_rb = nullptr;
	QMainWindow *m_win = nullptr;
	DCDebuggerTool *m_tool = nullptr;
};

void TST_NodePanel::init()
{
	Preferences::init(ReadPolicy::PREF, QString::fromLatin1(ReadPolicy::ONLY_UNCACHED));
	setMode(ReadPolicy::ONLY_UNCACHED);
	// The widgets must not read on their own, whatever the lazy loading preference says.
	Preferences::set("iiowidgets_use_lazy_loading", false);

	m_conn = new Connection("fake:0", "fake");
	m_ctx = new Context();
	m_ctx->setName("ctx");
	m_conn->add("fake", m_ctx);
	m_dev = new Device(m_ctx);
	m_dev->setName("dev");
	m_a = makeAttribute(m_dev, "a", "10", &m_ra);
	m_a->setCachedValue("cached");
	m_b = makeAttribute(m_dev, "b", "20", &m_rb);
}

void TST_NodePanel::cleanup()
{
	if(m_tool) {
		m_tool->clear();
	}
	delete m_win;
	m_win = nullptr;
	m_tool = nullptr;
	delete m_conn;
	m_conn = nullptr;
}

void TST_NodePanel::showTool()
{
	m_win = new QMainWindow();
	m_win->resize(1400, 900);
	m_win->show();
	m_tool = new DCDebuggerTool(m_conn);
	m_win->setCentralWidget(m_tool);
	QVERIFY(QTest::qWaitForWindowExposed(m_win));
}

QTreeView *TST_NodePanel::tree() const
{
	// The command log has a tree view too: take the one over the component tree.
	for(QTreeView *t : m_tool->findChildren<QTreeView *>()) {
		if(qobject_cast<QAbstractProxyModel *>(t->model())) {
			return t;
		}
	}
	return nullptr;
}

void TST_NodePanel::selectNode(QObject *node)
{
	auto *model = m_tool->findChild<ComponentTreeModel *>();
	auto *proxy = qobject_cast<QAbstractProxyModel *>(tree()->model());
	const QModelIndex idx = proxy->mapFromSource(model->indexOf(node));
	QVERIFY(idx.isValid());
	tree()->scrollTo(idx);
	tree()->setCurrentIndex(idx);
	QTest::qWait(20);
}

QObject *TST_NodePanel::currentNode() const
{
	auto *model = m_tool->findChild<ComponentTreeModel *>();
	auto *proxy = qobject_cast<QAbstractProxyModel *>(tree()->model());
	return model->objectAt(proxy->mapToSource(tree()->currentIndex()));
}

void TST_NodePanel::generalInfoStartsExpanded()
{
	// A node with attributes used to get its "General info" collapsed.
	NodePanel *panel = devicePanel();
	// "Attributes (2)" and "General info".
	const auto sections = panel->findChildren<MenuSectionCollapseWidget *>();
	QCOMPARE(sections.size(), 2);
	for(MenuSectionCollapseWidget *s : sections) {
		QVERIFY(!s->collapsed());
	}
	delete panel;
}

void TST_NodePanel::cachedAttributeIsNotReadOnSelection_data()
{
	QTest::addColumn<bool>("lazyLoading");
	QTest::newRow("lazy loading off") << false;
	QTest::newRow("lazy loading on") << true;
}

void TST_NodePanel::cachedAttributeIsNotReadOnSelection()
{
	QFETCH(bool, lazyLoading);
	Preferences::set("iiowidgets_use_lazy_loading", lazyLoading);
	// Only "b" is uncached: showing the panel reads it, not "a".
	NodePanel *panel = devicePanel();
	QCOMPARE(totalReads(), 0); // building reads nothing
	panel->show();
	QVERIFY(QTest::qWaitForWindowExposed(panel));
	QCOMPARE(m_ra->reads, 0);
	QCOMPARE(m_rb->reads, 1);
	// The lazy loading preference is the user's again.
	QCOMPARE(Preferences::get("iiowidgets_use_lazy_loading").toBool(), lazyLoading);
	delete panel;
}

void TST_NodePanel::cachedValueIsShown()
{
	NodePanel *panel = devicePanel();
	m_b->setCachedValue("beta"); // nothing to read then
	panel->show();
	QVERIFY(QTest::qWaitForWindowExposed(panel));
	QCOMPARE(totalReads(), 0);
	QStringList texts;
	for(QLineEdit *e : panel->findChildren<QLineEdit *>()) {
		texts.append(e->text());
	}
	QVERIFY2(texts.contains("cached"), qPrintable(texts.join('|')));
	QVERIFY2(texts.contains("beta"), qPrintable(texts.join('|')));
	delete panel;
}

void TST_NodePanel::uncachedAttributeIsAutoReadOnce()
{
	NodePanel *panel = devicePanel();
	panel->show();
	QVERIFY(QTest::qWaitForWindowExposed(panel));
	QCOMPARE(m_rb->reads, 1);
	QCOMPARE(m_b->cachedValue(), QString("20"));
	// Selected again, and again in a new panel: cached now, nothing to read.
	panel->hide();
	panel->show();
	NodePanel *second = devicePanel();
	second->show();
	QCOMPARE(totalReads(), 1);
	delete second;
	delete panel;
}

void TST_NodePanel::failedAutoReadIsNotRetried()
{
	m_rb->fail = true;
	NodePanel *panel = devicePanel();
	panel->show();
	QVERIFY(QTest::qWaitForWindowExposed(panel));
	QCOMPARE(m_rb->reads, 1);
	QVERIFY(m_b->cachedValue().isEmpty());
	panel->hide();
	panel->show();
	NodePanel *second = devicePanel();
	second->show();
	QCOMPARE(m_rb->reads, 1); // never auto-read twice
	QCOMPARE(m_ra->reads, 0);
	delete second;
	delete panel;
}

void TST_NodePanel::refreshButtonAlwaysReads()
{
	NodePanel *panel = devicePanel();
	panel->show();
	QVERIFY(QTest::qWaitForWindowExposed(panel));
	QCOMPARE(totalReads(), 1);
	auto *refresh = panel->findChild<AnimatedRefreshBtn *>();
	QVERIFY(refresh);
	refresh->click();
	QCOMPARE(m_ra->reads, 1); // cached or not
	QCOMPARE(m_rb->reads, 2);
	refresh->click();
	QCOMPARE(m_ra->reads, 2);
	QCOMPARE(m_rb->reads, 3);
	delete panel;
}

void TST_NodePanel::readButtonOfTheWidgetReads()
{
	NodePanel *panel = devicePanel();
	panel->show();
	QVERIFY(QTest::qWaitForWindowExposed(panel));
	const auto widgets = panel->findChildren<IIOWidget *>();
	QCOMPARE(widgets.size(), 2);
	const int before = totalReads();
	for(IIOWidget *w : widgets) {
		w->readAsync(); // what the read button of the widget does
	}
	QCOMPARE(totalReads(), before + 2);
	delete panel;
}

void TST_NodePanel::everySelectionReadsEachTime()
{
	setMode(ReadPolicy::EVERY_SELECTION);
	NodePanel *panel = devicePanel();
	QCOMPARE(totalReads(), 0);
	panel->show();
	QVERIFY(QTest::qWaitForWindowExposed(panel));
	QCOMPARE(m_ra->reads, 1);
	QCOMPARE(m_rb->reads, 1);
	panel->hide();
	panel->show();
	QCOMPARE(m_ra->reads, 2);
	QCOMPARE(m_rb->reads, 2);
	delete panel;
}

void TST_NodePanel::treeSelectionFollowsThePolicy_data()
{
	QTest::addColumn<QString>("mode");
	QTest::addColumn<int>("selectA");
	QTest::addColumn<int>("selectB");
	QTest::addColumn<int>("expandA");
	QTest::addColumn<int>("expandB");
	QTest::newRow("only uncached") << QString(ReadPolicy::ONLY_UNCACHED) << 0 << 1 << 0 << 1;
	QTest::newRow("every selection") << QString(ReadPolicy::EVERY_SELECTION) << 1 << 1 << 2 << 2;
}

void TST_NodePanel::treeSelectionFollowsThePolicy()
{
	QFETCH(QString, mode);
	QFETCH(int, selectA);
	QFETCH(int, selectB);
	QFETCH(int, expandA);
	QFETCH(int, expandB);
	setMode(mode.toLatin1().constData());
	showTool();
	QCOMPARE(totalReads(), 0);

	selectNode(m_dev);
	QCOMPARE(m_ra->reads, selectA);
	QCOMPARE(m_rb->reads, selectB);

	// Opening the group of attributes follows the same policy.
	auto *model = m_tool->findChild<ComponentTreeModel *>();
	auto *proxy = qobject_cast<QAbstractProxyModel *>(tree()->model());
	const QModelIndex group = model->indexOf(m_a).parent();
	QVERIFY(model->isGroup(group));
	tree()->expand(proxy->mapFromSource(group));
	QCOMPARE(m_ra->reads, expandA);
	QCOMPARE(m_rb->reads, expandB);
}

void TST_NodePanel::preferenceChangeAppliesOnNextSelection()
{
	showTool();
	selectNode(m_dev);
	QCOMPARE(m_ra->reads, 0);
	QCOMPARE(m_rb->reads, 1);
	setMode(ReadPolicy::EVERY_SELECTION);
	selectNode(m_ctx);
	selectNode(m_dev);
	QCOMPARE(m_ra->reads, 1);
	QCOMPARE(m_rb->reads, 2);
	setMode(ReadPolicy::ONLY_UNCACHED);
	selectNode(m_ctx);
	selectNode(m_dev);
	QCOMPARE(m_ra->reads, 1);
	QCOMPARE(m_rb->reads, 2);
}

void TST_NodePanel::logNavigationDoesNotRead_data()
{
	QTest::addColumn<QString>("mode");
	QTest::addColumn<int>("afterA");
	QTest::addColumn<int>("afterB");
	// Selecting the device afterwards: the navigation did not use up the one automatic read of "b".
	QTest::newRow("only uncached") << QString(ReadPolicy::ONLY_UNCACHED) << 0 << 1;
	QTest::newRow("every selection") << QString(ReadPolicy::EVERY_SELECTION) << 1 << 1;
}

void TST_NodePanel::logNavigationDoesNotRead()
{
	QFETCH(QString, mode);
	QFETCH(int, afterA);
	QFETCH(int, afterB);
	setMode(mode.toLatin1().constData());
	showTool();
	selectNode(m_ctx);
	QCOMPARE(totalReads(), 0);

	// A click on a log row of "b": deep in a collapsed group, whose panel was never shown.
	const CommandOrigin o = CommandMonitor::originOf(m_b);
	Q_EMIT m_tool->commandLog()->nodeRequested(o.path, o.backendId, o.object);
	QTest::qWait(50);
	QCOMPARE(currentNode(), m_b);
	QCOMPARE(m_ra->reads, 0);
	QCOMPARE(m_rb->reads, 0);

	// The row of an attribute of the already shown panel's node, and of the device itself.
	const CommandOrigin d = CommandMonitor::originOf(m_dev);
	Q_EMIT m_tool->commandLog()->nodeRequested(d.path, d.backendId, d.object);
	QTest::qWait(50);
	QCOMPARE(currentNode(), m_dev);
	QCOMPARE(totalReads(), 0);

	// Normal selection reads again.
	selectNode(m_ctx);
	selectNode(m_dev);
	QCOMPARE(m_ra->reads, afterA);
	QCOMPARE(m_rb->reads, afterB);
}

QTEST_MAIN(TST_NodePanel)
#include "tst_nodepanel.moc"
