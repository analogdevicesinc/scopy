/*
 * Copyright (c) 2020 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef PATTERNGENERATOR_API_H
#define PATTERNGENERATOR_API_H

#include "pattern_generator.h"
#include "pluginbase/apiobject.h"

namespace scopy::m2k::logic {
class PatternGenerator_API : public ApiObject
{
	Q_OBJECT

	/* channel settings */
	Q_PROPERTY(QList<int> enabledChannels READ getEnabledChannels WRITE setEnabledChannels)

	/* channel groups */
	Q_PROPERTY(QVector<QVector<int>> currentGroups READ getCurrentGroups WRITE setCurrentGroups)

	/* patterns from json */
	Q_PROPERTY(QVector<QPair<QVector<int>, QString>> enabledPatterns READ getEnabledPatterns
									WRITE setEnabledPatterns)

	/* common patterns + channels */
	Q_PROPERTY(QStringList channelNames READ getChannelNames WRITE setChannelNames)
	Q_PROPERTY(QList<double> channelHeights READ getChannelHeights WRITE setChannelHeights)
	Q_PROPERTY(QList<double> channelPosition READ getChannelPosition WRITE setChannelPosition)

	Q_PROPERTY(QString notes READ getNotes WRITE setNotes)


public:
	explicit PatternGenerator_API(logic::PatternGenerator *pattern)
		: ApiObject()
		, m_pattern(pattern) {
		qRegisterMetaType<QVector<int>>("vector(int)");
		qRegisterMetaTypeStreamOperators<QVector<int>>("vector(int)");
		qRegisterMetaType<QVector<QVector<int>>>("vector(vector(int))");
		qRegisterMetaTypeStreamOperators<QVector<QVector<int>>>("vector(vector(int))");

		qRegisterMetaType<QPair<QVector<int>, QString>>("pair(vector(int), string)");
		qRegisterMetaTypeStreamOperators<QPair<QVector<int>, QString>>("pair(vector(int), string)");
		qRegisterMetaType<QVector<QPair<QVector<int>, QString>>>("vector(pair(vector(int), string))");
		qRegisterMetaTypeStreamOperators<QVector<QPair<QVector<int>, QString>>>("vector(pair(vector(int), string))");

	}
	~PatternGenerator_API() {}

	QList<int> getEnabledChannels() const;
	void setEnabledChannels(const QList<int> &enabledChannels);

	QStringList getChannelNames() const;
	void setChannelNames(const QStringList &channelNames);

	QList<double> getChannelHeights() const;
	void setChannelHeights(const QList<double> &channelHeights);

	QList<double> getChannelPosition() const;
	void setChannelPosition(const QList<double> &channelPosition);

	QVector<QVector<int>> getCurrentGroups() const;
	void setCurrentGroups(const QVector<QVector<int> > &groups);

	QVector<QPair<QVector<int>, QString>> getEnabledPatterns() const;
	void setEnabledPatterns(const QVector<QPair<QVector<int>, QString>> &enabledPatterns);

	QString getNotes();
	void setNotes(QString);

private:
	logic::PatternGenerator *m_pattern;
};
} // namespace logic

#endif // PATTERNGENERATOR_API_H
