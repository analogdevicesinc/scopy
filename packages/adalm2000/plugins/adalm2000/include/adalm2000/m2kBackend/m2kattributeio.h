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
 */

#pragma once

#include <component/attribute.h>
#include <component/attributereader.h>
#include <component/attributewriter.h>
#include <component/navigation.h>

#include <qcoro/qcorotask.h>

#include <QString>

#include <optional>
#include <stdexcept>

// Blocking attribute access for source blocks, which run on the engine's worker thread.
namespace scopy::adalm2000::attr {

inline component::Attribute *find(QObject *node, const char *name)
{
	return node ? component::attributeByName(node, QString::fromLatin1(name)) : nullptr;
}

inline QString describe(const Error &e)
{
	const QString s = e.errorString();
	return s.isEmpty() ? QStringLiteral("error %1").arg(e.code) : s;
}

[[noreturn]] inline void fail(const QString &msg) { throw std::runtime_error(msg.toStdString()); }

inline QString path(QObject *node, const char *name)
{
	return QStringLiteral("%1/%2").arg(node ? node->objectName() : QStringLiteral("?"), QString::fromLatin1(name));
}

// Throws with the node, attribute and backend error, so a failed start says which step broke
inline void write(QObject *node, const char *name, const QString &value)
{
	const QString what = path(node, name);
	component::Attribute *attr = find(node, name);
	if(!attr || !attr->writeCapability()) {
		fail(what + QStringLiteral(" is missing or not writable"));
	}
	const auto resp = QCoro::waitFor(attr->writeCapability()->writeAsync(value));
	if(!resp) {
		fail(QStringLiteral("writing %1 = %2 failed: %3").arg(what, value, describe(resp.error())));
	}
}

// nullopt with `why` set to the node, attribute and backend error, for the caller to report
inline std::optional<QString> readString(QObject *node, const char *name, QString &why)
{
	component::Attribute *attr = find(node, name);
	if(!attr || !attr->readCapability()) {
		why = path(node, name) + QStringLiteral(" is missing or not readable");
		return std::nullopt;
	}
	const auto resp = QCoro::waitFor(attr->readCapability()->readAsync());
	if(!resp) {
		why = QStringLiteral("reading %1 failed: %2").arg(path(node, name), describe(resp.error()));
		return std::nullopt;
	}
	return QString::fromUtf8(resp.value()).trimmed();
}

inline std::optional<double> readDouble(QObject *node, const char *name, QString &why)
{
	const std::optional<QString> s = readString(node, name, why);
	if(!s) {
		return std::nullopt;
	}
	bool ok = false;
	const double value = s->toDouble(&ok);
	if(!ok) {
		why = QStringLiteral("%1 reads \"%2\", not a number").arg(path(node, name), *s);
		return std::nullopt;
	}
	return value;
}

} // namespace scopy::adalm2000::attr
