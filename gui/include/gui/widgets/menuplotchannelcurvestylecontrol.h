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
#include <QSlider>
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
	void onChannelSelected(int index);
	void onThicknessChanged(int index);
	void onStyleChanged(int index);
	void onAlphaChanged(int value);
	void updateControlsFromChannel();

private:
	void createCurveMenu(QWidget *parent);
	scopy::PlotChannel *currentChannel() const;

	QList<PlotChannel *> m_channels;
	MenuCombo *m_cbChannel;
	MenuCombo *m_cbThickness;
	MenuCombo *m_cbStyle;
	QSlider *m_alphaSlider;
};
} // namespace gui
} // namespace scopy

#endif // MENUPLOTCHANNELCURVESTYLECONTROL_H
