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

// Library-call tracing. A command decides once, when it starts, whether it records (the
// monitor's enabled flag at that moment); DC_TRACE then records every library call it makes.
// Off: DC_TRACE is one relaxed atomic load + the call (no record, no clock, no allocation).
class CONTROLLER_CORE_EXPORT CallTrace
{
public:
	using Sink = void (*)(const CommandLogEntry &entry);

	static bool isEnabled() { return s_enabled.load(std::memory_order_relaxed); }
	static void setEnabled(bool enabled);

	// Payloads (refill / push blocks, attr values) keep `previewBytes()` bytes, or everything
	// when captureData() is on.
	static bool captureData();
	static void setCaptureData(bool capture);
	static int previewBytes();
	static void setPreviewBytes(int bytes);

	// True while at least one command (on any thread) is recording.
	static bool active() { return s_active.load(std::memory_order_relaxed) > 0; }

	// Receiver of finished traced commands (the command monitor); called on the executor thread.
	static void setSink(Sink sink);
	// Executor hook: hands a finished traced command to the sink (no-op if it did not record).
	static void publish(const Command *cmd);
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

	static std::atomic<bool> s_enabled;
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
