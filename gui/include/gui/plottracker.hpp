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

#ifndef PLOTCHANNELTRACKER_H
#define PLOTCHANNELTRACKER_H

#include "scopy-gui_export.h"
#include <basictracker.hpp>

namespace scopy {

class PlotChannel;
class PlotWidget;
class ChannelTracker
{
public:
	explicit ChannelTracker();
	~ChannelTracker();

	PlotChannel *channel;
	BasicTracker *tracker;
};

class SCOPY_GUI_EXPORT PlotTracker : public QObject
{
	Q_OBJECT

public:
	explicit PlotTracker(PlotWidget *plot, QList<PlotChannel *> *channels = new QList<PlotChannel *>());
	~PlotTracker();

	void setEnabled(bool en);
	bool isEnabled();

	void addChannel(PlotChannel *ch);
	void removeChannel(PlotChannel *ch);

protected:
	void init(QList<PlotChannel *> *channels);
	ChannelTracker *createTracker(PlotChannel *ch);

protected Q_SLOTS:
	void onChannelSelected(PlotChannel *ch);

private:
	bool m_en;
	PlotWidget *m_plot;
	QSet<ChannelTracker *> *m_trackers;
};
} // namespace scopy

#endif // PLOTCHANNELTRACKER_H
