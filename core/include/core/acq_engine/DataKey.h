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
// Parsing is right-anchored — stage is the last segment, channel the one before
// it, and everything left over is the source. Source IDs therefore may contain
// underscores ("m2k_logic_DIO3_raw" -> source "m2k_logic", channel "DIO3"),
// while channel and stage may not.
struct DataKey
{
	QString key;

	DataKey() = default;
	explicit DataKey(const QString &k) : key(k) {}

	static DataKey raw(const QString &sourceId, const QString &channelId)
	{
		return withStage(sourceId, channelId, QStringLiteral("raw"));
	}

	static DataKey withStage(const QString &sourceId, const QString &channelId, const QString &stage)
	{
		return DataKey(sourceId + "_" + channelId + "_" + stage);
	}

	QString sourceId()  const { return head(2); }
	QString channelId() const { return tail(1); }
	QString stage()     const { return tail(0); }

	bool isRaw() const { return key.endsWith(QStringLiteral("_raw")); }

	bool operator<(const DataKey &o)  const noexcept { return key < o.key; }
	bool operator==(const DataKey &o) const noexcept { return key == o.key; }
	bool operator!=(const DataKey &o) const noexcept { return key != o.key; }

	QString toString() const { return key; }

private:
	// n-th segment counting back from the end.
	QString tail(int n) const
	{
		const QStringList p = key.split('_');
		const int i = p.size() - 1 - n;
		return i >= 0 ? p.at(i) : QString();
	}

	// Everything before the last `dropped` segments, rejoined.
	QString head(int dropped) const
	{
		const QStringList p = key.split('_');
		if(p.size() <= dropped)
			return {};
		return QStringList(p.mid(0, p.size() - dropped)).join('_');
	}
};

inline size_t qHash(const DataKey &k, size_t seed = 0) noexcept
{
	return qHash(k.key, seed);
}

} // namespace acq
} // namespace scopy

Q_DECLARE_METATYPE(scopy::acq::DataKey)
