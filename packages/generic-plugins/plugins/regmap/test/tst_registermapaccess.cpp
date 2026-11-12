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

#include "registermapaccess.hpp"
#include "registermapdatastore.hpp"

#include <QFile>
#include <QHash>
#include <QSet>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <qcoro/qcorotask.h>
#include <qcoro/qcorotimer.h>

#include <cerrno>
#include <chrono>

using namespace scopy;
using namespace scopy::component;
using namespace scopy::regmap;

namespace {
// A device in memory. Failing addresses answer -EIO; a delay keeps a call running.
struct FakeRegisters
{
	QHash<uint32_t, uint32_t> values;
	QSet<uint32_t> failing;
	std::chrono::milliseconds delay{0};
	QList<QVector<uint32_t>> reads; // the addresses of every read call
};

RegisterData answer(FakeRegisters *regs, uint32_t address)
{
	const QDateTime now = QDateTime::currentDateTime();
	if(regs->failing.contains(address)) {
		return RegisterData::failed(address, Error{-EIO, "bad address"}, now);
	}
	return RegisterData::ok(address, regs->values.value(address), now);
}

class FakeReadStrategy : public IRegisterReadStrategy
{
public:
	explicit FakeReadStrategy(FakeRegisters *regs)
		: m_regs(regs)
	{}
	QCoro::Task<RegisterData> read(RegisterData data) override
	{
		const QVector<RegisterData> input{data};
		QVector<RegisterData> results = co_await readRange(input);
		co_return results.first();
	}
	QCoro::Task<QVector<RegisterData>> readRange(QVector<RegisterData> data) override
	{
		FakeRegisters *regs = m_regs;
		QVector<uint32_t> addresses;
		for(const RegisterData &d : data) {
			addresses.append(d.address);
		}
		regs->reads.append(addresses);
		if(regs->delay.count()) {
			co_await QCoro::sleepFor(regs->delay);
		}
		QVector<RegisterData> results;
		for(uint32_t address : addresses) {
			results.append(answer(regs, address));
		}
		co_return results;
	}

private:
	FakeRegisters *m_regs;
};

class FakeWriteStrategy : public IRegisterWriteStrategy
{
public:
	explicit FakeWriteStrategy(FakeRegisters *regs)
		: m_regs(regs)
	{}
	QCoro::Task<RegisterData> write(RegisterData data) override
	{
		const QVector<RegisterData> input{data};
		QVector<RegisterData> results = co_await writeRange(input);
		co_return results.first();
	}
	QCoro::Task<QVector<RegisterData>> writeRange(QVector<RegisterData> data) override
	{
		const QDateTime now = QDateTime::currentDateTime();
		QVector<RegisterData> results;
		for(const RegisterData &d : data) {
			if(m_regs->failing.contains(d.address)) {
				results.append(RegisterData::failed(d.address, Error{-EIO, "bad address"}, now));
			} else {
				m_regs->values.insert(d.address, *d.value);
				results.append(RegisterData::ok(d.address, *d.value, now));
			}
		}
		co_return results;
	}

private:
	FakeRegisters *m_regs;
};
} // namespace

class TstRegisterMapAccess : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void rangeRefreshesOnce();
	void errorIsStored();
	void pendingRegisterIsSkipped();
	void writeIsReadBack();
	void failedWriteRestores();
	void partialWriteFromFile();
	void mixedPairHasNoReadBack();
	void dumpWritesOkEntries();
	void noStrategy();
};

void TstRegisterMapAccess::rangeRefreshesOnce()
{
	FakeRegisters regs;
	FakeReadStrategy reader(&regs);
	FakeWriteStrategy writer(&regs);
	RegisterMapAccess access(&reader, &writer, true, "dev");

	QStringList events;
	connect(access.store(), &RegisterMapDataStore::dataChanged, this,
		[&events](QList<uint32_t> a) { events.append(QString("data %1").arg(a.size())); });
	connect(access.store(), &RegisterMapDataStore::pendingChanged, this,
		[&events](QList<uint32_t> a, bool p) { events.append(QString("pending %1 %2").arg(a.size()).arg(p)); });

	const QVector<RegisterData> results = QCoro::waitFor(access.readRange(0x0, 0xff));
	QCOMPARE(results.size(), 256);
	// one strategy call, one refresh, the value before the buttons
	QCOMPARE(regs.reads.size(), 1);
	QCOMPARE(events, (QStringList{"pending 256 1", "data 256", "pending 256 0"}));
}

void TstRegisterMapAccess::errorIsStored()
{
	FakeRegisters regs;
	regs.values.insert(0x1, 0x11);
	regs.failing.insert(0x2);
	FakeReadStrategy reader(&regs);
	RegisterMapAccess access(&reader, nullptr, true, "dev");

	QCoro::waitFor(access.readRange(0x1, 0x3));
	const RegisterData *ok = access.store()->get(0x1);
	QVERIFY(ok && ok->isOk());
	QCOMPARE(*ok->value, 0x11u);
	const RegisterData *err = access.store()->get(0x2);
	QVERIFY(err && err->isError());
	QVERIFY(!err->value);
	QCOMPARE(err->error->code, -EIO);
	QVERIFY(err->completedAt.isValid());
	QVERIFY(!access.store()->isPending(0x2));
}

void TstRegisterMapAccess::pendingRegisterIsSkipped()
{
	FakeRegisters regs;
	regs.values.insert(0x1, 0x11);
	regs.delay = std::chrono::milliseconds(50);
	FakeReadStrategy reader(&regs);
	RegisterMapAccess access(&reader, nullptr, true, "dev");

	// QCoro tasks start right away: the first read is running
	QCoro::Task<RegisterData> running = access.read(0x1);
	QVERIFY(access.store()->isPending(0x1));
	// a pending entry with no earlier value has no value
	QVERIFY(!access.store()->get(0x1)->value);

	const RegisterData skipped = QCoro::waitFor(access.read(0x1));
	QVERIFY(skipped.isError());
	QCOMPARE(skipped.error->code, -EBUSY);
	// the JS path waits for it, then runs
	const RegisterData waited = QCoro::waitFor(access.readWhenIdle(0x1));
	QVERIFY(waited.isOk());
	QVERIFY(QCoro::waitFor(running).isOk());
	QCOMPARE(regs.reads.size(), 2);
	QVERIFY(!access.store()->isPending(0x1));
}

void TstRegisterMapAccess::writeIsReadBack()
{
	FakeRegisters regs;
	FakeReadStrategy reader(&regs);
	FakeWriteStrategy writer(&regs);
	RegisterMapAccess access(&reader, &writer, true, "dev");

	const RegisterData w = QCoro::waitFor(access.writeThenRead(0x5, 0x33));
	QVERIFY(w.isOk());
	QCOMPARE(regs.reads.size(), 1);
	QCOMPARE(*access.store()->get(0x5)->value, 0x33u);
	QVERIFY(!access.store()->isPending(0x5));
}

void TstRegisterMapAccess::failedWriteRestores()
{
	FakeRegisters regs;
	regs.values.insert(0x9, 0x1);
	FakeReadStrategy reader(&regs);
	FakeWriteStrategy writer(&regs);
	RegisterMapAccess access(&reader, &writer, true, "dev");
	QCoro::waitFor(access.read(0x9));

	regs.failing.insert(0x9);
	QSignalSpy data(access.store(), &RegisterMapDataStore::dataChanged);
	const RegisterData w = QCoro::waitFor(access.writeThenRead(0x9, 0x2));
	QVERIFY(w.isError());
	// no read-back, the entry is as before
	QCOMPARE(data.count(), 0);
	QCOMPARE(regs.reads.size(), 1);
	const RegisterData *entry = access.store()->get(0x9);
	QVERIFY(entry->isOk());
	QCOMPARE(*entry->value, 0x1u);
	QVERIFY(!access.store()->isPending(0x9));
}

void TstRegisterMapAccess::partialWriteFromFile()
{
	QTemporaryDir dir;
	const QString path = dir.filePath("values.csv");
	QFile file(path);
	QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
	file.write("0x5,0xc\n0x2,0xb\n0x1,0xa\n");
	file.close();

	FakeRegisters regs;
	regs.failing.insert(0x2);
	FakeReadStrategy reader(&regs);
	FakeWriteStrategy writer(&regs);
	RegisterMapAccess access(&reader, &writer, true, "dev");
	QSignalSpy data(access.store(), &RegisterMapDataStore::dataChanged);

	QCoro::waitFor(access.writeFromFile(path));
	// one read-back, of the registers written Ok, in file order
	QCOMPARE(regs.reads, (QList<QVector<uint32_t>>{{0x5, 0x1}}));
	QCOMPARE(data.count(), 1);
	QCOMPARE(*access.store()->get(0x5)->value, 0xcu);
	QCOMPARE(*access.store()->get(0x1)->value, 0xau);
	// the failed write goes back to never read
	QVERIFY(!access.store()->get(0x2));
	QVERIFY(!access.store()->isAnyPending({0x1, 0x2, 0x5}));
}

void TstRegisterMapAccess::mixedPairHasNoReadBack()
{
	FakeRegisters regs;
	FakeReadStrategy reader(&regs);
	FakeWriteStrategy writer(&regs);
	RegisterMapAccess access(&reader, &writer, false, "dev");

	const RegisterData w = QCoro::waitFor(access.writeThenRead(0x5, 0x33));
	QVERIFY(w.isOk());
	QCOMPARE(regs.reads.size(), 0);
	QVERIFY(!access.store()->get(0x5));
	QVERIFY(!access.store()->isPending(0x5));
}

void TstRegisterMapAccess::dumpWritesOkEntries()
{
	FakeRegisters regs;
	regs.values.insert(0x1, 0xab);
	regs.failing.insert(0x2);
	FakeReadStrategy reader(&regs);
	RegisterMapAccess access(&reader, nullptr, true, "dev");

	QTemporaryDir dir;
	const QString path = dir.filePath("dump.csv");
	QCoro::waitFor(access.dump(path, qMakePair(0x1u, 0x2u)));
	QFile file(path);
	QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
	QCOMPARE(QString::fromUtf8(file.readAll()), QString("1,ab\n"));
}

void TstRegisterMapAccess::noStrategy()
{
	RegisterMapAccess access(nullptr, nullptr, true, "dev");
	const RegisterData r = QCoro::waitFor(access.read(0x1));
	QVERIFY(r.isError());
	QCOMPARE(r.error->code, -ENOTSUP);
	const RegisterData w = QCoro::waitFor(access.writeThenRead(0x1, 0x1));
	QCOMPARE(w.error->code, -ENOTSUP);
	QVERIFY(!access.store()->get(0x1));
}

QTEST_MAIN(TstRegisterMapAccess)
#include "tst_registermapaccess.moc"
