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

// Identifies one DataStore stream as "<sourceId>_<channelId>_<stage>".
//
// The joined string is the identity (equality, hashing, combo boxes, settings); the
// three parts are split out once, at construction. Source ids and stage names may
// not contain '_' — the channel id may, since parsing takes the first segment as
// the source and the last as the stage ("adxl355_accel_x_raw" -> "adxl355",
// "accel_x", "raw").
class DataKey
{
public:
	DataKey() = default;
	explicit DataKey(const QString &k)
		: m_key(k)
	{
		const QStringList p = k.split('_');
		if(p.size() >= 3) {
			m_source = p.first();
			m_stage = p.last();
			m_channel = QStringList(p.mid(1, p.size() - 2)).join('_');
		} else if(!p.isEmpty()) {
			m_source = p.first();
			m_stage = p.size() == 2 ? p.last() : QString();
		}
	}

	static DataKey raw(const QString &sourceId, const QString &channelId)
	{
		return withStage(sourceId, channelId, QStringLiteral("raw"));
	}

	static DataKey withStage(const QString &sourceId, const QString &channelId, const QString &stage)
	{
		Q_ASSERT_X(!sourceId.contains('_') && !stage.contains('_'), "DataKey",
			   "source ids and stage names may not contain '_'");
		DataKey k;
		k.m_key = sourceId + '_' + channelId + '_' + stage;
		k.m_source = sourceId;
		k.m_channel = channelId;
		k.m_stage = stage;
		return k;
	}

	const QString &sourceId() const { return m_source; }
	const QString &channelId() const { return m_channel; }
	const QString &stage() const { return m_stage; }
	bool isRaw() const { return m_stage == QLatin1String("raw"); }
	bool isEmpty() const { return m_key.isEmpty(); }

	const QString &toString() const { return m_key; }

	bool operator<(const DataKey &o) const noexcept { return m_key < o.m_key; }
	bool operator==(const DataKey &o) const noexcept { return m_key == o.m_key; }
	bool operator!=(const DataKey &o) const noexcept { return m_key != o.m_key; }

private:
	QString m_key;
	QString m_source;
	QString m_channel;
	QString m_stage;
};

inline size_t qHash(const DataKey &k, size_t seed = 0) noexcept { return qHash(k.toString(), seed); }

} // namespace acq
} // namespace scopy

Q_DECLARE_METATYPE(scopy::acq::DataKey)
