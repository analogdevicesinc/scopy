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

#include "core/controller_core_export.h"
#include "core/result.h"

#include <QList>
#include <QMetaType>
#include <QString>
#include <QVariant>
#include <QVariantMap>

namespace scopy {

// One argument of a traced library call. Structured (no preformatted text) so the code
// generator can re-emit it; `kind` says how the value is typed in the C call.
struct CONTROLLER_CORE_EXPORT CallArg
{
	enum Kind
	{
		Int,
		Hex,
		Bool,
		String,
		Bytes,
		Handle,
		OutLen
	};

	QString name;
	QVariant value;
	Kind kind = Int;

	static CallArg integer(const char *name, qint64 value);
	static CallArg hex(const char *name, quint64 value);
	static CallArg boolean(const char *name, bool value);
	static CallArg string(const char *name, const QString &value);
	static CallArg bytes(const char *name, const QByteArray &value);
	static CallArg handle(const char *name, const void *ptr);
	static CallArg outLen(const char *name, qint64 len);

	// "name=value" (identifier strings are shown bare).
	QString toString() const;
	bool operator==(const CallArg &other) const;
};

// One real library call issued while a command ran ("iio_attr_read_raw", "capi_i2c_transmit", ...).
struct CONTROLLER_CORE_EXPORT CallRecord
{
	QString library;     // "libiio" | "remote-bus"
	QString function;    // real C name (stable key for code generation)
	quintptr handle = 0; // object the call acts on (iio_attr*, iio_block*, capi handle, ...)
	QList<CallArg> args; // identifying args (attr/channel/device names, address, sizes, ...)
	int rc = 0;	     // C return value (negative errno on failure)
	QVariant out;	     // value read / handle returned / payload preview
	CallArg::Kind outKind = CallArg::Int;
	qint64 outSize = -1; // full size of a payload whose preview is in `out` (-1: not a payload)
	qint64 startNs = 0;  // wall clock, ns since epoch
	qint64 durationNs = 0;

	bool failed() const { return rc < 0; }
	// Same function, handle, args and rc: what the log folds into one repeated row.
	bool sameCall(const CallRecord &other) const;
	// "iio_attr_read_raw(raw, len=4096) = 5 \"1234\""
	QString toString() const;
	QString argsString() const;
	QString outString() const;
};

// Keys of CommandOrigin::scope: identifying names of the tree nodes on the origin path, for
// consumers that re-address the node without the tree (code generation). Only present keys apply.
namespace originscope {
inline constexpr auto Kind = "kind";		   // component base class of the leaf ("Attribute", "InputStream", ...)
inline constexpr auto Context = "context";	   // context name
inline constexpr auto Device = "device";	   // device name (its id when unnamed)
inline constexpr auto DeviceId = "deviceId";	   // device id
inline constexpr auto Channel = "channel";	   // channel id (Channel or ScanElement)
inline constexpr auto Output = "output";	   // bool: output channel / stream
inline constexpr auto Stream = "stream";	   // "input" | "output"
inline constexpr auto BufferIndex = "bufferIndex"; // stream buffer index
inline constexpr auto Attribute = "attribute";	   // attribute name
inline constexpr auto AttributeOf = "attributeOf"; // "context" | "device" | "channel" | "buffer"
inline constexpr auto Debug = "debug";		   // bool: debug attribute
} // namespace originscope

// Where a command comes from: the component that enqueued it (set on the GUI thread).
struct CONTROLLER_CORE_EXPORT CommandOrigin
{
	QString path;	     // component path, "/"-joined object names ("ad9361-phy/voltage0/raw")
	QString uri;	     // connection uri
	QString backendId;   // view of the connection ("iio", "capi", ...)
	quintptr object = 0; // the component node (identity only, never dereferenced)
	QVariantMap scope;   // originscope keys
};

// Everything one traced command did; delivered by the command monitor once the command finished.
struct CONTROLLER_CORE_EXPORT CommandLogEntry
{
	QString origin;
	quintptr originObject = 0;
	QString commandClass;
	QString uri;
	QString backendId;
	qint64 startNs = 0;
	qint64 durationNs = 0;
	Error error; // code 0: success
	QList<CallRecord> calls;
	QVariantMap originScope; // CommandOrigin::scope (originscope keys)

	bool failed() const { return error.code < 0; }
};

} // namespace scopy

Q_DECLARE_METATYPE(scopy::CallArg)
Q_DECLARE_METATYPE(scopy::CallRecord)
Q_DECLARE_METATYPE(scopy::CommandLogEntry)
