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
#include <QWaitCondition>

#include <cerrno>
#include <functional>
#include <memory>
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

// Thread-safe collector of one sink's entries.
struct Collector
{
	QMutex mutex;
	QList<CommandLogEntry> entries;

	TraceConfig::Sink sink()
	{
		return [this](const CommandLogEntry &e) {
			QMutexLocker lock(&mutex);
			entries.append(e);
		};
	}
	int count()
	{
		QMutexLocker lock(&mutex);
		return entries.size();
	}
};

// Blocks a command inside run() until released.
struct Gate
{
	QMutex mutex;
	QWaitCondition cond;
	bool entered = false;
	bool open = false;

	void wait()
	{
		QMutexLocker lock(&mutex);
		entered = true;
		cond.wakeAll();
		while(!open) {
			cond.wait(&mutex);
		}
	}
	void waitEntered()
	{
		QMutexLocker lock(&mutex);
		while(!entered) {
			cond.wait(&mutex);
		}
	}
	void release()
	{
		QMutexLocker lock(&mutex);
		open = true;
		cond.wakeAll();
	}
};

} // namespace

class TstCallTrace : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void init() { m_config = std::make_unique<TraceConfig>(); }
	void cleanup()
	{
		m_config.reset();
		QVERIFY(!TraceConfig::anyEnabled());
		QVERIFY(!CallTrace::active());
	}

	void traceInsideCommand();
	void traceOutsideCommandIsNoop();
	void directExecuteNeverRecords();
	void disabledRecordsNothing();
	void disabledDoesNotAllocate();
	void toggleMidCommandKeepsDecision();
	void rcOutAndErrnoPreserved();
	void payloadPreviewAndCapture();
	void configCountsEnabled();
	void executorPublishesEntry();
	void executorWithoutConfigRecordsNothing();
	void twoExecutorsAreIndependent();
	void sinkDetachedWhileInFlight();
	void cancelledCommandPublishesNothing();
	void commandWithoutCallsPublishesNothing();
	void recordToString();

private:
	// Runs `cmd` on this thread the way an executor does, under m_config.
	void run(Command &cmd) { CallTrace::execute(&cmd, m_config.get()); }

	std::unique_ptr<TraceConfig> m_config;
};

void TstCallTrace::traceInsideCommand()
{
	m_config->setEnabled(true);
	Command *seen = nullptr;
	const TraceConfig *seenConfig = nullptr;
	FnTestCommand cmd([&seen, &seenConfig] {
		seen = Command::current();
		seenConfig = CallTrace::config();
		int a = DC_TRACE("testlib", "first_call", nullptr, fakeCall(1), CallArg::string("attr", "raw"));
		QThread::usleep(200);
		int b = DC_TRACE("testlib", "second_call", reinterpret_cast<void *>(0x10), fakeCall(2),
				 CallArg::hex("addr", 0x50));
		return a + b - 3;
	});
	QCOMPARE(Command::current(), nullptr);
	run(cmd);
	QCOMPARE(seen, &cmd);
	QCOMPARE(seenConfig, m_config.get());
	QCOMPARE(Command::current(), nullptr);
	QCOMPARE(CallTrace::config(), nullptr);

	QVERIFY(cmd.isTraced());
	QVERIFY(!cmd.isRecording());
	QCOMPARE(cmd.traceConfig(), nullptr);
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
	m_config->setEnabled(true);
	QCOMPARE(Command::current(), nullptr);
	int v = DC_TRACE("testlib", "outside", nullptr, fakeCall(7), CallArg::integer("x", 1));
	QCOMPARE(v, 7);
	QVERIFY(!CallTrace::active());
}

void TstCallTrace::directExecuteNeverRecords()
{
	// A config is on, but the command is not run by an executor.
	m_config->setEnabled(true);
	FnTestCommand cmd([] { return DC_TRACE("testlib", "call", nullptr, fakeCall(0)); });
	cmd.execute();
	QVERIFY(!cmd.isTraced());
	QVERIFY(cmd.calls().isEmpty());

	// Nor does a null config.
	CallTrace::execute(&cmd, nullptr);
	QVERIFY(!cmd.isTraced());
}

void TstCallTrace::disabledRecordsNothing()
{
	FnTestCommand cmd([] { return DC_TRACE("testlib", "call", nullptr, fakeCall(0)); });
	cmd.setOrigin(QStringLiteral("dev/attr"));
	run(cmd);
	QVERIFY(!cmd.isTraced());
	QVERIFY(cmd.calls().isEmpty());
	QCOMPARE(cmd.startedNs(), 0);
}

void TstCallTrace::disabledDoesNotAllocate()
{
#if defined(DC_HAVE_ALLOC_COUNTER)
	// Another executor's config is on: this one's commands still take the fast path.
	TraceConfig other;
	other.setEnabled(true);
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
	run(cmd);
	QCOMPARE(t_allocs, 0);
	other.setEnabled(false);

	// Sanity: the counter sees the allocations of a recording command.
	m_config->setEnabled(true);
	FnTestCommand traced([] {
		t_allocs = 0;
		t_countAllocs = true;
		DC_TRACE("testlib", "call", nullptr, fakeCall(1), CallArg::string("attr", QStringLiteral("raw")));
		t_countAllocs = false;
		return 0;
	});
	run(traced);
	QVERIFY(t_allocs > 0);
#else
	QSKIP("allocation counter needs glibc");
#endif
}

void TstCallTrace::toggleMidCommandKeepsDecision()
{
	TraceConfig *config = m_config.get();
	// Started on: keeps recording after the config is turned off.
	config->setEnabled(true);
	FnTestCommand on([config] {
		DC_TRACE("testlib", "a", nullptr, fakeCall(0));
		config->setEnabled(false);
		DC_TRACE("testlib", "b", nullptr, fakeCall(0));
		return 0;
	});
	run(on);
	QVERIFY(on.isTraced());
	QCOMPARE(on.calls().size(), 2);

	// Started off: records nothing even if turned on midway.
	FnTestCommand off([config] {
		DC_TRACE("testlib", "a", nullptr, fakeCall(0));
		config->setEnabled(true);
		DC_TRACE("testlib", "b", nullptr, fakeCall(0));
		return 0;
	});
	run(off);
	QVERIFY(!off.isTraced());
	QVERIFY(off.calls().isEmpty());
}

void TstCallTrace::rcOutAndErrnoPreserved()
{
	m_config->setEnabled(true);
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
	run(cmd);
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
	const QByteArray data(200, 'x');
	// Outside a recording command: the default preview.
	CallOut o = CallTrace::payload(data.constData(), data.size());
	QCOMPARE(o.kind, CallArg::Bytes);
	QCOMPARE(o.size, 200);
	QCOMPARE(o.value.toByteArray().size(), TraceConfig::DEFAULT_PREVIEW_BYTES);

	// Inside: the running executor's config decides.
	m_config->setEnabled(true);
	CallOut inside;
	FnTestCommand cmd([&inside, &data] {
		inside = CallTrace::payload(data.constData(), data.size());
		return 0;
	});
	m_config->setPreviewBytes(16);
	run(cmd);
	QCOMPARE(inside.value.toByteArray().size(), 16);
	QCOMPARE(inside.size, 200);
	m_config->setCaptureData(true);
	run(cmd);
	QCOMPARE(inside.value.toByteArray().size(), 200);

	const char txt[] = "1234\0";
	o = CallTrace::text(txt, sizeof(txt));
	QCOMPARE(o.kind, CallArg::String);
	QCOMPARE(o.value.toString(), QStringLiteral("1234"));
	QCOMPARE(o.size, 4);
}

void TstCallTrace::configCountsEnabled()
{
	QVERIFY(!TraceConfig::anyEnabled());
	QVERIFY(m_config->setEnabled(true));
	QVERIFY(!m_config->setEnabled(true)); // no change
	QVERIFY(TraceConfig::anyEnabled());
	{
		TraceConfig other;
		QVERIFY(other.setEnabled(true));
		QVERIFY(m_config->setEnabled(false));
		QVERIFY(TraceConfig::anyEnabled());
	} // destroyed while on
	QVERIFY(!TraceConfig::anyEnabled());
	QVERIFY(m_config->setCaptureData(true));
	QVERIFY(!m_config->setCaptureData(true));
	m_config->setPreviewBytes(-5);
	QCOMPARE(m_config->previewBytes(), 0);
}

void TstCallTrace::executorPublishesEntry()
{
	Collector c;
	auto config = std::make_shared<TraceConfig>();
	config->setSink(c.sink());
	config->setEnabled(true);
	PooledCmdExecutor exec(1);
	exec.setTraceConfig(config);
	QCOMPARE(exec.traceConfig(), config);

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

	QMutexLocker lock(&c.mutex);
	QCOMPARE(c.entries.size(), 2);
	const CommandLogEntry &e = c.entries[0];
	QCOMPARE(e.origin, origin.path);
	QCOMPARE(e.uri, origin.uri);
	QCOMPARE(e.backendId, origin.backendId);
	QCOMPARE(e.originObject, quintptr(0x1234));
	QVERIFY(e.commandClass.contains(QStringLiteral("FnTestCommand")));
	QVERIFY(!e.failed());
	QCOMPARE(e.calls.size(), 1);
	QVERIFY(e.durationNs >= e.calls[0].durationNs);
	QVERIFY(c.entries[1].failed());
	QCOMPARE(c.entries[1].error.code, -EBUSY);
	QCOMPARE(c.entries[1].calls[0].function, QStringLiteral("bad_call"));
	delete ok;
	delete bad;
}

void TstCallTrace::executorWithoutConfigRecordsNothing()
{
	m_config->setEnabled(true); // on, but not installed on this executor
	PooledCmdExecutor exec(1);
	QCOMPARE(exec.traceConfig(), nullptr);
	FnTestCommand cmd([] { return DC_TRACE("testlib", "call", nullptr, fakeCall(0)); });
	exec.execute(&cmd).waitForFinished();
	QVERIFY(!cmd.isTraced());
	QVERIFY(cmd.calls().isEmpty());
}

void TstCallTrace::twoExecutorsAreIndependent()
{
	Collector ca, cb;
	auto a = std::make_shared<TraceConfig>();
	auto b = std::make_shared<TraceConfig>();
	a->setSink(ca.sink());
	b->setSink(cb.sink());
	PooledCmdExecutor execA(1), execB(1);
	execA.setTraceConfig(a);
	execB.setTraceConfig(b);
	auto runBoth = [&]() {
		FnTestCommand ra([] { return DC_TRACE("testlib", "on_a", nullptr, fakeCall(0)); });
		FnTestCommand rb([] { return DC_TRACE("testlib", "on_b", nullptr, fakeCall(0)); });
		QFuture<void> fa = execA.execute(&ra);
		QFuture<void> fb = execB.execute(&rb);
		fa.waitForFinished();
		fb.waitForFinished();
	};

	a->setEnabled(true);
	runBoth();
	QCOMPARE(ca.count(), 1);
	QCOMPARE(cb.count(), 0);
	QCOMPARE(ca.entries[0].calls[0].function, QStringLiteral("on_a"));

	// Toggling b leaves a alone.
	b->setEnabled(true);
	runBoth();
	QCOMPARE(ca.count(), 2);
	QCOMPARE(cb.count(), 1);
	QCOMPARE(cb.entries[0].calls[0].function, QStringLiteral("on_b"));
	b->setEnabled(false);
	QVERIFY(a->isEnabled());
	runBoth();
	QCOMPARE(ca.count(), 3);
	QCOMPARE(cb.count(), 1);
}

void TstCallTrace::sinkDetachedWhileInFlight()
{
	Collector c;
	Gate gate;
	auto config = std::make_shared<TraceConfig>();
	config->setSink(c.sink());
	config->setEnabled(true);
	std::weak_ptr<TraceConfig> weak = config;
	auto *exec = new PooledCmdExecutor(1);
	exec->setTraceConfig(config);

	FnTestCommand cmd([&gate] {
		DC_TRACE("testlib", "before", nullptr, fakeCall(0));
		gate.wait();
		return DC_TRACE("testlib", "after", nullptr, fakeCall(0));
	});
	QFuture<void> f = exec->execute(&cmd);
	gate.waitEntered();
	// The owner goes away mid-command: detach, drop its reference.
	config->setEnabled(false);
	config->setSink(nullptr);
	config.reset();
	QVERIFY(!weak.expired()); // the executor keeps it alive
	gate.release();
	f.waitForFinished();
	QVERIFY(cmd.isTraced()); // decided at start, no partial entry
	QCOMPARE(cmd.calls().size(), 2);
	QCOMPARE(c.count(), 0); // but not delivered
	delete exec;
	QVERIFY(weak.expired());
}

void TstCallTrace::cancelledCommandPublishesNothing()
{
	Collector c;
	auto config = std::make_shared<TraceConfig>();
	config->setSink(c.sink());
	config->setEnabled(true);
	auto *cmd = new FnTestCommand([] { return 0; });
	cmd->cancel();
	PooledCmdExecutor exec(1);
	exec.setTraceConfig(config);
	exec.execute(cmd).waitForFinished();
	QCOMPARE(c.count(), 0);
	delete cmd;
}

void TstCallTrace::commandWithoutCallsPublishesNothing()
{
	Collector c;
	auto config = std::make_shared<TraceConfig>();
	config->setSink(c.sink());
	config->setEnabled(true);
	PooledCmdExecutor exec(1);
	exec.setTraceConfig(config);
	// A cached read: runs as a command, calls no library.
	FnTestCommand cached([] { return 0; });
	exec.execute(&cached).waitForFinished();
	QVERIFY(cached.isTraced());
	QVERIFY(cached.calls().isEmpty());
	QCOMPARE(c.count(), 0);
	FnTestCommand real([] { return DC_TRACE("testlib", "call", nullptr, fakeCall(0)); });
	exec.execute(&real).waitForFinished();
	QCOMPARE(c.count(), 1);
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
