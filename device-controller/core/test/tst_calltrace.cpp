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

#include "core/calltrace.h"
#include "core/pooledcmdexecutor.h"
#include "core/resultcommand.h"

#include <QFuture>
#include <QMutex>
#include <QTest>
#include <QThread>

#include <cerrno>
#include <functional>
#include <new>

#if defined(__GLIBC__)
#include <cstdlib>
// Counts heap allocations of the current thread (malloc covers Qt containers, new covers the rest).
static thread_local bool t_countAllocs = false;
static thread_local int t_allocs = 0;
extern "C" void *__libc_malloc(size_t);
extern "C" void *__libc_calloc(size_t, size_t);
extern "C" void *__libc_realloc(void *, size_t);
extern "C" void *malloc(size_t size)
{
	if(t_countAllocs) {
		++t_allocs;
	}
	return __libc_malloc(size);
}
extern "C" void *calloc(size_t n, size_t size)
{
	if(t_countAllocs) {
		++t_allocs;
	}
	return __libc_calloc(n, size);
}
extern "C" void *realloc(void *p, size_t size)
{
	if(t_countAllocs) {
		++t_allocs;
	}
	return __libc_realloc(p, size);
}
#define DC_HAVE_ALLOC_COUNTER 1
#endif

using namespace scopy;

namespace {

int fakeCall(int v) { return v; }

int failingCall()
{
	errno = EBUSY;
	return -EBUSY;
}

// Runs `body` as its run(); fails with `rc` when it is negative.
class FnTestCommand : public ResultCommand<void>
{
	Q_OBJECT
public:
	explicit FnTestCommand(std::function<int()> body)
		: ResultCommand(nullptr)
		, m_body(std::move(body))
	{}

protected:
	void run() override
	{
		const int rc = m_body();
		setResult(rc < 0 ? Result<void>(Unexpected{Error{rc, QStringLiteral("failed")}}) : Result<void>());
	}

private:
	std::function<int()> m_body;
};

QMutex s_entriesMutex;
QList<CommandLogEntry> s_entries;

void collect(const CommandLogEntry &e)
{
	QMutexLocker lock(&s_entriesMutex);
	s_entries.append(e);
}

} // namespace

class TstCallTrace : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void init()
	{
		CallTrace::setEnabled(false);
		CallTrace::setCaptureData(false);
		CallTrace::setPreviewBytes(64);
		CallTrace::setSink(nullptr);
		s_entries.clear();
	}
	void cleanup() { CallTrace::setEnabled(false); }

	void traceInsideCommand();
	void traceOutsideCommandIsNoop();
	void disabledRecordsNothing();
	void disabledDoesNotAllocate();
	void toggleMidCommandKeepsDecision();
	void rcOutAndErrnoPreserved();
	void payloadPreviewAndCapture();
	void executorPublishesEntry();
	void cancelledCommandPublishesNothing();
	void recordToString();
};

void TstCallTrace::traceInsideCommand()
{
	CallTrace::setEnabled(true);
	Command *seen = nullptr;
	FnTestCommand cmd([&seen] {
		seen = Command::current();
		int a = DC_TRACE("testlib", "first_call", nullptr, fakeCall(1), CallArg::string("attr", "raw"));
		QThread::usleep(200);
		int b = DC_TRACE("testlib", "second_call", reinterpret_cast<void *>(0x10), fakeCall(2),
				 CallArg::hex("addr", 0x50));
		return a + b - 3;
	});
	QCOMPARE(Command::current(), nullptr);
	cmd.execute();
	QCOMPARE(seen, &cmd);
	QCOMPARE(Command::current(), nullptr);

	QVERIFY(cmd.isTraced());
	QVERIFY(!cmd.isRecording());
	const QList<CallRecord> calls = cmd.calls();
	QCOMPARE(calls.size(), 2);
	QCOMPARE(calls[0].library, QStringLiteral("testlib"));
	QCOMPARE(calls[0].function, QStringLiteral("first_call"));
	QCOMPARE(calls[0].rc, 1);
	QCOMPARE(calls[0].args.size(), 1);
	QCOMPARE(calls[0].args[0].kind, CallArg::String);
	QCOMPARE(calls[1].function, QStringLiteral("second_call"));
	QCOMPARE(calls[1].handle, quintptr(0x10));
	QCOMPARE(calls[1].rc, 2);
	// Order and timing: both inside the command, the second after the first.
	QVERIFY(calls[0].startNs >= cmd.startedNs());
	QVERIFY(calls[1].startNs >= calls[0].startNs + calls[0].durationNs);
	QVERIFY(calls[1].startNs - calls[0].startNs >= 200 * 1000);
	QVERIFY(cmd.finishedNs() >= calls[1].startNs);
	QVERIFY(calls[0].durationNs >= 0);
	QVERIFY(!CallTrace::active());
}

void TstCallTrace::traceOutsideCommandIsNoop()
{
	CallTrace::setEnabled(true);
	QCOMPARE(Command::current(), nullptr);
	int v = DC_TRACE("testlib", "outside", nullptr, fakeCall(7), CallArg::integer("x", 1));
	QCOMPARE(v, 7);
	QVERIFY(!CallTrace::active());
}

void TstCallTrace::disabledRecordsNothing()
{
	FnTestCommand cmd([] { return DC_TRACE("testlib", "call", nullptr, fakeCall(0)); });
	cmd.setOrigin(QStringLiteral("dev/attr"));
	cmd.execute();
	QVERIFY(!cmd.isTraced());
	QVERIFY(cmd.calls().isEmpty());
	QCOMPARE(cmd.startedNs(), 0);
}

void TstCallTrace::disabledDoesNotAllocate()
{
#if defined(DC_HAVE_ALLOC_COUNTER)
	FnTestCommand cmd([] {
		const QString name = QStringLiteral("raw");
		t_allocs = 0;
		t_countAllocs = true;
		int sum = 0;
		for(int i = 0; i < 100; ++i) {
			sum += DC_TRACE("testlib", "call", nullptr, fakeCall(i), CallArg::string("attr", name),
					CallArg::integer("i", i));
		}
		t_countAllocs = false;
		return sum >= 0 ? 0 : -1;
	});
	cmd.execute();
	QCOMPARE(t_allocs, 0);

	// Sanity: the counter sees the allocations of a recording command.
	CallTrace::setEnabled(true);
	FnTestCommand traced([] {
		t_allocs = 0;
		t_countAllocs = true;
		DC_TRACE("testlib", "call", nullptr, fakeCall(1), CallArg::string("attr", QStringLiteral("raw")));
		t_countAllocs = false;
		return 0;
	});
	traced.execute();
	QVERIFY(t_allocs > 0);
#else
	QSKIP("allocation counter needs glibc");
#endif
}

void TstCallTrace::toggleMidCommandKeepsDecision()
{
	// Started on: keeps recording after the monitor is turned off.
	CallTrace::setEnabled(true);
	FnTestCommand on([] {
		DC_TRACE("testlib", "a", nullptr, fakeCall(0));
		CallTrace::setEnabled(false);
		DC_TRACE("testlib", "b", nullptr, fakeCall(0));
		return 0;
	});
	on.execute();
	QVERIFY(on.isTraced());
	QCOMPARE(on.calls().size(), 2);

	// Started off: records nothing even if turned on midway.
	FnTestCommand off([] {
		DC_TRACE("testlib", "a", nullptr, fakeCall(0));
		CallTrace::setEnabled(true);
		DC_TRACE("testlib", "b", nullptr, fakeCall(0));
		return 0;
	});
	off.execute();
	QVERIFY(!off.isTraced());
	QVERIFY(off.calls().isEmpty());
}

void TstCallTrace::rcOutAndErrnoPreserved()
{
	CallTrace::setEnabled(true);
	int errnoAfter = 0;
	FnTestCommand cmd([&errnoAfter] {
		uint32_t val = 0;
		int rc = DC_TRACE_OUT("testlib", "reg_read", nullptr, (val = 0xabcd, fakeCall(0)),
				      QVariant::fromValue(quint64(val)), CallArg::hex("addr", 0x10));
		errno = 0;
		rc = DC_TRACE("testlib", "fails", nullptr, failingCall());
		errnoAfter = errno;
		const char *p = DC_TRACE("testlib", "returns_null", nullptr, static_cast<const char *>(nullptr));
		Q_UNUSED(p);
		return rc;
	});
	cmd.execute();
	QCOMPARE(errnoAfter, EBUSY);
	const QList<CallRecord> calls = cmd.calls();
	QCOMPARE(calls.size(), 3);
	QCOMPARE(calls[0].out.toULongLong(), 0xabcdULL);
	QCOMPARE(calls[1].rc, -EBUSY);
	QVERIFY(calls[1].failed());
	QVERIFY(calls[2].rc < 0);
	QCOMPARE(calls[2].outKind, CallArg::Handle);
	QCOMPARE(cmd.failure().code, -EBUSY);
}

void TstCallTrace::payloadPreviewAndCapture()
{
	QByteArray data(200, 'x');
	CallOut o = CallTrace::payload(data.constData(), data.size());
	QCOMPARE(o.kind, CallArg::Bytes);
	QCOMPARE(o.size, 200);
	QCOMPARE(o.value.toByteArray().size(), 64);

	CallTrace::setCaptureData(true);
	o = CallTrace::payload(data.constData(), data.size());
	QCOMPARE(o.value.toByteArray().size(), 200);
	CallTrace::setCaptureData(false);

	const char txt[] = "1234\0";
	o = CallTrace::text(txt, sizeof(txt));
	QCOMPARE(o.kind, CallArg::String);
	QCOMPARE(o.value.toString(), QStringLiteral("1234"));
	QCOMPARE(o.size, 4);
}

void TstCallTrace::executorPublishesEntry()
{
	CallTrace::setSink(&collect);
	CallTrace::setEnabled(true);
	PooledCmdExecutor exec(1);

	auto *ok = new FnTestCommand([] { return DC_TRACE("testlib", "ok_call", nullptr, fakeCall(0)); });
	CommandOrigin origin;
	origin.path = QStringLiteral("ctx/dev/raw");
	origin.uri = QStringLiteral("ip:test");
	origin.backendId = QStringLiteral("iio");
	origin.object = 0x1234;
	ok->setOrigin(origin);
	auto *bad = new FnTestCommand([] { return DC_TRACE("testlib", "bad_call", nullptr, failingCall()); });

	QFuture<void> f1 = exec.execute(ok);
	QFuture<void> f2 = exec.execute(bad);
	f1.waitForFinished();
	f2.waitForFinished();

	QMutexLocker lock(&s_entriesMutex);
	QCOMPARE(s_entries.size(), 2);
	const CommandLogEntry &e = s_entries[0];
	QCOMPARE(e.origin, origin.path);
	QCOMPARE(e.uri, origin.uri);
	QCOMPARE(e.backendId, origin.backendId);
	QCOMPARE(e.originObject, quintptr(0x1234));
	QVERIFY(e.commandClass.contains(QStringLiteral("FnTestCommand")));
	QVERIFY(!e.failed());
	QCOMPARE(e.calls.size(), 1);
	QVERIFY(e.durationNs >= e.calls[0].durationNs);
	QVERIFY(s_entries[1].failed());
	QCOMPARE(s_entries[1].error.code, -EBUSY);
	QCOMPARE(s_entries[1].calls[0].function, QStringLiteral("bad_call"));
	delete ok;
	delete bad;
}

void TstCallTrace::cancelledCommandPublishesNothing()
{
	CallTrace::setSink(&collect);
	CallTrace::setEnabled(true);
	auto *cmd = new FnTestCommand([] { return 0; });
	cmd->cancel();
	PooledCmdExecutor exec(1);
	exec.execute(cmd).waitForFinished();
	QMutexLocker lock(&s_entriesMutex);
	QVERIFY(s_entries.isEmpty());
	delete cmd;
}

void TstCallTrace::recordToString()
{
	CallRecord r;
	r.function = QStringLiteral("iio_attr_read_raw");
	r.args = {CallArg::string("attr", QStringLiteral("raw")), CallArg::outLen("len", 4096)};
	r.rc = 5;
	r.out = QStringLiteral("1234");
	r.outKind = CallArg::String;
	QCOMPARE(r.toString(), QStringLiteral("iio_attr_read_raw(raw, len=4096) = 5 \"1234\""));

	CallRecord w;
	w.function = QStringLiteral("iio_device_reg_write");
	w.args = {CallArg::hex("address", 0x80), CallArg::hex("value", 0xff)};
	QCOMPARE(w.toString(), QStringLiteral("iio_device_reg_write(address=0x80, value=0xff) = 0"));

	CallRecord same = w;
	QVERIFY(same.sameCall(w));
	same.rc = -5;
	QVERIFY(!same.sameCall(w));
}

QTEST_GUILESS_MAIN(TstCallTrace)
#include "tst_calltrace.moc"
