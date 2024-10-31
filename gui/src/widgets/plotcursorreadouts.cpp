/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <widgets/plotcursorreadouts.h>

using namespace scopy;

PlotCursorReadouts::PlotCursorReadouts(QWidget *parent)
	: QWidget(parent)
	, xFormatter(nullptr)
	, yFormatter(nullptr)
	, xUnit("")
	, yUnit("")
{
	initContent();
}

PlotCursorReadouts::~PlotCursorReadouts() {}

void PlotCursorReadouts::setY1(double val)
{
	y1 = val;
	update();
}

void PlotCursorReadouts::setY2(double val)
{
	y2 = val;
	update();
}

void PlotCursorReadouts::setX1(double val)
{
	x1 = val;
	update();
}

void PlotCursorReadouts::setX2(double val)
{
	x2 = val;
	update();
}

void PlotCursorReadouts::update()
{
	const uint precision = 3;

	if(yFormatter && !xUnit.isEmpty()) {
		x1_val->setText(yFormatter->format(x1, xUnit, precision));
		x2_val->setText(yFormatter->format(x2, xUnit, precision));
		deltaX_val->setText(yFormatter->format(x2 - x1, xUnit, precision));
		invDeltaX_val->setText(yFormatter->format(1 / (x2 - x1), xUnit, precision));
	} else {
		x1_val->setText(QString::number(x1, 'g', precision) + " " + xUnit);
		x2_val->setText(QString::number(x2, 'g', precision) + " " + xUnit);
		deltaX_val->setText(QString::number(x2 - x1, 'g', precision) + " " + xUnit);
		invDeltaX_val->setText(QString::number(1 / (x2 - x1), 'g', precision) + " " + xUnit);
	}

	if(yFormatter && !yUnit.isEmpty()) {
		y1_val->setText(yFormatter->format(y1, yUnit, precision));
		y2_val->setText(yFormatter->format(y2, yUnit, precision));
		deltaY_val->setText(yFormatter->format(y2 - y1, yUnit, precision));
	} else {
		y1_val->setText(QString::number(y1, 'g', precision) + " " + yUnit);
		y2_val->setText(QString::number(y2, 'g', precision) + " " + yUnit);
		deltaY_val->setText(QString::number(y2 - y1, 'g', precision) + " " + yUnit);
	}
}

void PlotCursorReadouts::initContent()
{
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	QHBoxLayout *content_lay = new QHBoxLayout(this);
	setLayout(content_lay);
	content_lay->setMargin(10);
	content_lay->setSpacing(10);

	// y readouts
	y_contents = new QWidget(this);
	y_contents->setFixedWidth(120);
	QGridLayout *y_contents_lay = new QGridLayout(y_contents);
	y_contents_lay->setMargin(0);
	y_contents_lay->setSpacing(2);
	y_contents->setLayout(y_contents_lay);
	content_lay->addWidget(y_contents);

	y_contents_lay->addWidget(new QLabel("y1:", y_contents), 0, 0);
	y1_val = new QLabel("", this);
	y_contents_lay->addWidget(y1_val, 0, 1);

	y_contents_lay->addWidget(new QLabel("y2:", y_contents), 1, 0);
	y2_val = new QLabel("", this);
	y_contents_lay->addWidget(y2_val, 1, 1);

	y_contents_lay->addWidget(new QLabel("Δy:", y_contents), 2, 0);
	deltaY_val = new QLabel("", this);
	y_contents_lay->addWidget(deltaY_val, 2, 1);

	y_contents_lay->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding), 3, 0);

	// x readouts
	x_contents = new QWidget(this);
	x_contents->setFixedWidth(120);
	QGridLayout *x_contents_lay = new QGridLayout(x_contents);
	x_contents_lay->setMargin(0);
	x_contents_lay->setSpacing(2);
	x_contents->setLayout(x_contents_lay);
	content_lay->addWidget(x_contents);

	x_contents_lay->addWidget(new QLabel("x1:", x_contents), 0, 0);
	x1_val = new QLabel("", this);
	x_contents_lay->addWidget(x1_val, 0, 1);

	x_contents_lay->addWidget(new QLabel("x2:", x_contents), 1, 0);
	x2_val = new QLabel("", this);
	x_contents_lay->addWidget(x2_val, 1, 1);

	x_contents_lay->addWidget(new QLabel("Δx:", x_contents), 2, 0);
	deltaX_val = new QLabel("", this);
	x_contents_lay->addWidget(deltaX_val, 2, 1);

	x_contents_lay->addWidget(new QLabel("1/Δx:", x_contents), 3, 0);
	invDeltaX_val = new QLabel("", this);
	x_contents_lay->addWidget(invDeltaX_val, 3, 1);

	for(auto w : this->findChildren<QWidget *>()) {
		StyleHelper::TransparentWidget(w);
	}
}

void PlotCursorReadouts::setXVisible(bool visible) { x_contents->setVisible(visible); }

void PlotCursorReadouts::setYVisible(bool visible) { y_contents->setVisible(visible); }

bool PlotCursorReadouts::isXVisible() { return x_contents->isVisible(); }

bool PlotCursorReadouts::isYVisible() { return y_contents->isVisible(); }

void PlotCursorReadouts::setYUnits(QString unit)
{
	yUnit = unit;
	update();
}

void PlotCursorReadouts::setXUnits(QString unit)
{
	xUnit = unit;
	update();
}

void PlotCursorReadouts::setXFormatter(PrefixFormatter *formatter)
{
	yFormatter = formatter;
	update();
}

void PlotCursorReadouts::setYFormatter(PrefixFormatter *formatter)
{
	yFormatter = formatter;
	update();
}

#include "moc_plotcursorreadouts.cpp"
