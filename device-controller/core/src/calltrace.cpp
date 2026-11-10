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

std::atomic<bool> CallTrace::s_enabled{false};
std::atomic<int> CallTrace::s_active{0};

namespace {
thread_local Command *t_current = nullptr;
std::atomic<bool> s_captureData{false};
std::atomic<int> s_previewBytes{64};
std::atomic<CallTrace::Sink> s_sink{nullptr};
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
	if(CallTrace::isEnabled()) {
		TraceData *t = traceData();
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

// --- CallTrace ---

void CallTrace::setEnabled(bool enabled) { s_enabled.store(enabled, std::memory_order_relaxed); }

bool CallTrace::captureData() { return s_captureData.load(std::memory_order_relaxed); }

void CallTrace::setCaptureData(bool capture) { s_captureData.store(capture, std::memory_order_relaxed); }

int CallTrace::previewBytes() { return s_previewBytes.load(std::memory_order_relaxed); }

void CallTrace::setPreviewBytes(int bytes) { s_previewBytes.store(qMax(0, bytes), std::memory_order_relaxed); }

void CallTrace::setSink(Sink sink) { s_sink.store(sink, std::memory_order_release); }

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
	e.commandClass = QString::fromLatin1(cmd->metaObject()->className());
	e.startNs = cmd->startedNs();
	e.durationNs = cmd->finishedNs() - cmd->startedNs();
	e.error = cmd->failure();
	e.calls = cmd->calls();
	return e;
}

void CallTrace::publish(const Command *cmd)
{
	if(!cmd || !cmd->isTraced()) {
		return;
	}
	Sink sink = s_sink.load(std::memory_order_acquire);
	if(sink) {
		sink(entryOf(cmd));
	}
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
	const qint64 keep = captureData() ? size : qMin<qint64>(size, previewBytes());
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
	const qint64 keep = captureData() ? len : qMin<qint64>(len, previewBytes());
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
