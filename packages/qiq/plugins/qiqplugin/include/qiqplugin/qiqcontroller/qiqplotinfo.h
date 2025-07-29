#ifndef QIQPLOTINFO_H
#define QIQPLOTINFO_H

#include <QString>
#include <QVariantMap>
#include <QList>
#include <QPair>
#include <mapkeys.h>

namespace scopy::qiqplugin {

class QIQPlotInfo
{
public:
	enum PlotType
	{
		TIME,
		FREQ,
		XY,
		INVALID_TYPE
	};
	int id = -1;
	QString title;
	QString xLabel;
	QString yLabel;
	PlotType type;
	bool xyValues = false;
	QList<QPair<int, int>> channels;

	static PlotType plotTypeFromString(const QString &typeStr)
	{
		if(typeStr == "time") {
			return TIME;
		}
		if(typeStr == "freq") {
			return FREQ;
		}
		if(typeStr == "xy") {
			return XY;
		}
		return INVALID_TYPE;
	}

	static QIQPlotInfo fromVariantMap(const QVariantMap &map)
	{
		QIQPlotInfo info;
		info.id = map.value(KeysPlotInfo::ID, -1).toInt();
		info.title = map.value(KeysPlotInfo::TITLE).toString();
		info.xLabel = map.value(KeysPlotInfo::XLABEL).toString();
		info.yLabel = map.value(KeysPlotInfo::YLABEL).toString();
		info.type = plotTypeFromString(map.value(KeysPlotInfo::TYPE).toString());
		info.xyValues = map.value(KeysPlotInfo::XYVALUES).toBool();

		const QVariantList chList = map.value(KeysPlotInfo::CHANNELS).toList();
		for(const QVariant &chVar : chList) {
			QVariantMap chMap = chVar.toMap();
			int x = chMap.contains(KeysPlotInfo::CH_X) ? chMap.value(KeysPlotInfo::CH_X).toInt() : -1;
			int y = chMap.contains(KeysPlotInfo::CH_Y) ? chMap.value(KeysPlotInfo::CH_Y).toInt() : -1;
			info.channels.append({x, y});
		}

		return info;
	}

	bool isValid() const
	{
		if(id < 0 || title.isEmpty() || xLabel.isEmpty() || yLabel.isEmpty() || type == INVALID_TYPE) {
			return false;
		}

		if(channels.isEmpty()) {
			return false;
		}

		for(const auto &ch : channels) {
			if(xyValues) {
				if(ch.first < 0 || ch.second < 0) {
					return false;
				}
			} else {
				if(ch.second < 0) {
					return false;
				}
			}
		}

		return true;
	}
};
} // namespace scopy::qiqplugin
#endif // QIQPLOTINFO_H
