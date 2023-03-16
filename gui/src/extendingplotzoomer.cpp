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
#include "extendingplotzoomer.h"
#include "oscilloscope_plot.hpp"
#include <QPainterPath>

using namespace adiscope;

ExtendingPlotZoomer::ExtendingPlotZoomer(QWidget *parent, bool doReplot):
    LimitedPlotZoomer(parent, doReplot),
    widthPass(false),
    heightPass(false),
    extendWidth(false),
    extendHeight(false)
{
	extendMarkers.push_back(new QwtPlotShapeItem());
	extendMarkers.push_back(new QwtPlotShapeItem());

	cornerMarkers.push_back(new QwtPlotShapeItem());
	cornerMarkers.push_back(new QwtPlotShapeItem());
	cornerMarkers.push_back(new QwtPlotShapeItem());
	cornerMarkers.push_back(new QwtPlotShapeItem());
}

ExtendingPlotZoomer::~ExtendingPlotZoomer()
{
	for (auto it = extendMarkers.begin(); it != extendMarkers.end(); ++it) {
		delete *it;
	}

	for (auto it = cornerMarkers.begin(); it != cornerMarkers.end(); ++it) {
		delete *it;
	}
}

void ExtendingPlotZoomer::zoom(const QRectF &rect)
{
	widthPass = false;
	heightPass = false;
	extendHeight = false;
	extendWidth = false;
	cornerMarkers[0]->detach();
	cornerMarkers[1]->detach();
	cornerMarkers[2]->detach();
	cornerMarkers[3]->detach();
	extendMarkers[0]->detach();
	extendMarkers[1]->detach();

	LimitedPlotZoomer::zoom(rect);
}

QPolygon ExtendingPlotZoomer::adjustedPoints(const QPolygon &points) const
{
	QPolygon adjusted;
	if (points.size() < 2)
		return points;

	if (points[0] == points[1]) {
		cornerMarkers[0]->detach();
		cornerMarkers[1]->detach();
		cornerMarkers[2]->detach();
		cornerMarkers[3]->detach();
		return points;
	}

	const int width = qAbs(points[1].x() - points[0].x());
	const int height = qAbs(points[1].y() - points[0].y());

	if ((width > 50) && !widthPass) {
		widthPass = true;
	}

	if ((height > 50) && !heightPass) {
		heightPass = true;
	}

	//Handle width extension
	if (width < 40 && widthPass && !extendWidth && !extendHeight) {
		extendWidth = true;
	}
	if (width > 40 && widthPass && extendWidth && !extendHeight) {
		extendWidth = false;
	}

	//Handle height extension
	if (height < 40 && heightPass && !extendHeight && !extendWidth) {
		extendHeight = true;
	}
	if (height > 40 && heightPass && extendHeight && !extendWidth) {
		extendHeight = false;
	}

	if (extendWidth && !extendHeight) {
		QPoint topLeft(0, points[0].y());
		QPoint bottomRight(canvas()->width() - 1, points[1].y());
		adjusted += topLeft;
		adjusted += bottomRight;

		if (yAxis() == QwtAxisId(QwtAxis::YLeft, 0)) {
			cornerMarkers[0]->detach();
			cornerMarkers[1]->detach();
			cornerMarkers[2]->detach();
			cornerMarkers[3]->detach();

			QPointF x11 = invTransform(QPoint(points[0].x() - 40, points[1].y()));
			QPointF x21 = invTransform(QPoint(points[0].x() + 40, points[1].y()));

			QPainterPath path1;
			path1.moveTo(x11);
			path1.lineTo(x21);

			extendMarkers[0]->setShape(path1);
			extendMarkers[0]->setPen(Qt::white, 3, Qt::SolidLine);
			extendMarkers[0]->setZ(1000);
			extendMarkers[0]->attach((QwtPlot*)plot());


			QPointF x12 = invTransform(QPoint(points[0].x() - 40, points[0].y()));
			QPointF x22 = invTransform(QPoint(points[0].x() + 40, points[0].y()));

			QPainterPath path2;
			path2.moveTo(x12);
			path2.lineTo(x22);

			extendMarkers[1]->setShape(path2);
			extendMarkers[1]->setPen(Qt::white, 3, Qt::SolidLine);
			extendMarkers[1]->setZ(1000);
			extendMarkers[1]->attach((QwtPlot*)plot());


			static_cast<CapturePlot*>((QwtPlot*)plot())->replot();
		}

		return adjusted;
	}

	if (extendHeight && !extendWidth) {
		QPoint topLeft(points[0].x(), 0);
		QPoint bottomRight(points[1].x(), canvas()->height() - 1);
		adjusted += topLeft;
		adjusted += bottomRight;

		if (yAxis() == QwtAxisId(QwtAxis::YLeft, 0)) {
			cornerMarkers[0]->detach();
			cornerMarkers[1]->detach();
			cornerMarkers[2]->detach();
			cornerMarkers[3]->detach();

			QPointF x11 = invTransform(QPoint(points[0].x(), points[0].y() - 40));
			QPointF x21 = invTransform(QPoint(points[0].x(), points[0].y() + 40));

			QPainterPath path1;
			path1.moveTo(x11);
			path1.lineTo(x21);

			extendMarkers[0]->setShape(path1);
			extendMarkers[0]->setPen(Qt::white, 3, Qt::SolidLine);
			extendMarkers[0]->setZ(1000);
			extendMarkers[0]->attach((QwtPlot*)plot());


			QPointF x12 = invTransform(QPoint(points[1].x(), points[0].y() - 40));
			QPointF x22 = invTransform(QPoint(points[1].x(), points[0].y() + 40));

			QPainterPath path2;
			path2.moveTo(x12);
			path2.lineTo(x22);

			extendMarkers[1]->setShape(path2);
			extendMarkers[1]->setPen(Qt::white, 3, Qt::SolidLine);
			extendMarkers[1]->setZ(1000);
			extendMarkers[1]->attach((QwtPlot*)plot());


			static_cast<CapturePlot*>((QwtPlot*)plot())->replot();
		}

		return adjusted;
	}


	if (yAxis() == QwtAxisId(QwtAxis::YLeft, 0)) {

		extendMarkers[0]->detach();
		extendMarkers[1]->detach();

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

		cornerMarkers[0]->setShape(path1);
		cornerMarkers[0]->setPen(Qt::white, 3, Qt::SolidLine);
		cornerMarkers[0]->setZ(1000);
		cornerMarkers[0]->attach((QwtPlot*)plot());

		cornerMarkers[1]->setShape(path2);
		cornerMarkers[1]->setPen(Qt::white, 3, Qt::SolidLine);
		cornerMarkers[1]->setZ(1000);
		cornerMarkers[1]->attach((QwtPlot*)plot());

		cornerMarkers[2]->setShape(path3);
		cornerMarkers[2]->setPen(Qt::white, 3, Qt::SolidLine);
		cornerMarkers[2]->setZ(1000);
		cornerMarkers[2]->attach((QwtPlot*)plot());

		cornerMarkers[3]->setShape(path4);
		cornerMarkers[3]->setPen(Qt::white, 3, Qt::SolidLine);
		cornerMarkers[3]->setZ(1000);
		cornerMarkers[3]->attach((QwtPlot*)plot());

		static_cast<CapturePlot*>((QwtPlot*)plot())->replot();
	}
	return points;
}
