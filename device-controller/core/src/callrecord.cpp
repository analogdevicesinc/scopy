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

#include "core/callrecord.h"

using namespace scopy;

namespace {
bool isPrintable(const QByteArray &b)
{
	for(char c : b) {
		const auto u = static_cast<unsigned char>(c);
		if(u < 0x20 && c != '\n' && c != '\r' && c != '\t') {
			return false;
		}
		if(u >= 0x7f) {
			return false;
		}
	}
	return true;
}

QString hexOf(quint64 v) { return QStringLiteral("0x%1").arg(v, 0, 16); }

QString bytesString(const QByteArray &b, qint64 size)
{
	const qint64 total = size >= 0 ? size : b.size();
	if(!b.isEmpty() && isPrintable(b) && b.size() == total) {
		return QLatin1Char('"') + QString::fromLatin1(b) + QLatin1Char('"');
	}
	QString s = QStringLiteral("[%1 B").arg(total);
	if(!b.isEmpty()) {
		s += QStringLiteral(": ") + QString::fromLatin1(b.toHex(' '));
		if(b.size() < total) {
			s += QStringLiteral(" …");
		}
	}
	return s + QLatin1Char(']');
}

QString valueString(const QVariant &v, CallArg::Kind kind, qint64 size = -1)
{
	switch(kind) {
	case CallArg::Hex:
	case CallArg::Handle:
		return hexOf(v.toULongLong());
	case CallArg::Bool:
		return v.toBool() ? QStringLiteral("true") : QStringLiteral("false");
	case CallArg::String: {
		QString s = v.toString();
		if(size >= 0 && size > s.toUtf8().size()) {
			s += QStringLiteral("…");
		}
		return QLatin1Char('"') + s + QLatin1Char('"');
	}
	case CallArg::Bytes:
		return bytesString(v.toByteArray(), size);
	case CallArg::Int:
	case CallArg::OutLen:
		return v.toString();
	}
	return v.toString();
}
} // namespace

CallArg CallArg::integer(const char *name, qint64 value) { return {QString::fromLatin1(name), value, Int}; }

CallArg CallArg::hex(const char *name, quint64 value) { return {QString::fromLatin1(name), value, Hex}; }

CallArg CallArg::boolean(const char *name, bool value) { return {QString::fromLatin1(name), value, Bool}; }

CallArg CallArg::string(const char *name, const QString &value) { return {QString::fromLatin1(name), value, String}; }

CallArg CallArg::bytes(const char *name, const QByteArray &value) { return {QString::fromLatin1(name), value, Bytes}; }

CallArg CallArg::handle(const char *name, const void *ptr)
{
	return {QString::fromLatin1(name), QVariant::fromValue(quintptr(ptr)), Handle};
}

CallArg CallArg::outLen(const char *name, qint64 len) { return {QString::fromLatin1(name), len, OutLen}; }

QString CallArg::toString() const
{
	if(kind == String) {
		return value.toString();
	}
	return name + QLatin1Char('=') + valueString(value, kind);
}

bool CallArg::operator==(const CallArg &other) const
{
	return kind == other.kind && name == other.name && value == other.value;
}

bool CallRecord::sameCall(const CallRecord &other) const
{
	return function == other.function && handle == other.handle && rc == other.rc && library == other.library &&
		args == other.args;
}

QString CallRecord::argsString() const
{
	QStringList parts;
	parts.reserve(args.size());
	for(const CallArg &a : args) {
		parts << a.toString();
	}
	return parts.join(QStringLiteral(", "));
}

QString CallRecord::outString() const
{
	if(!out.isValid()) {
		return {};
	}
	return valueString(out, outKind, outSize);
}

QString CallRecord::toString() const
{
	QString s = function + QLatin1Char('(') + argsString() + QStringLiteral(") = ") + QString::number(rc);
	const QString o = outString();
	if(!o.isEmpty()) {
		s += QLatin1Char(' ') + o;
	}
	return s;
}
