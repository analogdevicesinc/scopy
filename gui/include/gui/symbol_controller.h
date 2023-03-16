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

#ifndef M2K_SYMBOL_CONTROLLER_H
#define M2K_SYMBOL_CONTROLLER_H

#include <qpointer.h>
#include <qwt_widget_overlay.h>
#include "symbol.h"
#include "scopygui_export.h"

class QwtPlot;

class SCOPYGUI_EXPORT SymbolController: public QObject
{
	Q_OBJECT

public:
	SymbolController(QwtPlot *);
	virtual ~SymbolController();

	const QwtPlot *plot() const;
	QwtPlot *plot();

	void attachSymbol(Symbol *symbol);
	void detachSymbol(Symbol *symbol);

	void setEnabled(bool);
	bool isEnabled() const;

	void drawOverlay(QPainter *) const;
	QRegion maskHint() const;

	virtual bool eventFilter(QObject *, QEvent *);

public Q_SLOTS:
	void updateOverlay();

private:
	void drawCursor(QPainter *) const;

	void symbolPositionUpdate(Symbol *symbol) const;

	bool pressed(const QPoint &);
	bool moved(const QPoint &);
	void released(const QPoint &);

	bool d_isEnabled;
	QPointer<QwtWidgetOverlay> d_overlay;
	QList<Symbol *> d_symbols;
	Symbol *d_selectedSymbol;
	QPoint d_prevPos;
	int axesCreated;
};

#endif /* M2K_SYMBOL_CONTROLLER_H */
