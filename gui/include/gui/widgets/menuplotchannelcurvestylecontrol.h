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

#ifndef MENUPLOTCHANNELCURVESTYLECONTROL_H
#define MENUPLOTCHANNELCURVESTYLECONTROL_H

#include <QWidget>
#include "scopy-gui_export.h"
#include <menucombo.h>

namespace scopy {
class PlotChannel;
namespace gui {

class SCOPY_GUI_EXPORT MenuPlotChannelCurveStyleControl : public QWidget
{
	Q_OBJECT
public:
	MenuPlotChannelCurveStyleControl(QWidget *parent);
	~MenuPlotChannelCurveStyleControl();

public Q_SLOTS:
	void addChannels(PlotChannel *c);
	void removeChannels(PlotChannel *c);

private Q_SLOTS:
	void setStyleSlot();
	void setThicknessSlot();

private:
	void createCurveMenu(QWidget *parent);
	QList<PlotChannel *> m_channels;
	MenuCombo *cbThicknessW;
	MenuCombo *cbStyleW;
};
} // namespace gui
} // namespace scopy

#endif // MENUPLOTCHANNELCURVESTYLECONTROL_H
