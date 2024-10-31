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

#ifndef PLOTINFO_H
#define PLOTINFO_H

#include "hoverwidget.h"
#include <QWidget>
#include <scopy-gui_export.h>

class QLabel;
namespace scopy {

enum InfoPosition
{
	IP_LEFT,
	IP_RIGHT,
	IP_TOP,
	IP_BOTTOM
};

class SCOPY_GUI_EXPORT PlotInfo : public QWidget
{
	Q_OBJECT
public:
	PlotInfo(QWidget *parent = nullptr);
	virtual ~PlotInfo();

	void addCustomInfo(QWidget *info, InfoPosition hpos = IP_LEFT, InfoPosition vpos = IP_BOTTOM);
	QLabel *addLabelInfo(InfoPosition hpos = IP_LEFT, InfoPosition vpos = IP_BOTTOM);

	void removeInfo(uint index, InfoPosition pos);
	QWidget *getInfo(uint index, InfoPosition pos);

protected:
	void initLayouts();

private:
	QWidget *m_parent;
	int m_margin;
	int m_spacing;

	QWidget *m_leftInfo;
	HoverWidget *m_leftHover;
	QVBoxLayout *m_leftLayout;

	QWidget *m_rightInfo;
	HoverWidget *m_rightHover;
	QVBoxLayout *m_rightLayout;
};

} // namespace scopy

#endif // PLOTINFO_H
