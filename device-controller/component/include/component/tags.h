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

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>

namespace scopy::component {

namespace tags {
inline constexpr auto SampleRate = "sample-rate";
inline constexpr auto TriggerLevel = "trigger-level";
inline constexpr auto Offset = "offset";
inline constexpr auto Enable = "enable";
} // namespace tags

inline QStringList componentTags(const QObject *o) { return o->property("tags").toStringList(); }

inline bool hasTag(const QObject *o, const QString &tag) { return componentTags(o).contains(tag); }

inline void addTag(QObject *o, const QString &tag)
{
	QStringList t = componentTags(o);
	if(!t.contains(tag)) {
		t.append(tag);
		o->setProperty("tags", t);
	}
}

template <typename T>
QList<T *> findByTag(const QObject *root, const QString &tag, bool recursive = true)
{
	const auto opt = recursive ? Qt::FindChildrenRecursively : Qt::FindDirectChildrenOnly;
	QList<T *> out;
	for(T *c : root->findChildren<T *>(QString(), opt)) {
		if(hasTag(c, tag)) {
			out.append(c);
		}
	}
	return out;
}

inline QList<QObject *> findByTag(const QObject *root, const QString &tag, bool recursive = true)
{
	return findByTag<QObject>(root, tag, recursive);
}

} // namespace scopy::component
