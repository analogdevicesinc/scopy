/*
 * Copyright (c) 2019 Analog Devices Inc.
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

#ifndef M2K_SYMBOL_H
#define M2K_SYMBOL_H

#include "scopy-gui_export.h"

#include <qwt_painter.h>
#include <qwt_plot.h>

class SCOPY_GUI_EXPORT Symbol : public QObject
{
	Q_OBJECT

public:
	Symbol(QObject *parent, const QSize &size, QwtAxisId fixedAxis, QwtAxisId mobileAxis,
	       bool opposedToFixed = false, bool floats = false);
	virtual ~Symbol();

	const QwtPlot *plot() const;
	QwtPlot *plot();

	void setSurface(const QRect &rect);
	QRect surface();

	void setAnchor(const QPoint &anchor);
	QPoint anchor();

	void setPlotCoord(const QPointF &pos);
	QPointF plotCoord();

	void setSelected(bool selected);
	bool isSelected();

	QwtAxisId fixedAxis() const;
	QwtAxisId mobileAxis() const;

	void setMobileAxis(QwtAxisId);

	void setStepSize(double);
	double stepSize();

	bool opposedToFixed();

	bool floats();

	void setCanLeavePlot(bool);
	bool canLeavePlot();

	void setVisible(bool en);
	bool isVisible();

	void setPen(const QPen &);
	const QPen &pen();

	QPointF invTransform(const QPointF &) const;
	QPointF transform(const QPointF &) const;

	void updateSurfacePos();
	void updatePlotCoordFromSurfacePos();

	virtual void draw(QPainter *painter) const = 0;
	virtual bool moveWith(double plotDeltaX, double plotDeltaY) = 0;

	void triggerMove();
	void setFixedAxis(QwtAxisId newAxis);
Q_SIGNALS:
	void positionChanged(double x, double y);
	void pixelPositionChanged(int x, int y);
	void selected(bool selected);
	void visibilityChanged(bool en);

protected Q_SLOTS:
	void onFixedScaleChanged();
	void onMobileScaleChanged();

protected:
	QRect d_surface;
	QPoint d_anchor; // sets which point of d_surface to point exactly to d_plotCoord. Default point is: top-left of
			 // d_surface (d_anchor = (0, 0))
	QPointF d_plotCoord; // Position of symbol in plot coordinates
	bool d_selected;

	QwtAxisId d_fixedAxis;	// The axis on which the symbol must not move
	QwtAxisId d_mobileAxis; // The axis on which the symbol can be shifted
	double d_stepSize;  // Symbol position can take only values that are a multiple of d_stepSize. If d_stepSize = 0
			    // symbol pos can take any value.
	bool d_oppToFixed;  // If symbol should be placed diametrically opposed to the fixed axis
	bool d_floats;	    // If symbols floats on top of the plot or is attached to the plot
	bool d_within_plot; // If symbol position can only be within the plot visible area

	bool d_visible;

	QPen d_pen;
};

class SCOPY_GUI_EXPORT VertDebugSymbol : public Symbol
{
	Q_OBJECT

public:
	VertDebugSymbol(QObject *parent, const QSize &size, bool opposedToFixed = false, bool floats = false);

	void draw(QPainter *painter) const;

	virtual bool moveWith(double plotDeltaX, double plotDeltaY);
	double getPosition();

Q_SIGNALS:
	void positionChanged(double);
	void pixelPositionChanged(int);

public Q_SLOTS:
	void setPosition(double vertPos);
	void setPixelPosition(int vertPos);

private Q_SLOTS:
	void onBasePositionChanged(double, double);
	void onBasePixelPositionChanged(int, int);
};

class SCOPY_GUI_EXPORT HorizDebugSymbol : public Symbol
{
	Q_OBJECT

public:
	HorizDebugSymbol(QObject *parent, const QSize &size, bool opposedToFixed = false, bool floats = false);

	void draw(QPainter *painter) const;

	virtual bool moveWith(double plotDeltaX, double plotDeltaY);
	double getPosition();

Q_SIGNALS:
	void positionChanged(double);
	void pixelPositionChanged(int);

public Q_SLOTS:
	void setPosition(double horizPos);
	void setPixelPosition(int horizPos);

private Q_SLOTS:
	void onBasePositionChanged(double, double);
	void onBasePixelPositionChanged(int, int);
};

class SCOPY_GUI_EXPORT TriggerLevelCursor : public VertDebugSymbol
{
	Q_OBJECT

public:
	TriggerLevelCursor(QObject *parent, const QPixmap &pixmap);

	void draw(QPainter *painter) const;

private:
	QPixmap d_cursorPixmap;
};

class SCOPY_GUI_EXPORT TriggerDelayCursor : public HorizDebugSymbol
{
	Q_OBJECT

public:
	TriggerDelayCursor(QObject *parent, const QPixmap &pixmap);

	void draw(QPainter *painter) const;

private:
	QPixmap d_cursorPixmap;
};

class SCOPY_GUI_EXPORT HorizBar : public VertDebugSymbol
{
	Q_OBJECT

public:
	HorizBar(QObject *parent, bool floats = false);

	void draw(QPainter *painter) const;
};

class SCOPY_GUI_EXPORT VertBar : public HorizDebugSymbol
{
	Q_OBJECT

public:
	VertBar(QObject *parent, bool floats = false);

	void draw(QPainter *painter) const;
};

#endif /* M2K_SYMBOL_H */
