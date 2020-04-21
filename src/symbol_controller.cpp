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

#include "symbol_controller.h"

#include "qwt_painter.h"

#include <QDebug>
#include <QMouseEvent>
#include <qevent.h>
#include <qpainter.h>
#include <qwt_plot.h>

class Overlay : public QwtWidgetOverlay {
public:
	Overlay(QWidget *parent, SymbolController *symbolController)
		: QwtWidgetOverlay(parent), d_symbolCtrl(symbolController) {}

protected:
	virtual void drawOverlay(QPainter *painter) const {
		d_symbolCtrl->drawOverlay(painter);
	}

	virtual QRegion maskHint() const { return d_symbolCtrl->maskHint(); }

private:
	SymbolController *d_symbolCtrl;
};

SymbolController::SymbolController(QwtPlot *plot)
	: QObject(plot)
	, d_isEnabled(false)
	, d_selectedSymbol(NULL)
	, axesCreated(0) {
	setEnabled(true);

	d_overlay = new Overlay(plot->canvas(), this);
}

SymbolController::~SymbolController() {
	// d_overlay gets destroyed with parent
}

QwtPlot *SymbolController::plot() { return static_cast<QwtPlot *>(parent()); }

const QwtPlot *SymbolController::plot() const {
	return static_cast<const QwtPlot *>(parent());
}

void SymbolController::attachSymbol(Symbol *symbol) {
	if (d_symbols.indexOf(symbol) == -1) {
		d_symbols.push_back(symbol);
		QObject::connect((const QObject *)symbol,
				 SIGNAL(positionChanged(double)), this,
				 SLOT(updateOverlay()));
		QObject::connect((const QObject *)symbol,
				 SIGNAL(visibilityChanged(bool)), this,
				 SLOT(updateOverlay()));

		d_overlay->updateOverlay();
	}
}

void SymbolController::detachSymbol(Symbol *symbol) {
	d_symbols.removeOne(symbol);
	QObject::disconnect((const QObject *)symbol,
			    SIGNAL(positionChanged(double)), this,
			    SLOT(updateOverlay()));
	QObject::disconnect((const QObject *)symbol,
			    SIGNAL(visibilityChanged(bool)), this,
			    SLOT(updateOverlay()));
	d_overlay->updateOverlay();
}

void SymbolController::setEnabled(bool on) {
	if (on == d_isEnabled)
		return;

	QwtPlot *plot = static_cast<QwtPlot *>(parent());
	if (plot) {
		d_isEnabled = on;

		if (on) {
			plot->canvas()->installEventFilter(this);
		} else {
			plot->canvas()->removeEventFilter(this);
		}
	}
}

bool SymbolController::isEnabled() const { return d_isEnabled; }

void SymbolController::updateOverlay() { d_overlay->updateOverlay(); }

bool SymbolController::eventFilter(QObject *object, QEvent *event) {
	QwtPlot *plot = static_cast<QwtPlot *>(parent());

	if (plot && object == plot->canvas()) {
		switch (event->type()) {
		case QEvent::MouseButtonPress: {
			const QMouseEvent *mouseEvent =
				dynamic_cast<QMouseEvent *>(event);

			if (mouseEvent->button() == Qt::LeftButton) {
				const bool accepted =
					pressed(mouseEvent->pos());

				if (accepted) {
					d_overlay->updateOverlay();
					d_overlay->show();
				}
			}

			break;
		}
		case QEvent::MouseMove: {
			const QMouseEvent *mouseEvent =
				dynamic_cast<QMouseEvent *>(event);
			const bool accepted = moved(mouseEvent->pos());

			if (accepted)
				d_overlay->updateOverlay();

			break;
		}
		case QEvent::MouseButtonRelease: {
			const QMouseEvent *mouseEvent =
				dynamic_cast<QMouseEvent *>(event);

			if (mouseEvent->button() == Qt::LeftButton) {
				released(mouseEvent->pos());
			}

			break;
		}
		case QEvent::Resize: {
			for (int i = 0; i < d_symbols.size(); i++)
				d_symbols[i]->updateSurfacePos();
			break;
		}
		default:
			break;
		}

		return false;
	}

	return QObject::eventFilter(object, event);
}

bool SymbolController::pressed(const QPoint &pos) {
	for (int i = 0; i < d_symbols.size(); i++) {
		Symbol *symbol = d_symbols[i];

		if (symbol->surface().contains(pos)) {
			symbol->setSelected(true);
			d_selectedSymbol = symbol;
			d_prevPos = pos;

			return true;
		}
	}
	return false;
}

bool SymbolController::moved(const QPoint &pos) {
	if (plot() == NULL)
		return false;

	if (d_selectedSymbol && d_selectedSymbol->isSelected()) {
		QPointF delta = d_selectedSymbol->invTransform(pos) -
			d_selectedSymbol->invTransform(d_prevPos);
		bool moved = d_selectedSymbol->moveWith(delta.x(), delta.y());
		if (moved)
			d_prevPos = pos;
	}

	return true;
}

void SymbolController::released(const QPoint &pos) {
	Q_UNUSED(pos);

	if (d_selectedSymbol && d_selectedSymbol->isSelected()) {
		d_selectedSymbol->setSelected(false);
		d_selectedSymbol = NULL;
	}
}

void SymbolController::drawCursor(QPainter *painter) const {
	for (int i = 0; i < d_symbols.size(); i++)
		if (d_symbols[i]->isVisible())
			d_symbols[i]->draw(painter);
}

void SymbolController::drawOverlay(QPainter *painter) const {
	drawCursor(painter);
}

QRegion SymbolController::maskHint() const { return QRegion(); }
