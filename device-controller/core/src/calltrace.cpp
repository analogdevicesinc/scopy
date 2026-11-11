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

#include <QDateTime>

#include <chrono>

using namespace scopy;

std::atomic<int> CallTrace::s_active{0};
std::atomic<int> TraceConfig::s_enabledCount{0};

namespace {
thread_local Command *t_current = nullptr;
// Config of the executor running a command on this thread (CallTrace::execute), nullptr elsewhere.
thread_local const TraceConfig *t_execConfig = nullptr;
} // namespace

// --- Command ---

Command *Command::current() { return t_current; }

Command::TraceData *Command::traceData()
{
	if(!m_trace) {
		m_trace = std::make_unique<TraceData>();
	}
	return m_trace.get();
}

Command *Command::beginRun()
{
	Command *outer = t_current;
	t_current = this;
	const TraceConfig *config = t_execConfig;
	if(config && config->isEnabled()) {
		TraceData *t = traceData();
		t->config = config;
		t->calls.clear();
		t->startedNs = CallTrace::nowNs();
		t->steadyStartNs = CallTrace::steadyNs();
		t->finishedNs = t->startedNs;
		t->recording = true;
		t->traced = true;
		CallTrace::s_active.fetch_add(1, std::memory_order_relaxed);
	} else if(m_trace) {
		m_trace->traced = false;
	}
	return outer;
}

void Command::endRun(Command *outer)
{
	if(m_trace && m_trace->recording) {
		m_trace->finishedNs = m_trace->startedNs + (CallTrace::steadyNs() - m_trace->steadyStartNs);
		m_trace->recording = false;
		m_trace->config = nullptr;
		CallTrace::s_active.fetch_sub(1, std::memory_order_relaxed);
	}
	t_current = outer;
}

void Command::trace(CallRecord record)
{
	if(isRecording()) {
		m_trace->calls.append(std::move(record));
	}
}

void Command::setOrigin(const QString &path) { traceData()->origin.path = path; }

void Command::setOrigin(const CommandOrigin &origin) { traceData()->origin = origin; }

// --- TraceConfig ---

TraceConfig::~TraceConfig() { setEnabled(false); }

bool TraceConfig::setEnabled(bool enabled)
{
	if(m_enabled.exchange(enabled, std::memory_order_relaxed) == enabled) {
		return false;
	}
	s_enabledCount.fetch_add(enabled ? 1 : -1, std::memory_order_relaxed);
	return true;
}

bool TraceConfig::setCaptureData(bool capture)
{
	return m_captureData.exchange(capture, std::memory_order_relaxed) != capture;
}

void TraceConfig::setPreviewBytes(int bytes) { m_previewBytes.store(qMax(0, bytes), std::memory_order_relaxed); }

void TraceConfig::setSink(Sink sink)
{
	// Taken by publish() for the whole call: once this returns no thread is still in the old sink.
	std::lock_guard<std::mutex> lock(m_sinkMutex);
	m_sink = std::move(sink);
}

void TraceConfig::publish(const CommandLogEntry &entry) const
{
	std::lock_guard<std::mutex> lock(m_sinkMutex);
	if(m_sink) {
		m_sink(entry);
	}
}

// --- CallTrace ---

void CallTrace::execute(Command *cmd, const TraceConfig *config)
{
	if(!cmd) {
		return;
	}
	const TraceConfig *outer = t_execConfig;
	t_execConfig = config;
	cmd->execute();
	t_execConfig = outer;
	publish(cmd, config);
}

const TraceConfig *CallTrace::config()
{
	const Command *cmd = Command::current();
	return cmd ? cmd->traceConfig() : nullptr;
}

CommandLogEntry CallTrace::entryOf(const Command *cmd)
{
	CommandLogEntry e;
	if(!cmd) {
		return e;
	}
	const CommandOrigin o = cmd->originInfo();
	e.origin = o.path;
	e.originObject = o.object;
	e.uri = o.uri;
	e.backendId = o.backendId;
	e.originScope = o.scope;
	e.commandClass = QString::fromLatin1(cmd->metaObject()->className());
	e.startNs = cmd->startedNs();
	e.durationNs = cmd->finishedNs() - cmd->startedNs();
	e.error = cmd->failure();
	e.calls = cmd->calls();
	return e;
}

void CallTrace::publish(const Command *cmd, const TraceConfig *config)
{
	// A command that reached no library (cached / client-side reads and writes) is not logged: the
	// log lists the library calls, and such an entry carries none.
	if(!cmd || !config || !cmd->isTraced() || cmd->calls().isEmpty()) {
		return;
	}
	config->publish(entryOf(cmd));
}

qint64 CallTrace::nowNs()
{
	using namespace std::chrono;
	return duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count();
}

qint64 CallTrace::steadyNs()
{
	using namespace std::chrono;
	return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
}

CallOut CallTrace::payload(const void *data, qint64 size)
{
	CallOut o;
	o.kind = CallArg::Bytes;
	o.size = qMax<qint64>(0, size);
	if(!data || size <= 0) {
		o.value = QByteArray();
		return o;
	}
	const TraceConfig *c = config();
	const bool all = c && c->captureData();
	const qint64 keep = all ? size : qMin<qint64>(size, c ? c->previewBytes() : TraceConfig::DEFAULT_PREVIEW_BYTES);
	o.value = QByteArray(static_cast<const char *>(data), static_cast<qsizetype>(keep));
	return o;
}

CallOut CallTrace::text(const char *data, qint64 size)
{
	CallOut o;
	o.kind = CallArg::String;
	qint64 len = (data && size > 0) ? size : 0;
	while(len > 0 && data[len - 1] == '\0') {
		--len;
	}
	o.size = len;
	const TraceConfig *c = config();
	const bool all = c && c->captureData();
	const qint64 keep = all ? len : qMin<qint64>(len, c ? c->previewBytes() : TraceConfig::DEFAULT_PREVIEW_BYTES);
	o.value = QString::fromUtf8(data, static_cast<qsizetype>(keep));
	return o;
}

CallRecord CallTrace::begin(const char *library, const char *function, const void *handle)
{
	CallRecord rec;
	rec.library = QString::fromLatin1(library);
	rec.function = QString::fromLatin1(function);
	rec.handle = quintptr(handle);
	rec.startNs = nowNs();
	return rec;
}
