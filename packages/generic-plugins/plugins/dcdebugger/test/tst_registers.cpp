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

#include "fakeregisters.h"
#include "registers/bittogglegrid.h"
#include "registers/registersession.h"
#include "registers/registerview.h"

#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalSpy>
#include <QTableWidget>
#include <QtTest>

#include <component/device.h>

using namespace scopy;
using namespace scopy::component;
using namespace scopy::dcdebugger;
using namespace scopy::dcdebugger::test;

class TST_Registers : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void init();
	void cleanup();
	void hexHelpers();
	void readAndWriteGoToHistory();
	void dumpKeepsOrderAndCounts();
	void failuresAreRecordedNotReported();
	void readOnlyTargetRefusesWrites();
	void viewReadsTogglesAndWrites();
	void viewDumpFillsTheTable();

private:
	QPushButton *button(QWidget *root, const QString &text) const;

	Device *m_dev = nullptr;
	FakeRegisterFile m_file;
	FakeRegisterReader *m_reader = nullptr;
	FakeRegisterWriter *m_writer = nullptr;
};

void TST_Registers::init()
{
	m_file = FakeRegisterFile{};
	m_dev = new Device();
	m_dev->setName("adc0");
	m_dev->setId("iio:device0");
	m_reader = new FakeRegisterReader(&m_file, 16, 16, m_dev);
	m_writer = new FakeRegisterWriter(&m_file, 16, 16, m_dev);
}

void TST_Registers::cleanup() { delete m_dev; }

QPushButton *TST_Registers::button(QWidget *root, const QString &text) const
{
	for(QPushButton *b : root->findChildren<QPushButton *>()) {
		if(b->text() == text) {
			return b;
		}
	}
	return nullptr;
}

void TST_Registers::hexHelpers()
{
	uint32_t v = 0;
	QVERIFY(RegisterSession::parseHex("0x1F", 8, v));
	QCOMPARE(v, 31u);
	QVERIFY(RegisterSession::parseHex("ff", 8, v));
	QCOMPARE(v, 255u);
	QVERIFY(!RegisterSession::parseHex("0x100", 8, v)); // over 8 bits
	QVERIFY(!RegisterSession::parseHex("0xZZ", 8, v));
	QVERIFY(!RegisterSession::parseHex("", 8, v));
	QVERIFY(RegisterSession::parseHex("FFFFFFFF", 32, v));
	QCOMPARE(v, 0xFFFFFFFFu);
	QCOMPARE(RegisterSession::hex(0xA5, 8), QString("0xA5"));
	QCOMPARE(RegisterSession::hex(0xA5, 16), QString("0x00A5"));
	QCOMPARE(RegisterSession::hex(0xA5, 32), QString("0x000000A5"));
	QCOMPARE(RegisterSession::mask(16), 0xFFFFu);
	QCOMPARE(RegisterSession::mask(32), 0xFFFFFFFFu);
}

void TST_Registers::readAndWriteGoToHistory()
{
	RegisterSession s;
	s.setTarget(m_reader, m_writer);
	QCOMPARE(s.addressBits(), 16);
	QCOMPARE(s.valueBits(), 16);
	QSignalSpy done(&s, &RegisterSession::readDone);
	QSignalSpy busy(&s, &RegisterSession::busyChanged);

	m_file.regs[0x40] = 0x1234;
	s.read(0x40);
	QTRY_COMPARE(done.count(), 1);
	QCOMPARE(done[0][0].toUInt(), 0x40u);
	QCOMPARE(done[0][1].toUInt(), 0x1234u);

	s.write(0x40, 0xA5);
	QTRY_COMPARE(m_file.regs.value(0x40), 0xA5u);
	QTRY_VERIFY(!s.isBusy());

	QCOMPARE(s.history().size(), 2);
	QCOMPARE(s.history()[0].op, QString("read"));
	QCOMPARE(s.history()[0].value, 0x1234u);
	QCOMPARE(s.history()[1].op, QString("write"));
	QCOMPARE(s.history()[1].address, 0x40u);
	QCOMPARE(s.history()[1].value, 0xA5u);
	QVERIFY(s.history()[1].ok);
	QVERIFY(busy.count() >= 2); // went busy and idle at least once
	s.clearHistory();
	QVERIFY(s.history().isEmpty());
}

void TST_Registers::dumpKeepsOrderAndCounts()
{
	RegisterSession s;
	s.setTarget(m_reader, m_writer);
	QSignalSpy done(&s, &RegisterSession::readDone);
	QSignalSpy finished(&s, &RegisterSession::dumpFinished);
	s.dump(0x10, 4);
	QTRY_COMPARE(finished.count(), 1);
	QCOMPARE(finished[0][0].toInt(), 4);
	QCOMPARE(finished[0][1].toInt(), 4);
	QCOMPARE(done.count(), 0); // dump rows are history only
	QCOMPARE(s.history().size(), 4);
	for(int i = 0; i < 4; ++i) {
		QCOMPARE(s.history()[i].address, 0x10u + i);
		QCOMPARE(s.history()[i].value, 0x10u + i); // unknown registers read as their address
	}
	QCOMPARE(m_file.reads, 4);
}

void TST_Registers::failuresAreRecordedNotReported()
{
	RegisterSession s;
	s.setTarget(m_reader, m_writer);
	QSignalSpy done(&s, &RegisterSession::readDone);
	QSignalSpy added(&s, &RegisterSession::entryAdded);
	m_file.fail = true;
	s.read(0x1);
	s.write(0x2, 0x3);
	QTRY_COMPARE(added.count(), 2);
	QCOMPARE(done.count(), 0);
	QVERIFY(!s.history()[0].ok);
	QVERIFY(s.history()[0].error.contains("fake read error"));
	QVERIFY(!s.history()[1].ok);
	QVERIFY(s.history()[1].error.contains("fake write error"));
	QVERIFY(!m_file.regs.contains(0x2));
}

void TST_Registers::readOnlyTargetRefusesWrites()
{
	RegisterSession s;
	s.setTarget(m_reader, nullptr);
	QVERIFY(s.canRead());
	QVERIFY(!s.canWrite());
	QSignalSpy added(&s, &RegisterSession::entryAdded);
	s.write(0x5, 0x6);
	QTRY_COMPARE(added.count(), 1);
	QVERIFY(!s.history()[0].ok);
	QVERIFY(s.history()[0].error.contains("no register writer"));
	QCOMPARE(m_file.writes, 0);
}

void TST_Registers::viewReadsTogglesAndWrites()
{
	RegisterView view(m_dev);
	view.show();
	QVERIFY(QTest::qWaitForWindowExposed(&view));
	auto *address = view.findChild<QLineEdit *>("registerAddress");
	auto *value = view.findChild<QLineEdit *>("registerValue");
	QVERIFY(address && value);
	// Width follows the leaf: 16 bits.
	QCOMPARE(view.bits()->width(), 16);
	QComboBox *width = nullptr;
	for(QComboBox *c : view.findChildren<QComboBox *>()) {
		if(c->findText("32") >= 0) {
			width = c;
		}
	}
	QVERIFY(width);
	QCOMPARE(width->currentText(), QString("16"));

	m_file.regs[0x40] = 0x00A5;
	address->setText("0x40");
	button(&view, "Read")->click();
	QTRY_COMPARE(value->text(), QString("0x00A5"));
	QCOMPARE(view.bits()->value(), 0xA5u);
	QVERIFY(view.bits()->bitButton(0)->isChecked());
	QVERIFY(!view.bits()->bitButton(1)->isChecked());

	// Toggle bit 1 (0xA5 -> 0xA7): only the field changes until Write.
	view.bits()->bitButton(1)->click();
	QCOMPARE(value->text(), QString("0x00A7"));
	QCOMPARE(m_file.regs.value(0x40), 0xA5u);
	button(&view, "Write")->click();
	QTRY_COMPARE(m_file.regs.value(0x40), 0xA7u);

	auto *table = view.findChild<QTableWidget *>();
	QTRY_COMPARE(table->rowCount(), 2);
	QCOMPARE(table->item(0, 1)->text(), QString("read"));
	QCOMPARE(table->item(0, 2)->text(), QString("0x0040"));
	QCOMPARE(table->item(0, 3)->text(), QString("0x00A5"));
	QCOMPARE(table->item(1, 1)->text(), QString("write"));
	QCOMPARE(table->item(1, 3)->text(), QString("0x00A7"));

	// A bad address is refused before anything is queued.
	address->setText("0x12345");
	button(&view, "Read")->click();
	QCOMPARE(m_file.reads, 1);

	button(&view, "Clear")->click();
	QCOMPARE(table->rowCount(), 0);
	QVERIFY(view.session()->history().isEmpty());
}

void TST_Registers::viewDumpFillsTheTable()
{
	RegisterView view(m_dev);
	view.show();
	QVERIFY(QTest::qWaitForWindowExposed(&view));
	auto *from = view.findChild<QLineEdit *>("dumpFrom");
	QVERIFY(from);
	from->setText("0x100");
	// Default count is 16.
	button(&view, "Dump")->click();
	auto *table = view.findChild<QTableWidget *>();
	QTRY_COMPARE(table->rowCount(), 16);
	QCOMPARE(table->item(0, 2)->text(), QString("0x0100"));
	QCOMPARE(table->item(15, 2)->text(), QString("0x010F"));
	QCOMPARE(table->item(15, 3)->text(), QString("0x010F"));
	QCOMPARE(m_file.reads, 16);
}

QTEST_MAIN(TST_Registers)
#include "tst_registers.moc"
