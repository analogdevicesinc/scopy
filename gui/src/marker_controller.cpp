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
#include <qwt_interval.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_picker.h>
#include <qwt_scale_div.h>

#include <scopy/gui/marker_controller.hpp>
#include <scopy/gui/spectrum_marker.hpp>

using namespace scopy::gui;

/*
 * Class MarkerController has the purpose to control SpectrumMarker objects. The
 * controlling is done by monitoring the plot canvas for user clicks and
 * detecting if a marker has selected, moved or released (at the end of the
 * move action). It also implements the actual moving of the marker.
 * It doesn't own the markers but shares them. It operates on a list of markers
 * that is modified using the registerMarker() and unRegisterMarker().
 */
MarkerController::MarkerController(QwtPlot* plot)
	: QObject(static_cast<QObject*>(plot))
	, m_picker(new QwtPlotPicker(plot->canvas()))
	, m_pickedMrk(nullptr)
	, m_itemMoving(nullptr)
	, m_mrksDefaultZ(0.0)
{
	m_picker->setStateMachine(new QwtPickerDragPointMachine);
	connect(m_picker, SIGNAL(selected(QPointF)), this, SLOT(onPickerSelected(QPointF)));
	connect(m_picker, SIGNAL(moved(QPoint)), this, SLOT(onPickerMoved(QPoint)));
}

bool MarkerController::enabled() const { return m_picker->isEnabled(); }

void MarkerController::setEnabled(bool en) { m_picker->setEnabled(en); }

void MarkerController::onPickerSelected(QPointF pf)
{
	if (m_itemMoving) {
		m_itemMoving = false;
		Q_EMIT markerReleased(m_pickedMrk);
		m_pickedMrk = nullptr;
		return;
	}

	auto mrks = m_mrksOverlapOrder;

	m_selectedMkr = nullptr;
	for (int i = 0; i < mrks.size(); i++) {
		QPoint p = mrks[i]->plotValueToPixels(pf);

		QRect rect = mrks[i]->boundingRectPixels();
		if (p.x() >= rect.topLeft().x() && p.x() <= rect.bottomRight().x() && p.y() >= rect.topLeft().y() &&
		    p.y() <= rect.bottomRight().y() && !m_selectedMkr) {

			markerBringToFront(mrks[i]);

			selectMarker(mrks[i]);
		}
	}

	plot()->replot();
	m_pickedMrk = nullptr;
}

void MarkerController::onPickerMoved(QPoint p)
{
	if (!m_pickedMrk) {
		auto mrks = m_mrksOverlapOrder;

		for (int i = 0; i < mrks.size(); i++) {
			QRect rect = mrks[i]->boundingRectPixels();
			if (p.x() >= rect.topLeft().x() && p.x() <= rect.bottomRight().x() &&
			    p.y() >= rect.topLeft().y() && p.y() <= rect.bottomRight().y() && !m_pickedMrk) {

				markerBringToFront(mrks[i]);

				m_mrksOverlapOrder.removeOne(mrks[i]);
				m_mrksOverlapOrder.push_front(mrks[i]);

				m_pickedMrk = mrks[i];
				selectMarker(mrks[i]);
			}
		}
	}

	if (m_pickedMrk) {
		QPointF val = m_pickedMrk->plotPixelsToValue(p);
		QwtInterval xItv = plot()->axisScaleDiv(QwtPlot::xBottom).interval();
		QwtInterval yItv = plot()->axisScaleDiv(QwtPlot::yLeft).interval();

		// Make sure the marker does not leave the plot canvas
		if (val.x() < xItv.minValue()) {
			val.setX(xItv.minValue());
		} else if (val.x() > xItv.maxValue()) {
			val.setX(xItv.maxValue());
		}
		if (val.y() < yItv.minValue()) {
			val.setY(yItv.minValue());
		} else if (val.y() > yItv.maxValue()) {
			val.setY(yItv.maxValue());
		}

		m_pickedMrk->setValue(val);
		Q_EMIT markerPositionChanged(m_pickedMrk);
		plot()->replot();
	}
	m_itemMoving = true;
}

void MarkerController::registerMarker(marker_sptr marker)
{
	m_markers.push_back(marker);
	m_mrksOverlapOrder.push_front(marker);

	if (m_markers.size() == 1) {
		m_mrksDefaultZ = marker->z();
	}
}

void MarkerController::unRegisterMarker(marker_sptr marker)
{
	m_markers.removeOne(marker);
	m_mrksOverlapOrder.removeOne(marker);

	if (marker == m_selectedMkr) {
		m_selectedMkr = nullptr;
	}
}

MarkerController::marker_sptr MarkerController::selectedMarker() const { return m_selectedMkr; }

void MarkerController::markerBringToFront(marker_sptr marker)
{
	m_mrksOverlapOrder.removeOne(marker);
	m_mrksOverlapOrder.push_front(marker);
	marker->setZ(m_mrksDefaultZ + 1);
	plot()->replot();
	marker->setZ(m_mrksDefaultZ);
}

QwtPlot* MarkerController::plot() { return static_cast<QwtPlot*>(parent()); }

const QwtPlot* MarkerController::plot() const { return static_cast<const QwtPlot*>(parent()); }

void MarkerController::selectMarker(marker_sptr marker)
{
	if (marker->isVisible()) {
		m_selectedMkr = marker;
		Q_EMIT markerSelected(marker);
	}
}
