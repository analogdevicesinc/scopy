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
#include "monitor/monitorview.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QMainWindow>
#include <QTabWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QTreeView>
#include <QtTest>

#include <component/connection.h>
#include <component/context.h>
#include <component/channel.h>
#include <component/device.h>
#include <menucombo.h>
#include <menucontrolbutton.h>
#include <menuspinbox.h>
#include <menuonoffswitch.h>
#include <plotwidget.h>

using namespace scopy;
using namespace scopy::component;
using namespace scopy::dcdebugger;
using namespace scopy::dcdebugger::test;

// Drives the real tool widget (tree + Acquisition tab) over a fake stream, inside a shown window.
class TST_AcquisitionUi : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void init();
	void cleanup();
	void runAfterFailedOpen();
	void disconnectWhileRunning();
	void viewModesFillTheMeasurements();
	void generatorStartsAndReappliesEdits();
	void monitorPollsEnabledChannels();

private:
	QPushButton *button(const char *className) const;
	QComboBox *comboStartingWith(const QString &firstItem) const;
	QTableWidget *measurements() const;
	void selectStream();
	void selectNode(QObject *node);

	Connection *m_conn = nullptr;
	FakeInputStream *m_stream = nullptr;
	FakeOutputStream *m_out = nullptr;
	Device *m_dev = nullptr;
	QMainWindow *m_win = nullptr;
	DCDebuggerTool *m_tool = nullptr;
};

void TST_AcquisitionUi::init()
{
	m_conn = new Connection("fake:0", "fake");
	auto *ctx = new Context();
	ctx->setName("ctx");
	m_conn->add("fake", ctx);
	auto *dev = new Device(ctx);
	dev->setName("adc");
	m_stream = new FakeInputStream(2);
	m_stream->setParent(dev);
	m_out = new FakeOutputStream(2);
	m_out->setParent(dev);
	m_dev = dev;
	auto *temp = new Channel(dev);
	temp->setId("temp0");
	makeAttribute(temp, "input", "25.5");
	auto *volt = new Channel(dev);
	volt->setId("voltage0");
	makeAttribute(volt, "raw", "100");
	makeAttribute(volt, "scale", "0.5");
	makeAttribute(volt, "offset", "0");

	m_win = new QMainWindow();
	m_win->resize(1400, 900);
	m_win->show();
	// Like the plugin: built without a parent, adopted by the tool stack later.
	m_tool = new DCDebuggerTool(m_conn);
	m_win->setCentralWidget(m_tool);
	QVERIFY(QTest::qWaitForWindowExposed(m_win));
}

void TST_AcquisitionUi::cleanup()
{
	if(m_tool) {
		m_tool->clear();
	}
	delete m_win;
	delete m_conn;
}

QPushButton *TST_AcquisitionUi::button(const char *className) const
{
	for(QPushButton *b : m_tool->findChildren<QPushButton *>()) {
		if(b->inherits(className) && b->isVisible()) {
			return b;
		}
	}
	return nullptr;
}

void TST_AcquisitionUi::selectStream() { selectNode(m_stream); }

void TST_AcquisitionUi::selectNode(QObject *node)
{
	auto *model = m_tool->findChild<ComponentTreeModel *>();
	QVERIFY(model);
	// The command log has a tree view too: take the one over the component tree.
	QTreeView *tree = nullptr;
	for(QTreeView *t : m_tool->findChildren<QTreeView *>()) {
		if(qobject_cast<QAbstractProxyModel *>(t->model())) {
			tree = t;
			break;
		}
	}
	QVERIFY(tree);
	const QModelIndex src = model->indexOf(node);
	QVERIFY(src.isValid());
	auto *proxy = qobject_cast<QAbstractProxyModel *>(tree->model());
	tree->scrollTo(proxy->mapFromSource(src));
	tree->setCurrentIndex(proxy->mapFromSource(src));
	// The panel opens on its Attributes tab: show the view (the first tab).
	for(QTabWidget *tabs : m_tool->findChildren<QTabWidget *>()) {
		if(tabs->isVisible()) {
			tabs->setCurrentIndex(0);
		}
	}
	QTest::qWait(50);
}

void TST_AcquisitionUi::runAfterFailedOpen()
{
	selectStream();
	QPushButton *run = button("scopy::RunBtn");
	QVERIFY(run);

	m_stream->failOpens = 1;
	run->setChecked(true);
	QTRY_VERIFY(!run->isChecked()); // failed open stops the run
	QVERIFY(!m_stream->isOpen());

	run->setChecked(true);
	QTRY_VERIFY(m_stream->refills >= 3);
	run->setChecked(false);
	QTRY_VERIFY(!m_stream->isOpen());
}

void TST_AcquisitionUi::disconnectWhileRunning()
{
	selectStream();
	QPushButton *run = button("scopy::RunBtn");
	QVERIFY(run);
	run->setChecked(true);
	QTRY_VERIFY(m_stream->refills >= 2);
	m_tool->clear();
	QVERIFY(!m_stream->isOpen());
}

QComboBox *TST_AcquisitionUi::comboStartingWith(const QString &firstItem) const
{
	for(QComboBox *c : m_tool->findChildren<QComboBox *>()) {
		if(c->count() > 0 && c->itemText(0) == firstItem) {
			return c;
		}
	}
	return nullptr;
}

QTableWidget *TST_AcquisitionUi::measurements() const
{
	for(QTableWidget *t : m_tool->findChildren<QTableWidget *>()) {
		if(t->columnCount() == 9 && t->isVisible()) {
			return t;
		}
	}
	return nullptr;
}

void TST_AcquisitionUi::viewModesFillTheMeasurements()
{
	selectStream();
	QPushButton *single = button("scopy::SingleShotBtn");
	QVERIFY(single);
	single->setChecked(true);
	QTRY_VERIFY(!single->isChecked());

	QTableWidget *table = measurements();
	QVERIFY(table);
	QCOMPARE(table->rowCount(), 2);
	QCOMPARE(table->item(0, 8)->text(), QString("—")); // peak only in spectrum mode

	QComboBox *mode = comboStartingWith("Time");
	QVERIFY(mode);
	mode->setCurrentIndex(1); // spectrum, re-rendered from the last buffer
	QCOMPARE(table->rowCount(), 2);
	QVERIFY(table->item(0, 8)->text().contains("@"));

	mode->setCurrentIndex(2); // XY: voltage1 (Y) against voltage0 (X), both in the table
	QCOMPARE(table->rowCount(), 2);
	QComboBox *xCh = nullptr;
	QComboBox *yCh = nullptr;
	for(MenuCombo *c : m_tool->findChildren<MenuCombo *>()) {
		if(c->title() == "X channel") {
			xCh = c->combo();
		} else if(c->title() == "Y channel") {
			yCh = c->combo();
		}
	}
	QVERIFY(xCh && yCh);
	QCOMPARE(xCh->currentIndex(), 0);
	QCOMPARE(yCh->currentIndex(), 1);
	yCh->setCurrentIndex(0); // same channel on both axes: one row
	QCOMPARE(table->rowCount(), 1);
	yCh->setCurrentIndex(1);

	QComboBox *units = comboStartingWith("Scaled");
	QVERIFY(units);
	units->setCurrentIndex(1);
	mode->setCurrentIndex(1);
	QVERIFY(table->item(0, 8)->text().contains("dBFS"));

	MenuOnOffSwitch *iq = nullptr;
	for(MenuOnOffSwitch *s : m_tool->findChildren<MenuOnOffSwitch *>()) {
		if(s->label()->text().startsWith("Complex I/Q")) {
			iq = s;
		}
	}
	QVERIFY(iq && iq->isEnabled());
	iq->onOffswitch()->setChecked(true); // voltage0 + j voltage1: one curve, both rows share the peak
	QCOMPARE(table->rowCount(), 2);
	QVERIFY(table->item(0, 8)->text().contains("(I/Q)"));
	QCOMPARE(table->item(0, 8)->text(), table->item(1, 8)->text());
	mode->setCurrentIndex(0);
	QVERIFY(!iq->isEnabled());
}

void TST_AcquisitionUi::generatorStartsAndReappliesEdits()
{
	selectNode(m_out);
	QPushButton *start = button("scopy::RunBtn");
	QVERIFY(start);
	QCOMPARE(start->text(), QString("Start"));
	start->setChecked(true);
	QTRY_COMPARE(m_out->pushes, 1);
	QVERIFY(m_out->lastPushCyclic);
	QCOMPARE(start->text(), QString("Stop"));

	// Editing a tone while running pushes a new cyclic buffer.
	gui::MenuSpinbox *amplitude = nullptr;
	for(gui::MenuSpinbox *sb : m_tool->findChildren<gui::MenuSpinbox *>()) {
		if(sb->name() == "Amplitude") {
			amplitude = sb;
			break;
		}
	}
	QVERIFY(amplitude);
	amplitude->setValue(0.25);
	QTRY_COMPARE(m_out->pushes, 2);

	// Unchecking a channel's checkbox in the channel list drops it from the stream.
	MenuControlButton *ch1 = nullptr;
	for(MenuControlButton *b : m_tool->findChildren<MenuControlButton *>()) {
		for(QLabel *l : b->findChildren<QLabel *>()) {
			if(l->text() == "voltage1" && b->isVisible()) {
				ch1 = b;
			}
		}
	}
	QVERIFY(ch1);
	ch1->checkBox()->setChecked(false);
	QTRY_COMPARE(m_out->enabledChannels(), QList<int>({0}));

	start->setChecked(false);
	QTRY_VERIFY(!m_out->isOpen());
	QCOMPARE(start->text(), QString("Start"));
}

void TST_AcquisitionUi::monitorPollsEnabledChannels()
{
	selectNode(m_dev);
	QPushButton *start = button("scopy::RunBtn");
	QVERIFY(start);
	QTableWidget *table = nullptr;
	for(QTableWidget *t : m_tool->findChildren<QTableWidget *>()) {
		if(t->columnCount() == 5 && t->isVisible()) {
			table = t;
		}
	}
	QVERIFY(table);
	QCOMPARE(table->rowCount(), 0);
	// One plot from the start, shown before anything is polled.
	auto *view = m_tool->findChild<MonitorView *>();
	QVERIFY(view);
	QCOMPARE(view->plotCount(), 1);
	QTRY_VERIFY(view->plot(0)->isVisible());

	// Enable both channels from their checkboxes in the channel list.
	int enabled = 0;
	for(MenuControlButton *b : m_tool->findChildren<MenuControlButton *>()) {
		for(QLabel *l : b->findChildren<QLabel *>()) {
			if(b->isVisible() && (l->text() == "temp0" || l->text() == "voltage0")) {
				b->checkBox()->setChecked(true);
				enabled++;
			}
		}
	}
	QCOMPARE(enabled, 2);
	start->setChecked(true);
	QTRY_COMPARE(table->rowCount(), 2);
	QTRY_VERIFY(table->item(0, 1)->text().startsWith("25.5"));
	QTRY_VERIFY(table->item(1, 1)->text().startsWith("50")); // raw 100 * scale 0.5
	start->setChecked(false);
}

QTEST_MAIN(TST_AcquisitionUi)
#include "tst_acquisitionui.moc"
