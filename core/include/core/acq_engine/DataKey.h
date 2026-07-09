#pragma once

#include <QMetaType>
#include <QString>
#include <QStringList>

namespace scopy {
namespace acq {

struct DataKey
{
	QString key;

	DataKey() = default;
	explicit DataKey(const QString &k) : key(k) {}

	static DataKey raw(const QString &sourceId, const QString &channelId)
	{
		return DataKey(sourceId + "_" + channelId + "_raw");
	}

	static DataKey withStage(const QString &sourceId, const QString &channelId, const QString &stage)
	{
		return DataKey(sourceId + "_" + channelId + "_" + stage);
	}

	static DataKey withStages(const QString &sourceId, const QString &channelId, const QStringList &stages)
	{
		return DataKey(sourceId + "_" + channelId + "_" + stages.join("_"));
	}

	QString sourceId()   const { return parts().value(0); }
	QString channelId()  const { return parts().value(1); }
	QString stages()     const
	{
		const QStringList p = parts();
		if(p.size() < 3) return {};
		return QStringList(p.mid(2)).join("_");
	}
	QString firstStage() const { return parts().value(2); }
	// A key is "raw" if its final stage segment is "raw". Checking the
	// suffix (rather than parts().value(2)) keeps this correct when the
	// sourceId itself contains underscores, e.g. "m2k_logic".
	bool    isRaw()      const { return key.endsWith(QStringLiteral("_raw")); }

	bool operator<(const DataKey &o)  const noexcept { return key < o.key; }
	bool operator==(const DataKey &o) const noexcept { return key == o.key; }
	bool operator!=(const DataKey &o) const noexcept { return key != o.key; }

	QString toString() const { return key; }

private:
	QStringList parts() const { return key.split('_'); }
};

inline size_t qHash(const DataKey &k, size_t seed = 0) noexcept
{
	return qHash(k.key, seed);
}

} // namespace acq
} // namespace scopy

Q_DECLARE_METATYPE(scopy::acq::DataKey)
