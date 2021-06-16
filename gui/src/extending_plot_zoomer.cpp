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
#include <QPainterPath>

#include <scopy/gui/extending_plot_zoomer.hpp>
#include <scopy/gui/oscilloscope_plot.hpp>

using namespace scopy::gui;

ExtendingPlotZoomer::ExtendingPlotZoomer(QWidget* parent, bool doReplot)
	: LimitedPlotZoomer(parent, doReplot)
	, m_widthPass(false)
	, m_heightPass(false)
	, m_extendWidth(false)
	, m_extendHeight(false)
{
	m_extendMarkers.push_back(new QwtPlotShapeItem());
	m_extendMarkers.push_back(new QwtPlotShapeItem());

	m_cornerMarkers.push_back(new QwtPlotShapeItem());
	m_cornerMarkers.push_back(new QwtPlotShapeItem());
	m_cornerMarkers.push_back(new QwtPlotShapeItem());
	m_cornerMarkers.push_back(new QwtPlotShapeItem());
}

ExtendingPlotZoomer::~ExtendingPlotZoomer()
{
	for (auto it = m_extendMarkers.begin(); it != m_extendMarkers.end(); ++it) {
		delete *it;
	}

	for (auto it = m_cornerMarkers.begin(); it != m_cornerMarkers.end(); ++it) {
		delete *it;
	}
}

void ExtendingPlotZoomer::zoom(const QRectF& rect)
{
	m_widthPass = false;
	m_heightPass = false;
	m_extendHeight = false;
	m_extendWidth = false;
	m_cornerMarkers[0]->detach();
	m_cornerMarkers[1]->detach();
	m_cornerMarkers[2]->detach();
	m_cornerMarkers[3]->detach();
	m_extendMarkers[0]->detach();
	m_extendMarkers[1]->detach();

	LimitedPlotZoomer::zoom(rect);
}

QPolygon ExtendingPlotZoomer::adjustedPoints(const QPolygon& points) const
{
	QPolygon adjusted;
	if (points.size() < 2)
		return points;

	if (points[0] == points[1]) {
		m_cornerMarkers[0]->detach();
		m_cornerMarkers[1]->detach();
		m_cornerMarkers[2]->detach();
		m_cornerMarkers[3]->detach();
		static_cast<CapturePlot*>((QwtPlot*)plot())->replot();
		return points;
	}

	const int width = qAbs(points[1].x() - points[0].x());
	const int height = qAbs(points[1].y() - points[0].y());

	if ((width > 50) && !m_widthPass) {
		m_widthPass = true;
	}

	if ((height > 50) && !m_heightPass) {
		m_heightPass = true;
	}

	// Handle width extension
	if (width < 40 && m_widthPass && !m_extendWidth && !m_extendHeight) {
		m_extendWidth = true;
	}
	if (width > 40 && m_widthPass && m_extendWidth && !m_extendHeight) {
		m_extendWidth = false;
	}

	// Handle height extension
	if (height < 40 && m_heightPass && !m_extendHeight && !m_extendWidth) {
		m_extendHeight = true;
	}
	if (height > 40 && m_heightPass && m_extendHeight && !m_extendWidth) {
		m_extendHeight = false;
	}

	if (m_extendWidth && !m_extendHeight) {
		QPoint topLeft(0, points[0].y());
		QPoint bottomRight(canvas()->width() - 1, points[1].y());
		adjusted += topLeft;
		adjusted += bottomRight;

		if (yAxis() == QwtAxisId(QwtPlot::yLeft, 0)) {
			m_cornerMarkers[0]->detach();
			m_cornerMarkers[1]->detach();
			m_cornerMarkers[2]->detach();
			m_cornerMarkers[3]->detach();

			QPointF x11 = invTransform(QPoint(points[0].x() - 40, points[1].y()));
			QPointF x21 = invTransform(QPoint(points[0].x() + 40, points[1].y()));

			QPainterPath path1;
			path1.moveTo(x11);
			path1.lineTo(x21);

			m_extendMarkers[0]->setShape(path1);
			m_extendMarkers[0]->setPen(Qt::white, 3, Qt::SolidLine);
			m_extendMarkers[0]->setZ(1000);
			m_extendMarkers[0]->attach((QwtPlot*)plot());

			QPointF x12 = invTransform(QPoint(points[0].x() - 40, points[0].y()));
			QPointF x22 = invTransform(QPoint(points[0].x() + 40, points[0].y()));

			QPainterPath path2;
			path2.moveTo(x12);
			path2.lineTo(x22);

			m_extendMarkers[1]->setShape(path2);
			m_extendMarkers[1]->setPen(Qt::white, 3, Qt::SolidLine);
			m_extendMarkers[1]->setZ(1000);
			m_extendMarkers[1]->attach((QwtPlot*)plot());

			static_cast<CapturePlot*>((QwtPlot*)plot())->replot();
		}

		return adjusted;
	}

	if (m_extendHeight && !m_extendWidth) {
		QPoint topLeft(points[0].x(), 0);
		QPoint bottomRight(points[1].x(), canvas()->height() - 1);
		adjusted += topLeft;
		adjusted += bottomRight;

		if (yAxis() == QwtAxisId(QwtPlot::yLeft, 0)) {
			m_cornerMarkers[0]->detach();
			m_cornerMarkers[1]->detach();
			m_cornerMarkers[2]->detach();
			m_cornerMarkers[3]->detach();

			QPointF x11 = invTransform(QPoint(points[0].x(), points[0].y() - 40));
			QPointF x21 = invTransform(QPoint(points[0].x(), points[0].y() + 40));

			QPainterPath path1;
			path1.moveTo(x11);
			path1.lineTo(x21);

			m_extendMarkers[0]->setShape(path1);
			m_extendMarkers[0]->setPen(Qt::white, 3, Qt::SolidLine);
			m_extendMarkers[0]->setZ(1000);
			m_extendMarkers[0]->attach((QwtPlot*)plot());

			QPointF x12 = invTransform(QPoint(points[1].x(), points[0].y() - 40));
			QPointF x22 = invTransform(QPoint(points[1].x(), points[0].y() + 40));

			QPainterPath path2;
			path2.moveTo(x12);
			path2.lineTo(x22);

			m_extendMarkers[1]->setShape(path2);
			m_extendMarkers[1]->setPen(Qt::white, 3, Qt::SolidLine);
			m_extendMarkers[1]->setZ(1000);
			m_extendMarkers[1]->attach((QwtPlot*)plot());

			static_cast<CapturePlot*>((QwtPlot*)plot())->replot();
		}

		return adjusted;
	}

	if (yAxis() == QwtAxisId(QwtPlot::yLeft, 0)) {

		m_extendMarkers[0]->detach();
		m_extendMarkers[1]->detach();

		QPainterPath path1, path2, path3, path4;

		if (points[0].x() < points[1].x()) {

			QPointF origin1 = invTransform(points[0]);
			QPointF bottom1;
			if (points[0].y() < points[1].y())
				bottom1 = invTransform(QPoint(points[0].x(), points[0].y() + 20));
			else
				bottom1 = invTransform(QPoint(points[0].x(), points[0].y() - 20));
			QPointF right1 = invTransform(QPoint(points[0].x() + 20, points[0].y()));

			path1.moveTo(bottom1);
			path1.lineTo(origin1);
			path1.lineTo(right1);

			QPointF origin2 = invTransform(QPoint(points[1].x(), points[0].y()));
			QPointF bottom2;
			if (points[0].y() < points[1].y())
				bottom2 = invTransform(QPoint(points[1].x(), points[0].y() + 20));
			else
				bottom2 = invTransform(QPoint(points[1].x(), points[0].y() - 20));
			QPointF right2 = invTransform(QPoint(points[1].x() - 20, points[0].y()));

			path2.moveTo(bottom2);
			path2.lineTo(origin2);
			path2.lineTo(right2);

			QPointF origin3 = invTransform(QPoint(points[1].x(), points[1].y()));
			QPointF bottom3;
			if (points[0].y() < points[1].y())
				bottom3 = invTransform(QPoint(points[1].x(), points[1].y() - 20));
			else
				bottom3 = invTransform(QPoint(points[1].x(), points[1].y() + 20));
			QPointF right3 = invTransform(QPoint(points[1].x() - 20, points[1].y()));

			path3.moveTo(bottom3);
			path3.lineTo(origin3);
			path3.lineTo(right3);

			QPointF origin4 = invTransform(QPoint(points[0].x(), points[1].y()));
			QPointF bottom4;
			if (points[0].y() < points[1].y())
				bottom4 = invTransform(QPoint(points[0].x(), points[1].y() - 20));
			else
				bottom4 = invTransform(QPoint(points[0].x(), points[1].y() + 20));
			QPointF right4 = invTransform(QPoint(points[0].x() + 20, points[1].y()));

			path4.moveTo(bottom4);
			path4.lineTo(origin4);
			path4.lineTo(right4);

		} else {

			QPointF origin1 = invTransform(points[0]);
			QPointF bottom1;
			if (points[0].y() < points[1].y())
				bottom1 = invTransform(QPoint(points[0].x(), points[0].y() + 20));
			else
				bottom1 = invTransform(QPoint(points[0].x(), points[0].y() - 20));
			QPointF right1 = invTransform(QPoint(points[0].x() - 20, points[0].y()));

			path1.moveTo(bottom1);
			path1.lineTo(origin1);
			path1.lineTo(right1);

			QPointF origin2 = invTransform(QPoint(points[1].x(), points[0].y()));
			QPointF bottom2;
			if (points[0].y() < points[1].y())
				bottom2 = invTransform(QPoint(points[1].x(), points[0].y() + 20));
			else
				bottom2 = invTransform(QPoint(points[1].x(), points[0].y() - 20));
			QPointF right2 = invTransform(QPoint(points[1].x() + 20, points[0].y()));

			path2.moveTo(bottom2);
			path2.lineTo(origin2);
			path2.lineTo(right2);

			QPointF origin3 = invTransform(QPoint(points[1].x(), points[1].y()));
			QPointF bottom3;
			if (points[0].y() < points[1].y())
				bottom3 = invTransform(QPoint(points[1].x(), points[1].y() - 20));
			else
				bottom3 = invTransform(QPoint(points[1].x(), points[1].y() + 20));
			QPointF right3 = invTransform(QPoint(points[1].x() + 20, points[1].y()));

			path3.moveTo(bottom3);
			path3.lineTo(origin3);
			path3.lineTo(right3);

			QPointF origin4 = invTransform(QPoint(points[0].x(), points[1].y()));
			QPointF bottom4;
			if (points[0].y() < points[1].y())
				bottom4 = invTransform(QPoint(points[0].x(), points[1].y() - 20));
			else
				bottom4 = invTransform(QPoint(points[0].x(), points[1].y() + 20));
			QPointF right4 = invTransform(QPoint(points[0].x() - 20, points[1].y()));

			path4.moveTo(bottom4);
			path4.lineTo(origin4);
			path4.lineTo(right4);
		}

		m_cornerMarkers[0]->setShape(path1);
		m_cornerMarkers[0]->setPen(Qt::white, 3, Qt::SolidLine);
		m_cornerMarkers[0]->setZ(1000);
		m_cornerMarkers[0]->attach((QwtPlot*)plot());

		m_cornerMarkers[1]->setShape(path2);
		m_cornerMarkers[1]->setPen(Qt::white, 3, Qt::SolidLine);
		m_cornerMarkers[1]->setZ(1000);
		m_cornerMarkers[1]->attach((QwtPlot*)plot());

		m_cornerMarkers[2]->setShape(path3);
		m_cornerMarkers[2]->setPen(Qt::white, 3, Qt::SolidLine);
		m_cornerMarkers[2]->setZ(1000);
		m_cornerMarkers[2]->attach((QwtPlot*)plot());

		m_cornerMarkers[3]->setShape(path4);
		m_cornerMarkers[3]->setPen(Qt::white, 3, Qt::SolidLine);
		m_cornerMarkers[3]->setZ(1000);
		m_cornerMarkers[3]->attach((QwtPlot*)plot());

		static_cast<CapturePlot*>((QwtPlot*)plot())->replot();
	}
	return points;
}
