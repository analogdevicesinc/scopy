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

#ifndef PLOTMANAGERCOMBOBOX_H
#define PLOTMANAGERCOMBOBOX_H

#include <scopy-gui_export.h>
#include <channelcomponent.h>
#include <QWidget>
#include <menucombo.h>

namespace scopy {

class PlotManager;

class SCOPY_GUI_EXPORT PlotManagerCombobox : public QWidget
{
	Q_OBJECT
public:
	PlotManagerCombobox(PlotManager *man, ChannelComponent *c, QWidget *parent = nullptr);
	~PlotManagerCombobox();

public Q_SLOTS:
	void addPlot(PlotComponent *p);
	void removePlot(PlotComponent *p);
	void renamePlot(PlotComponent *p);

private Q_SLOTS:
	void renamePlotSlot();

private:
	PlotManager *m_man;
	ChannelComponent *m_ch;
	MenuCombo *m_mcombo;
	QComboBox *m_combo;

	int findIndexFromUuid(uint32_t uuid);
};
} // namespace scopy
#endif // PLOTMANAGERCOMBOBOX_H
