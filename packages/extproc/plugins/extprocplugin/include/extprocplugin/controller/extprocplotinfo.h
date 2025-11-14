/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#ifndef EXTPROCPLOTINFO_H
#define EXTPROCPLOTINFO_H

#include <QString>
#include <QVariantMap>
#include <QList>
#include <QPair>
#include <mapkeys.h>

namespace scopy::extprocplugin {

class ExtProcPlotInfo
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

	QString id;
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

	static ExtProcPlotInfo fromVariantMap(const QVariantMap &map)
	{
		ExtProcPlotInfo info;
		info.id = map.value(KeysPlotInfo::ID).toString();
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
} // namespace scopy::extprocplugin
#endif // EXTPROCPLOTINFO_H
