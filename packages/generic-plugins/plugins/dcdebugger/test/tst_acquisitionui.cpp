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

#include <QComboBox>
#include <QLabel>
#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QTreeView>
#include <QtTest>

#include <component/connection.h>
#include <component/context.h>
#include <component/device.h>
#include <menucombo.h>
#include <menuonoffswitch.h>

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

private:
	QPushButton *button(const char *className) const;
	QComboBox *comboStartingWith(const QString &firstItem) const;
	QTableWidget *measurements() const;
	void selectStream();

	Connection *m_conn = nullptr;
	FakeInputStream *m_stream = nullptr;
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

void TST_AcquisitionUi::selectStream()
{
	auto *tree = m_tool->findChild<QTreeView *>();
	auto *model = m_tool->findChild<ComponentTreeModel *>();
	QVERIFY(tree && model);
	const QModelIndex src = model->indexOf(m_stream);
	QVERIFY(src.isValid());
	auto *proxy = qobject_cast<QAbstractProxyModel *>(tree->model());
	tree->scrollTo(proxy->mapFromSource(src));
	tree->setCurrentIndex(proxy->mapFromSource(src));
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

QTEST_MAIN(TST_AcquisitionUi)
#include "tst_acquisitionui.moc"
