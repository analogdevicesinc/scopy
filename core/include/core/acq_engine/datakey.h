/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#include <QMetaType>
#include <QString>
#include <QStringList>

namespace scopy {
namespace acq {

// Identifies one data stream in the DataStore as "<sourceId>_<channelId>_<stage>".
//
// The string is the identity: DataKey round-trips through GUI combo boxes and
// settings as plain text, so equality and hashing are string-based and the
// components are parsed back out on demand.
//
// The source is the first segment, the stage the last, and the channel is
// everything in between ("adxl355_accel_x_raw" -> source "adxl355", channel
// "accel_x", stage "raw"). Channel IDs may therefore contain underscores, while
// source IDs and stage names may not — a source named "m2k_logic" would parse as
// source "m2k" with "logic_" pulled into every one of its channel IDs.
struct DataKey
{
	QString key;

	DataKey() = default;
	explicit DataKey(const QString &k)
		: key(k)
	{}

	static DataKey raw(const QString &sourceId, const QString &channelId)
	{
		return withStage(sourceId, channelId, QStringLiteral("raw"));
	}

	static DataKey withStage(const QString &sourceId, const QString &channelId, const QString &stage)
	{
		return DataKey(sourceId + "_" + channelId + "_" + stage);
	}

	QString sourceId() const { return segment(0); }
	QString channelId() const { return middle(); }
	QString stage() const { return segment(-1); }

	bool isRaw() const { return key.endsWith(QStringLiteral("_raw")); }

	bool operator<(const DataKey &o) const noexcept { return key < o.key; }
	bool operator==(const DataKey &o) const noexcept { return key == o.key; }
	bool operator!=(const DataKey &o) const noexcept { return key != o.key; }

	QString toString() const { return key; }

private:
	// i-th segment, negative counting back from the end (-1 is the last).
	QString segment(int i) const
	{
		const QStringList p = key.split('_');
		const int idx = i < 0 ? p.size() + i : i;
		return (idx >= 0 && idx < p.size()) ? p.at(idx) : QString();
	}

	// Everything between the first and last segments, rejoined.
	QString middle() const
	{
		const QStringList p = key.split('_');
		if(p.size() <= 2)
			return {};
		return QStringList(p.mid(1, p.size() - 2)).join('_');
	}
};

inline size_t qHash(const DataKey &k, size_t seed = 0) noexcept { return qHash(k.key, seed); }

} // namespace acq
} // namespace scopy

Q_DECLARE_METATYPE(scopy::acq::DataKey)
