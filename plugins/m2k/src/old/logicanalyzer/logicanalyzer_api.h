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


#ifndef LOGICANALYZER_API_H
#define LOGICANALYZER_API_H

#include "logic_analyzer.h"
#include "pluginbase/apiobject.h"

namespace adiscope::m2k {
namespace logic {
class LogicAnalyzer_API : public ApiObject
{
	Q_OBJECT

	/*cursor settings*/
	Q_PROPERTY(bool cursors READ hasCursors WRITE setCursors);
	Q_PROPERTY(int cursors_position READ getCursorsPosition
		  WRITE setCursorsPosition)
	Q_PROPERTY(int cursors_transparency READ getCursorsTransparency
		  WRITE setCursorsTransparency)

	/* sweep settings */
	Q_PROPERTY(bool streamOneShot READ getStreamOrOneShot WRITE setStreamOrOneShot)
	Q_PROPERTY(double sampleRate READ getSampleRate WRITE setSampleRate)
	Q_PROPERTY(int bufferSize READ getBufferSize WRITE setBufferSize)
	Q_PROPERTY(int delay READ getDelay WRITE setDelay)

	/* channel settings */
	Q_PROPERTY(QList<int> enabledChannels READ getEnabledChannels WRITE setEnabledChannels)

	/* decoders */
	Q_PROPERTY(QStringList enabledDecoders READ getEnabledDecoders WRITE setEnabledDecoders)
	Q_PROPERTY(QList<QList<QPair<int, int>>> assignedDecoderChannels READ getAssignedDecoderChannels
								WRITE setAssignedDecoderChannels)
	Q_PROPERTY(QList<QStringList> decoderStack READ getDecoderStack WRITE setDecoderStack)
	Q_PROPERTY(QList<QStringList> decoderSettings READ getDecoderSettings WRITE setDecoderSettings)

	/* common decoders + channels */
	Q_PROPERTY(QStringList channelNames READ getChannelNames WRITE setChannelNames)
	Q_PROPERTY(QList<double> channelHeights READ getChannelHeights WRITE setChannelHeights)
	Q_PROPERTY(QList<double> channelPosition READ getChannelPosition WRITE setChannelPosition)

	/* groups */
	Q_PROPERTY(QVector<QVector<int>> currentGroups READ getCurrentGroups WRITE setCurrentGroups)

	/* notes */
	Q_PROPERTY(QString notes READ getNotes WRITE setNotes)

public:
	explicit LogicAnalyzer_API(logic::LogicAnalyzer *logic):
	ApiObject(), m_logic(logic) {
		// Register type. TODO: maybe a cleaner way of doing this
		// QVariant needs qRegisterMetaTypeStreamOperators for serialization/deserialization
		qRegisterMetaType<QPair<int, int>>("pair");
		qRegisterMetaTypeStreamOperators<QPair<int, int>>("pair");
		qRegisterMetaType<QList<QPair<int, int>>>("list(pair)");
		qRegisterMetaTypeStreamOperators<QList<QPair<int, int>>>("list(pair)");
		qRegisterMetaType<QList<QList<QPair<int, int>>>>("list(list(pair))");
		qRegisterMetaTypeStreamOperators<QList<QList<QPair<int, int>>>>("list(list(pair))");

		qRegisterMetaType<QList<QStringList>>("list(stringlist)");
		qRegisterMetaTypeStreamOperators<QList<QStringList>>("list(stringlist)");

		qRegisterMetaType<QVector<int>>("vector(int)");
		qRegisterMetaTypeStreamOperators<QVector<int>>("vector(int)");
		qRegisterMetaType<QVector<QVector<int>>>("vector(vector(int))");
		qRegisterMetaTypeStreamOperators<QVector<QVector<int>>>("vector(vector(int))");
	}
	~LogicAnalyzer_API() {}

	double getSampleRate() const;
	void setSampleRate(double sampleRate);

	int getBufferSize() const;
	void setBufferSize(int bufferSize);

	QList<int> getEnabledChannels() const;
	void setEnabledChannels(const QList<int> &enabledChannels);

	QStringList getEnabledDecoders() const;
	void setEnabledDecoders(const QStringList &decoders);

	bool getStreamOrOneShot() const;
	void setStreamOrOneShot(bool streamOrOneShot);

	int getDelay() const;
	void setDelay(int delay);

	QStringList getChannelNames() const;
	void setChannelNames(const QStringList &channelNames);

	QList<double> getChannelHeights() const;
	void setChannelHeights(const QList<double> &channelHeights);

	QList<double> getChannelPosition() const;
	void setChannelPosition(const QList<double> &channelPosition);

	QList<QList<QPair<int, int>>> getAssignedDecoderChannels() const;
	void setAssignedDecoderChannels(const QList<QList<QPair<int, int>>> &assignedDecoderChannels);

	QList<QStringList> getDecoderStack() const;
	void setDecoderStack(const QList<QStringList> &decoderStack);

	QList<QStringList> getDecoderSettings() const;
	void setDecoderSettings(const QList<QStringList> &decoderSettings);

	QVector<QVector<int>> getCurrentGroups() const;
	void setCurrentGroups(const QVector<QVector<int> > &groups);

	QString getNotes();
	void setNotes(QString str);

private:
	logic::LogicAnalyzer *m_logic;

	bool hasCursors() const;
	void setCursors(bool en);

	int getCursorsPosition() const;
	void setCursorsPosition(int val);

	int getCursorsTransparency() const;
	void setCursorsTransparency(int val);
};
}
}

#endif // LOGICANALYZER_API_H
