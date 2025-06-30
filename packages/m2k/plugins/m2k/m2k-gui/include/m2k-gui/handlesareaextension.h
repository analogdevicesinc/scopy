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

#ifndef HANDLESAREAEXTENSION_H
#define HANDLESAREAEXTENSION_H

#include "scopy-m2k-gui_export.h"
#include <QPainter>

class QwtPlot;

class SCOPY_M2K_GUI_EXPORT HandlesAreaExtension
{
public:
	HandlesAreaExtension(QwtPlot *plot);
	virtual ~HandlesAreaExtension() = default;

	// Override to provide extensions drawing behaviour
	// return false if other extensions will be drawn afther this one
	// return true if this will be the last extension to be drawn no matter
	// the number of extensions installed
	virtual bool draw(QPainter *painter, QWidget *owner) = 0;

protected:
	QwtPlot *m_plot;
};

class SCOPY_M2K_GUI_EXPORT XBottomRuller : public HandlesAreaExtension
{
public:
	XBottomRuller(QwtPlot *plot);
	virtual ~XBottomRuller() = default;

	virtual bool draw(QPainter *painter, QWidget *owner) Q_DECL_OVERRIDE;
};

class SCOPY_M2K_GUI_EXPORT YLeftRuller : public HandlesAreaExtension
{
public:
	YLeftRuller(QwtPlot *plot);
	virtual ~YLeftRuller() = default;

	virtual bool draw(QPainter *painter, QWidget *owner) Q_DECL_OVERRIDE;
};

class SCOPY_M2K_GUI_EXPORT XTopRuller : public HandlesAreaExtension
{
public:
	XTopRuller(QwtPlot *plot);
	virtual ~XTopRuller() = default;

	virtual bool draw(QPainter *painter, QWidget *owner) Q_DECL_OVERRIDE;
};

#endif // HANDLESAREAEXTENSION_H
