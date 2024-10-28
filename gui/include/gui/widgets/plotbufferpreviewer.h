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

#ifndef PLOTBUFFERPREVIEWER_H
#define PLOTBUFFERPREVIEWER_H

#include "scopy-gui_export.h"
#include <QWidget>
#include "plotwidget.h"
#include "buffer_previewer.hpp"

namespace scopy {

class SCOPY_GUI_EXPORT PlotBufferPreviewer : public QWidget
{
	Q_OBJECT
public:
	explicit PlotBufferPreviewer(PlotWidget *p, BufferPreviewer *b, QWidget *parent = nullptr);
	~PlotBufferPreviewer();

	void setManualDataLimits(bool enabled);
	void updateDataLimits(double min, double max);

public Q_SLOTS:
	void updateBufferPreviewer();

private:
	double m_bufferDataLimitMin;
	double m_bufferDataLimitMax;
	bool m_manualDataLimits;

	void setupBufferPreviewer();
	PlotWidget *m_plot;
	BufferPreviewer *m_bufferPreviewer;
	double m_lastMin;
};

} // namespace scopy

#endif // PLOTBUFFERPREVIEWER_H
