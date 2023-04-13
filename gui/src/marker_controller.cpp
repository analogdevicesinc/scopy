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
#include "marker_controller.h"
#include "spectrum_marker.hpp"

#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
#include <qwt_scale_div.h>
#include <qwt_interval.h>

using namespace scopy;

/*
 * Class MarkerController has the purpose to control SpectrumMarker objects. The
 * controlling is done by monitoring the plot canvas for user clicks and
 * detecting if a marker has selected, moved or released (at the end of the
 * move action). It also implements the actual moving of the marker.
 * It doesn't own the markers but shares them. It operates on a list of markers
 * that is modified using the registerMarker() and unRegisterMarker().
 */
MarkerController::MarkerController(QwtPlot *plot):
	QObject(static_cast<QObject *>(plot)),
	d_picker(new QwtPlotPicker(plot->canvas())),
	d_picked_mrk(nullptr),
	d_item_moving(nullptr),
	d_mrks_default_z(0.0)
{
	d_picker->setStateMachine(new QwtPickerDragPointMachine);
	connect(d_picker, SIGNAL(selected(QPointF)),
		this, SLOT(onPickerSelected(QPointF)));
	connect(d_picker, SIGNAL(moved(QPoint)),
		this, SLOT(onPickerMoved(QPoint)));
}

bool MarkerController::enabled() const
{
	return d_picker->isEnabled();
}

void MarkerController::setEnabled(bool en)
{
	d_picker->setEnabled(en);
}

void MarkerController::onPickerSelected(QPointF pf)
{
	if (d_item_moving) {
		d_item_moving = false;
		Q_EMIT markerReleased(d_picked_mrk);
		d_picked_mrk = nullptr;
		return;
	}

	auto mrks = d_mrks_overlap_order;

	d_selected_mkr = nullptr;
	for (int i = 0; i < mrks.size(); i++) {
		QPoint p = mrks[i]->plotValueToPixels(pf);

		QRect rect = mrks[i]->boundingRectPixels();
		if (p.x() >= rect.topLeft().x() &&
				p.x() <= rect.bottomRight().x() &&
				p.y() >= rect.topLeft().y() &&
				p.y() <= rect.bottomRight().y() &&
				!d_selected_mkr) {

			markerBringToFront(mrks[i]);

			selectMarker(mrks[i]);
		}
	}

	plot()->replot();
	d_picked_mrk = nullptr;
}

void MarkerController::onPickerMoved(QPoint p)
{
	if (!d_picked_mrk) {
		auto mrks = d_mrks_overlap_order;

		for (int i = 0; i < mrks.size(); i++) {
			QRect rect = mrks[i]->boundingRectPixels();
			if (p.x() >= rect.topLeft().x() &&
				p.x() <= rect.bottomRight().x() &&
				p.y() >= rect.topLeft().y() &&
				p.y() <= rect.bottomRight().y() &&
				!d_picked_mrk) {

				markerBringToFront(mrks[i]);

				d_mrks_overlap_order.removeOne(mrks[i]);
				d_mrks_overlap_order.push_front(mrks[i]);

				d_picked_mrk = mrks[i];
				selectMarker(mrks[i]);
			}
		}
	}

	if (d_picked_mrk) {
		QPointF val = d_picked_mrk->plotPixelsToValue(p);
		QwtInterval xItv = plot()->axisScaleDiv(
			QwtAxis::XBottom).interval();
		QwtInterval yItv = plot()->axisScaleDiv(
			QwtAxis::YLeft).interval();

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

		d_picked_mrk->setValue(val);
		Q_EMIT markerPositionChanged(d_picked_mrk);
		plot()->replot();
	}
	d_item_moving = true;
}

void MarkerController::registerMarker(marker_sptr marker)
{
	d_markers.push_back(marker);
	d_mrks_overlap_order.push_front(marker);

	if (d_markers.size() == 1) {
		d_mrks_default_z = marker->z();
	}
}

void MarkerController::unRegisterMarker(marker_sptr marker)
{
	d_markers.removeOne(marker);
	d_mrks_overlap_order.removeOne(marker);

	if (marker == d_selected_mkr) {
		d_selected_mkr = nullptr;
	}
}

MarkerController::marker_sptr MarkerController::selectedMarker() const
{
	return d_selected_mkr;
}

void MarkerController::markerBringToFront(marker_sptr marker)
{
	d_mrks_overlap_order.removeOne(marker);
	d_mrks_overlap_order.push_front(marker);
	marker->setZ(d_mrks_default_z + 1);
	plot()->replot();
	marker->setZ(d_mrks_default_z);
}

QwtPlot *MarkerController::plot()
{
	return static_cast<QwtPlot *>(parent());
}

const QwtPlot *MarkerController::plot() const
{
	return static_cast<const QwtPlot *>(parent());
}

void MarkerController::selectMarker(marker_sptr marker)
{
	if (marker->isVisible()) {
		d_selected_mkr = marker;
		Q_EMIT markerSelected(marker);
	}
}

#include "moc_marker_controller.cpp"
