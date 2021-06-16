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

#include "qwt_painter.h"

#include <QDebug>
#include <QMouseEvent>
#include <qevent.h>
#include <qpainter.h>
#include <qwt_plot.h>

#include <scopy/gui/symbol_controller.hpp>

class Overlay : public QwtWidgetOverlay
{
public:
	Overlay(QWidget* parent, SymbolController* symbolController)
		: QwtWidgetOverlay(parent)
		, d_symbolCtrl(symbolController)
	{}

protected:
	virtual void drawOverlay(QPainter* painter) const { d_symbolCtrl->drawOverlay(painter); }

	virtual QRegion maskHint() const { return d_symbolCtrl->maskHint(); }

private:
	SymbolController* d_symbolCtrl;
};

SymbolController::SymbolController(QwtPlot* plot)
	: QObject(plot)
	, m_isEnabled(false)
	, m_selectedSymbol(NULL)
	, m_axesCreated(0)
{
	setEnabled(true);

	m_overlay = new Overlay(plot->canvas(), this);
}

SymbolController::~SymbolController()
{
	// d_overlay gets destroyed with parent
}

QwtPlot* SymbolController::plot() { return static_cast<QwtPlot*>(parent()); }

const QwtPlot* SymbolController::plot() const { return static_cast<const QwtPlot*>(parent()); }

void SymbolController::attachSymbol(Symbol* symbol)
{
	if (m_symbols.indexOf(symbol) == -1) {
		m_symbols.push_back(symbol);
		QObject::connect((const QObject*)symbol, SIGNAL(positionChanged(double)), this, SLOT(updateOverlay()));
		QObject::connect((const QObject*)symbol, SIGNAL(visibilityChanged(bool)), this, SLOT(updateOverlay()));

		m_overlay->updateOverlay();
	}
}

void SymbolController::detachSymbol(Symbol* symbol)
{
	m_symbols.removeOne(symbol);
	QObject::disconnect((const QObject*)symbol, SIGNAL(positionChanged(double)), this, SLOT(updateOverlay()));
	QObject::disconnect((const QObject*)symbol, SIGNAL(visibilityChanged(bool)), this, SLOT(updateOverlay()));
	m_overlay->updateOverlay();
}

void SymbolController::setEnabled(bool on)
{
	if (on == m_isEnabled)
		return;

	QwtPlot* plot = static_cast<QwtPlot*>(parent());
	if (plot) {
		m_isEnabled = on;

		if (on) {
			plot->canvas()->installEventFilter(this);
		} else {
			plot->canvas()->removeEventFilter(this);
		}
	}
}

bool SymbolController::isEnabled() const { return m_isEnabled; }

void SymbolController::updateOverlay() { m_overlay->updateOverlay(); }

bool SymbolController::eventFilter(QObject* object, QEvent* event)
{
	QwtPlot* plot = static_cast<QwtPlot*>(parent());

	if (plot && object == plot->canvas()) {
		switch (event->type()) {
		case QEvent::MouseButtonPress: {
			const QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);

			if (mouseEvent->button() == Qt::LeftButton) {
				const bool accepted = pressed(mouseEvent->pos());

				if (accepted) {
					m_overlay->updateOverlay();
					m_overlay->show();
				}
			}

			break;
		}
		case QEvent::MouseMove: {
			const QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
			const bool accepted = moved(mouseEvent->pos());

			if (accepted)
				m_overlay->updateOverlay();

			break;
		}
		case QEvent::MouseButtonRelease: {
			const QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);

			if (mouseEvent->button() == Qt::LeftButton) {
				released(mouseEvent->pos());
			}

			break;
		}
		case QEvent::Resize: {
			for (int i = 0; i < m_symbols.size(); i++)
				m_symbols[i]->updateSurfacePos();
			break;
		}
		default:
			break;
		}

		return false;
	}

	return QObject::eventFilter(object, event);
}

bool SymbolController::pressed(const QPoint& pos)
{
	for (int i = 0; i < m_symbols.size(); i++) {
		Symbol* symbol = m_symbols[i];

		if (symbol->surface().contains(pos)) {
			symbol->setSelected(true);
			m_selectedSymbol = symbol;
			m_prevPos = pos;

			return true;
		}
	}
	return false;
}

bool SymbolController::moved(const QPoint& pos)
{
	if (plot() == NULL)
		return false;

	if (m_selectedSymbol && m_selectedSymbol->isSelected()) {
		QPointF delta = m_selectedSymbol->invTransform(pos) - m_selectedSymbol->invTransform(m_prevPos);
		bool moved = m_selectedSymbol->moveWith(delta.x(), delta.y());
		if (moved)
			m_prevPos = pos;
	}

	return true;
}

void SymbolController::released(const QPoint& pos)
{
	Q_UNUSED(pos);

	if (m_selectedSymbol && m_selectedSymbol->isSelected()) {
		m_selectedSymbol->setSelected(false);
		m_selectedSymbol = NULL;
	}
}

void SymbolController::drawCursor(QPainter* painter) const
{
	for (int i = 0; i < m_symbols.size(); i++)
		if (m_symbols[i]->isVisible())
			m_symbols[i]->draw(painter);
}

void SymbolController::drawOverlay(QPainter* painter) const { drawCursor(painter); }

QRegion SymbolController::maskHint() const { return QRegion(); }
