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

#pragma once

#include "core/callrecord.h"
#include "core/command.h"
#include "core/controller_core_export.h"

#include <QByteArray>
#include <QList>
#include <QVariant>

#include <atomic>
#include <cerrno>
#include <functional>
#include <mutex>
#include <type_traits>
#include <utility>

namespace scopy {

// Value a traced call produced (read value, returned handle, payload preview).
struct CallOut
{
	QVariant value;
	qint64 size = -1; // full payload size when `value` is a preview
	CallArg::Kind kind = CallArg::Int;

	CallOut() = default;
	CallOut(const QVariant &v)
		: value(v)
		, kind(kindOf(v))
	{}
	template <typename T,
		  typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, CallOut> &&
					      !std::is_same_v<std::decay_t<T>, QVariant> &&
					      !std::is_pointer_v<std::decay_t<T>>>>
	CallOut(const T &v)
		: CallOut(QVariant::fromValue(v))
	{}

	static CallArg::Kind kindOf(const QVariant &v)
	{
		switch(v.typeId()) {
		case QMetaType::QString:
			return CallArg::String;
		case QMetaType::QByteArray:
			return CallArg::Bytes;
		case QMetaType::Bool:
			return CallArg::Bool;
		default:
			return CallArg::Int;
		}
	}
};

// Trace switches and sink of one executor (one connection). Off by default. Shared: the executor
// keeps it alive while a command runs, the owner (the connection's command monitor) detaches its
// sink before it goes away. All members are thread-safe.
class CONTROLLER_CORE_EXPORT TraceConfig
{
public:
	using Sink = std::function<void(const CommandLogEntry &entry)>;
	static constexpr int DEFAULT_PREVIEW_BYTES = 64;

	TraceConfig() = default;
	~TraceConfig();
	TraceConfig(const TraceConfig &) = delete;
	TraceConfig &operator=(const TraceConfig &) = delete;

	// Commands that start afterwards on the executor record their calls. Returns true if changed.
	bool isEnabled() const { return m_enabled.load(std::memory_order_relaxed); }
	bool setEnabled(bool enabled);

	// Payloads (refill / push blocks, attr values) keep previewBytes() bytes, or everything when
	// captureData() is on. setCaptureData returns true if changed.
	bool captureData() const { return m_captureData.load(std::memory_order_relaxed); }
	bool setCaptureData(bool capture);
	int previewBytes() const { return m_previewBytes.load(std::memory_order_relaxed); }
	void setPreviewBytes(int bytes);

	// Receiver of finished recorded commands; called on the executor thread. Once setSink()
	// returns, the previous sink is never called again (nullptr detaches).
	void setSink(Sink sink);
	void publish(const CommandLogEntry &entry) const;

	// True while at least one config (any executor) is enabled: one relaxed load.
	static bool anyEnabled() { return s_enabledCount.load(std::memory_order_relaxed) > 0; }

private:
	std::atomic<bool> m_enabled{false};
	std::atomic<bool> m_captureData{false};
	std::atomic<int> m_previewBytes{DEFAULT_PREVIEW_BYTES};
	mutable std::mutex m_sinkMutex;
	Sink m_sink;

	static std::atomic<int> s_enabledCount;
};

// Library-call tracing. An executor runs each command under its TraceConfig (execute()); the
// command decides once, when it starts, whether it records (that config's enabled flag at that
// moment); DC_TRACE then records every library call it makes. A command executed directly
// (outside any executor) never records.
// Off: DC_TRACE is one relaxed atomic load + the call (no record, no clock, no allocation).
class CONTROLLER_CORE_EXPORT CallTrace
{
public:
	// Executor hook: runs `cmd` under `config` (may be null: no tracing) and hands it to the
	// config's sink if it recorded.
	static void execute(Command *cmd, const TraceConfig *config);
	// Config of the command recording on this thread, nullptr elsewhere.
	static const TraceConfig *config();

	// True while at least one command (on any thread) is recording.
	static bool active() { return s_active.load(std::memory_order_relaxed) > 0; }

	// Hands a finished command to the config's sink (no-op if it did not record).
	static void publish(const Command *cmd, const TraceConfig *config);
	static CommandLogEntry entryOf(const Command *cmd);

	static qint64 nowNs();	  // wall clock, ns since epoch
	static qint64 steadyNs(); // monotonic, for durations

	static CallOut payload(const void *data, qint64 size);
	static CallOut text(const char *data, qint64 size); // NUL-stripped, as a string

	template <typename T>
	static int rcOf(const T &v)
	{
		if constexpr(std::is_pointer_v<T>) {
			return v ? 0 : (errno ? -errno : -1);
		} else if constexpr(std::is_arithmetic_v<T> || std::is_enum_v<T>) {
			return static_cast<int>(v);
		} else {
			return 0;
		}
	}

	template <typename T>
	static CallOut outOf(const T &v)
	{
		if constexpr(std::is_pointer_v<T>) {
			CallOut o(QVariant::fromValue(quintptr(v)));
			o.kind = CallArg::Handle;
			return o;
		} else {
			return {};
		}
	}

	// Body of DC_TRACE_EX; use the macros.
	template <typename Call, typename Args, typename Rc, typename Out>
	static auto traced(const char *library, const char *function, const void *handle, Call &&call, Args &&args,
			   Rc &&rc, Out &&out) -> decltype(call())
	{
		Command *cmd = Command::current();
		if(!cmd || !cmd->isRecording()) {
			return call();
		}
		CallRecord rec = begin(library, function, handle);
		rec.args = args();
		const qint64 t0 = steadyNs();
		if constexpr(std::is_void_v<decltype(call())>) {
			call();
			const int err = errno;
			rec.durationNs = steadyNs() - t0;
			cmd->trace(std::move(rec));
			errno = err;
		} else {
			auto ret = call();
			const int err = errno;
			rec.durationNs = steadyNs() - t0;
			rec.rc = rc(ret);
			CallOut o = out(ret);
			rec.out = std::move(o.value);
			rec.outSize = o.size;
			rec.outKind = o.kind;
			cmd->trace(std::move(rec));
			errno = err; // callers read errno after a failed call (e.g. buffer open)
			return ret;
		}
	}

	static std::atomic<int> s_active;

private:
	static CallRecord begin(const char *library, const char *function, const void *handle);
};

} // namespace scopy

// DC_TRACE_EX(lib, fn, handle, expr, rcExpr, outExpr, args...)
//   Runs `expr` (the library call). Inside a recording command it also appends a CallRecord with
//   `lib` / `fn` (string literals), `handle` (object acted on), the CallArg list `args...`,
//   rc = `rcExpr` and out = `outExpr` (both may use `dcRc`, the value `expr` returned).
// DC_TRACE_OUT(lib, fn, handle, expr, outExpr, args...): default rc (integer value, or 0 / -errno for a pointer).
// DC_TRACE(lib, fn, handle, expr, args...): default rc, out = returned handle for pointer results.
#define DC_TRACE_EX(lib, fn, handle, expr, rcExpr, outExpr, ...)                                                       \
	(Q_LIKELY(!::scopy::CallTrace::active())                                                                       \
		 ? (expr)                                                                                              \
		 : ::scopy::CallTrace::traced(                                                                         \
			   (lib), (fn), (handle), [&]() -> decltype(auto) { return expr; },                            \
			   [&]() { return QList<::scopy::CallArg>{__VA_ARGS__}; },                                     \
			   [&]([[maybe_unused]] const auto &dcRc) -> int { return (rcExpr); },                         \
			   [&]([[maybe_unused]] const auto &dcRc) -> ::scopy::CallOut { return (outExpr); }))

#define DC_TRACE_OUT(lib, fn, handle, expr, outExpr, ...)                                                              \
	DC_TRACE_EX(lib, fn, handle, expr, ::scopy::CallTrace::rcOf(dcRc), outExpr, __VA_ARGS__)

#define DC_TRACE(lib, fn, handle, expr, ...)                                                                           \
	DC_TRACE_EX(lib, fn, handle, expr, ::scopy::CallTrace::rcOf(dcRc), ::scopy::CallTrace::outOf(dcRc), __VA_ARGS__)
