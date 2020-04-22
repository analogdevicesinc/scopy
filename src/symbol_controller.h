/*
 * Copyright 2016 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef M2K_SYMBOL_CONTROLLER_H
#define M2K_SYMBOL_CONTROLLER_H

#include "symbol.h"

#include <qpointer.h>
#include <qwt_widget_overlay.h>

class QwtPlot;

class SymbolController : public QObject
{
	Q_OBJECT

public:
	SymbolController(QwtPlot*);
	virtual ~SymbolController();

	const QwtPlot* plot() const;
	QwtPlot* plot();

	void attachSymbol(Symbol* symbol);
	void detachSymbol(Symbol* symbol);

	void setEnabled(bool);
	bool isEnabled() const;

	void drawOverlay(QPainter*) const;
	QRegion maskHint() const;

	virtual bool eventFilter(QObject*, QEvent*);

public Q_SLOTS:
	void updateOverlay();

private:
	void drawCursor(QPainter*) const;

	void symbolPositionUpdate(Symbol* symbol) const;

	bool pressed(const QPoint&);
	bool moved(const QPoint&);
	void released(const QPoint&);

	bool d_isEnabled;
	QPointer<QwtWidgetOverlay> d_overlay;
	QList<Symbol*> d_symbols;
	Symbol* d_selectedSymbol;
	QPoint d_prevPos;
	int axesCreated;
};

#endif /* M2K_SYMBOL_CONTROLLER_H */
