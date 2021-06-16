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
#ifndef MARKER_CONTROLLER_H
#define MARKER_CONTROLLER_H

#include <QList>
#include <QObject>
#include <qwt_plot.h>

#include <memory>

class QwtPlotPicker;

namespace scopy {
namespace gui {

class SpectrumMarker;

class MarkerController : public QObject
{
	Q_OBJECT

public:
	typedef std::shared_ptr<SpectrumMarker> marker_sptr;

	explicit MarkerController(QwtPlot* plot);
	MarkerController(const MarkerController&) = delete;
	void operator=(const MarkerController&) = delete;

	bool enabled() const;
	void setEnabled(bool en);

	void registerMarker(marker_sptr marker);
	void unRegisterMarker(marker_sptr marker);

	marker_sptr selectedMarker() const;
	void markerBringToFront(marker_sptr marker);

	const QwtPlot* plot() const;
	QwtPlot* plot();

Q_SIGNALS:
	void markerSelected(std::shared_ptr<SpectrumMarker>&);
	void markerPositionChanged(std::shared_ptr<SpectrumMarker>&);
	void markerReleased(std::shared_ptr<SpectrumMarker>&);

private Q_SLOTS:
	void onPickerSelected(QPointF);
	void onPickerMoved(QPoint);

private:
	void selectMarker(marker_sptr marker);

private:
	QwtPlotPicker* m_picker;

	QList<marker_sptr> m_markers;
	QList<marker_sptr> m_mrksOverlapOrder;
	marker_sptr m_pickedMrk;
	marker_sptr m_selectedMkr;

	bool m_itemMoving;
	double m_mrksDefaultZ;
};

} // namespace gui
} // namespace scopy

#endif // MARKER_CONTROLLER_H
