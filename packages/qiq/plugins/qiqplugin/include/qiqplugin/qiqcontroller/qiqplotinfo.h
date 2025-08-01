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
		PLOT_WIDGET,
		INVALID_TYPE
	};

	typedef struct
	{
		QString x;
		QString y;
	} PlotInfoCh;

	int id = -1;
	QString title;
	QString xLabel;
	QString yLabel;
	PlotType type;
	QList<PlotInfoCh> channels;
	QStringList flags;

	static PlotType plotTypeFromString(const QString &typeStr)
	{
		if(typeStr == "plot") {
			return PLOT_WIDGET;
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
		info.flags = map.value(KeysPlotInfo::FLAGS).toStringList();

		const QVariantList chList = map.value(KeysPlotInfo::CHANNELS).toList();
		for(const QVariant &chVar : chList) {
			QVariantList chPair = chVar.toList();
			if(chPair.size() >= 2) {
				QString x = chPair[0].toString();
				QString y = chPair[1].toString();
				info.channels.append({x, y});
			}
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
			if(ch.x.isEmpty() || ch.y.isEmpty()) {
				return false;
			}
		}

		return true;
	}
};
} // namespace scopy::qiqplugin
#endif // QIQPLOTINFO_H
