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

// Blocking attribute access for source blocks, which run on the engine's worker thread.
namespace scopy::adalm2000::attr {

inline component::Attribute *find(QObject *node, const char *name)
{
	return node ? component::attributeByName(node, QString::fromLatin1(name)) : nullptr;
}

inline bool write(component::Attribute *attr, const QString &value)
{
	return attr && attr->writeCapability() && QCoro::waitFor(attr->writeCapability()->writeAsync(value));
}

inline std::optional<double> readDouble(component::Attribute *attr)
{
	if(!attr || !attr->readCapability()) {
		return std::nullopt;
	}
	const auto resp = QCoro::waitFor(attr->readCapability()->readAsync());
	bool ok = false;
	const double value = resp ? QString::fromUtf8(resp.value()).toDouble(&ok) : 0.0;
	return ok ? std::optional<double>(value) : std::nullopt;
}

// Fire-and-forget, for a GUI-thread setter that must not block.
inline void post(component::Attribute *attr, const QString &value)
{
	if(attr && attr->writeCapability()) {
		attr->writeCapability()->writeAsync(value);
	}
}

} // namespace scopy::adalm2000::attr
