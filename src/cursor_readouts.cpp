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

#include "cursor_readouts.h"
#include "ui_cursor_readouts.h"

#include <qevent.h>
#include <QLabel>
#include <QGridLayout>
#include <qwt_plot.h>
#include <qwt_scale_div.h>

using namespace adiscope;

CursorReadouts::CursorReadouts(QwtPlot *plot):
	QWidget(plot),
	ui(new Ui::CursorReadouts),
	d_voltage_rd_visible(true),
	d_time_rd_visible(true),
	d_topLeft(QPoint(0, 0)),
	currentPosition(CustomPlotPositionButton::topLeft)
{
	ui->setupUi(this);

	ui->TimeCursors->setParent(plot->canvas());
	ui->VoltageCursors->setParent(plot->canvas());
	this->setGeometry(0, 0, 0, 0);

	plot->canvas()->installEventFilter(this);

	ui->TimeCursors->setAttribute(Qt::WA_TransparentForMouseEvents);
	ui->VoltageCursors->setAttribute(Qt::WA_TransparentForMouseEvents);

	anim = new QPropertyAnimation(ui->VoltageCursors, "geometry");
	anim2 = new QPropertyAnimation(ui->TimeCursors, "geometry");
}

CursorReadouts::~CursorReadouts()
{
	delete ui;
}

void CursorReadouts::setTransparency(int value)
{
	double percent = (100 - value) / 100.0;
	QString color = "rgba(20, 20, 22, " + QString::number(percent);

	ui->TimeCursors->setStyleSheet("QWidget {"
					"background-color: " + color + ");"
					"color: white;"
					"}");
	ui->VoltageCursors->setStyleSheet("QWidget {"
					"background-color: " + color + ");"
					"color: white;"
					"}");
}

void CursorReadouts::moveToPosition(CustomPlotPositionButton::ReadoutsPosition position)
{
	switch (position) {

	case CustomPlotPositionButton::topLeft:
		moveTopLeft();
		break;
	case CustomPlotPositionButton::topRight:
		moveTopRight();
		break;
	case CustomPlotPositionButton::bottomLeft:
		moveBottomLeft();
		break;
	case CustomPlotPositionButton::bottomRight:
		moveBottomRight();
		break;

	default:
		break;
	}

	currentPosition = position;
}

CustomPlotPositionButton::ReadoutsPosition CursorReadouts::getCurrentPosition()
{
	return currentPosition;
}

void CursorReadouts::showEvent(QShowEvent *event)
{
	updateSizeAndPosition(true);

	QWidget::showEvent(event);
}

QwtPlot *CursorReadouts::plot()
{
	return static_cast<QwtPlot *>(parent());
}

const QwtPlot *CursorReadouts::plot() const
{
	return static_cast<const QwtPlot *>(parent());
}

void CursorReadouts::setVoltageReadoutVisible(bool on)
{
	if (d_voltage_rd_visible != on) {
		d_voltage_rd_visible = on;
		ui->VoltageCursors->setVisible(on);
		updateSizeAndPosition();
	}
}

bool CursorReadouts::isVoltageReadoutVisible()
{
	return d_voltage_rd_visible;
}

void CursorReadouts::setTimeReadoutVisible(bool on)
{
	if (d_time_rd_visible != on) {
		d_time_rd_visible = on;
		ui->TimeCursors->setVisible(on);
		updateSizeAndPosition();
	}
}

bool CursorReadouts::isTimeReadoutVisible()
{
	return d_time_rd_visible;
}

void CursorReadouts::setTopLeftStartingPoint(QPoint point)
{
	d_topLeft = point;
}

QPoint CursorReadouts::topLeftStartingPoint()
{
	return d_topLeft;
}

void CursorReadouts::setTimeCursor1Text(const QString &text)
{
	ui->cursorT1->setText(text);
}

QString CursorReadouts::timeCursor1Text()
{
	return ui->cursorT1->text();
}

void CursorReadouts::setTimeCursor2Text(const QString &text)
{
	ui->cursorT2->setText(text);
}

QString CursorReadouts::timeCursor2Text()
{
	return ui->cursorT2->text();
}

void CursorReadouts::setTimeDeltaText(const QString &text)
{
	ui->timeDelta->setText(text);
}

QString CursorReadouts::timeDeltaText()
{
	return ui->timeDelta->text();
}

void CursorReadouts::setFreqDeltaText(const QString &text)
{
	ui->frequencyDelta->setText(text);
}

QString CursorReadouts::freqDeltaText()
{
	return ui->frequencyDelta->text();
}

void CursorReadouts::setVoltageCursor1Text(const QString &text)
{
	ui->cursorV1->setText(text);
}

QString CursorReadouts::voltageCursor1Text()
{
	return ui->cursorV1->text();
}

void CursorReadouts::setVoltageCursor2Text(const QString &text)
{
	ui->cursorV2->setText(text);
}

QString CursorReadouts::voltageCursor2Text()
{
	return ui->cursorV2->text();
}

void CursorReadouts::setVoltageDeltaText(const QString &text)
{
	ui->voltageDelta->setText(text);
}

QString CursorReadouts::voltageDeltaText()
{
	return ui->voltageDelta->text();
}

bool CursorReadouts::eventFilter(QObject *object, QEvent *event)
{
	if (object == plot()->canvas()) {
		switch(event->type()) {
			case QEvent::Resize: {
				updateSizeAndPosition(true);
				break;
			}
			default:
				break;
			}

		return false;
	}

	return QObject::eventFilter(object, event);
}

QPoint CursorReadouts::plotPointToPixelPoint(const QPointF &point) const
{
	const QwtScaleMap xMap = plot()->canvasMap(QwtPlot::xBottom);
	const QwtScaleMap yMap = plot()->canvasMap(QwtPlot::yLeft);

	return QwtScaleMap::transform(xMap, yMap, point).toPoint();
}

void CursorReadouts::updateSizeAndPosition(bool resize)
{
	switch (currentPosition) {

	case CustomPlotPositionButton::topLeft:
		moveTopLeft(resize);
		break;
	case CustomPlotPositionButton::topRight:
		moveTopRight(resize);
		break;
	case CustomPlotPositionButton::bottomLeft:
		moveBottomLeft(resize);
		break;
	case CustomPlotPositionButton::bottomRight:
		moveBottomRight(resize);
		break;

	default:
		break;
	}
}

void CursorReadouts::moveTopLeft(bool resize)
{
	if (!isVisible())
		return;

	int firstPos = 3;
	int secondPos = 6;

	d_topLeft = QPoint(8, 8);

	QwtScaleDiv vScaleDiv = plot()->axisScaleDiv(QwtPlot::yLeft);
	QwtScaleDiv hScaleDiv = plot()->axisScaleDiv(QwtPlot::xBottom);

	QList<double> vMajorTicks = vScaleDiv.ticks(QwtScaleDiv::MajorTick);
	QList<double> hMajorTicks = hScaleDiv.ticks(QwtScaleDiv::MajorTick);

	int vLastTick = vMajorTicks.size() - 1;

	int firstVPos = vLastTick - 2;

	if (plot()->width() > 100) {
		while (plotPointToPixelPoint(QPointF(hMajorTicks[firstPos],
						     vMajorTicks[firstVPos])).x() < 140) firstPos++;
		while (plotPointToPixelPoint(QPointF(hMajorTicks[secondPos],
						     vMajorTicks[firstVPos])).x() < 320) secondPos++;
		while (plotPointToPixelPoint(QPointF(hMajorTicks[secondPos],
						     vMajorTicks[firstVPos])).y() < 80) firstVPos--;
	}

	QPoint bottomRightFirstRect = plotPointToPixelPoint(QPointF(hMajorTicks[firstPos], vMajorTicks[firstVPos]));
	QPoint bottomRightSecondRect = plotPointToPixelPoint(QPointF(hMajorTicks[secondPos], vMajorTicks[firstVPos]));

	bottomRightFirstRect -= QPoint(0, 1);
	bottomRightSecondRect -= QPoint(1, 1);

	if (!d_time_rd_visible || !d_voltage_rd_visible) {
		bottomRightFirstRect -= QPoint(1, 0);
	}

	QRect timeRect, voltageRect;

	if (!d_time_rd_visible && d_voltage_rd_visible) {
		timeRect = QRect(d_topLeft, bottomRightFirstRect);
		voltageRect = timeRect;
	} else {
		timeRect = QRect(d_topLeft, bottomRightFirstRect);
		voltageRect = QRect(timeRect.topRight(), bottomRightSecondRect);
	}

	int diff = voltageRect.x() - lastVoltageRect.x();
	if (diff < 10 && diff > -10) diff = voltageRect.y() - lastVoltageRect.y();
	int duration = diff < 0 ? -diff : diff;
	duration = duration > 200 ? 200 : duration;
	if (resize) duration = 0;
	anim->setDuration(duration), anim2->setDuration(duration);

	QRect start = lastVoltageRect;
	QRect final = voltageRect;
	QRect start2 = lastTimeRect;
	QRect final2 = timeRect;

	anim->setStartValue(start), anim2->setStartValue(start2);
	anim->setEndValue(final), anim2->setEndValue(final2);
	anim->start(), anim2->start();

	lastTimeRect = timeRect;
	lastVoltageRect = voltageRect;
}

void CursorReadouts::moveTopRight(bool resize)
{
	if (!isVisible())
		return;

	QwtScaleDiv vScaleDiv = plot()->axisScaleDiv(QwtPlot::yLeft);
	QwtScaleDiv hScaleDiv = plot()->axisScaleDiv(QwtPlot::xBottom);

	QList<double> vMajorTicks = vScaleDiv.ticks(QwtScaleDiv::MajorTick);
	QList<double> hMajorTicks = hScaleDiv.ticks(QwtScaleDiv::MajorTick);

	int firstPos = hMajorTicks.size() - 4;
	int secondPos = hMajorTicks.size() - 1;

	int vLastTick = vMajorTicks.size() - 1;
	int firstVPos = vLastTick - 2;

	while (plotPointToPixelPoint(QPointF(hMajorTicks[firstPos], vMajorTicks[firstVPos])).x() > plot()->width() - 140) firstPos--;
	while (plotPointToPixelPoint(QPointF(hMajorTicks[firstPos], vMajorTicks[firstVPos])).y() < 80) firstVPos--;
	int topLeftPos = firstPos - 3;
	while (plotPointToPixelPoint(QPointF(hMajorTicks[topLeftPos], vMajorTicks[firstVPos] + 2)).x() > plot()->width() - 320) topLeftPos--;

	QPoint bottomRightFirstRect = plotPointToPixelPoint(QPointF(hMajorTicks[firstPos], vMajorTicks[firstVPos]));
	QPoint bottomRightSecondRect = plotPointToPixelPoint(QPointF(hMajorTicks[secondPos], vMajorTicks[firstVPos]));

	bottomRightSecondRect -= QPoint(8, 0);

	d_topLeft =  plotPointToPixelPoint(QPointF(hMajorTicks[topLeftPos], vMajorTicks[firstVPos]));
	d_topLeft.setY(8);

	bottomRightFirstRect -= QPoint(0, 1);
	bottomRightSecondRect -= QPoint(1, 1);

	if (!d_time_rd_visible || !d_voltage_rd_visible) {
		bottomRightFirstRect -= QPoint(1, 0);
	}

	QRect timeRect, voltageRect;

	if (d_time_rd_visible && !d_voltage_rd_visible) {
		voltageRect = QRect(QPoint(bottomRightFirstRect.x(), 8), bottomRightSecondRect);
		timeRect = voltageRect;

	} else {
		timeRect = QRect(d_topLeft, bottomRightFirstRect);
		voltageRect = QRect(timeRect.topRight(), bottomRightSecondRect);
	}

	int diff = timeRect.x() - lastTimeRect.x();
	if (diff < 10 && diff > -10) diff = timeRect.y() - lastTimeRect.y();
	int duration = diff < 0 ? -diff : diff;
	duration = duration > 200 ? 200 : duration;
	if (resize) duration = 0;
	anim->setDuration(duration), anim2->setDuration(duration);

	QRect start = lastVoltageRect;
	QRect final = voltageRect;
	QRect start2 = lastTimeRect;
	QRect final2 = timeRect;

	anim->setStartValue(start), anim2->setStartValue(start2);
	anim->setEndValue(final), anim2->setEndValue(final2);
	anim->start(), anim2->start();

	lastTimeRect = timeRect;
	lastVoltageRect = voltageRect;
}

void CursorReadouts::moveBottomLeft(bool resize)
{
	if (!isVisible())
		return;

	int firstPos = 3;
	int secondPos = 6;

	QwtScaleDiv vScaleDiv = plot()->axisScaleDiv(QwtPlot::yLeft);
	QwtScaleDiv hScaleDiv = plot()->axisScaleDiv(QwtPlot::xBottom);

	QList<double> vMajorTicks = vScaleDiv.ticks(QwtScaleDiv::MajorTick);
	QList<double> hMajorTicks = hScaleDiv.ticks(QwtScaleDiv::MajorTick);

	int firstVPos = 0;

	while (plotPointToPixelPoint(QPointF(hMajorTicks[firstPos], vMajorTicks[firstVPos])).x() < 140) firstPos++;
	while (plotPointToPixelPoint(QPointF(hMajorTicks[secondPos], vMajorTicks[firstVPos])).x() < 320) secondPos++;
	while (plotPointToPixelPoint(QPointF(hMajorTicks[firstPos], vMajorTicks[firstVPos])).y() > plot()->height() - 80) firstVPos++;

	QPoint bottomRightFirstRect = plotPointToPixelPoint(QPointF(hMajorTicks[firstPos], vMajorTicks[0]));
	QPoint bottomRightSecondRect = plotPointToPixelPoint(QPointF(hMajorTicks[secondPos], vMajorTicks[0]));

	d_topLeft = plotPointToPixelPoint(QPointF(hMajorTicks[firstPos], vMajorTicks[firstVPos]));
	d_topLeft.setX(8);

	bottomRightFirstRect -= QPoint(0, 8);
	bottomRightSecondRect -= QPoint(0, 8);

	bottomRightFirstRect -= QPoint(0, 1);
	bottomRightSecondRect -= QPoint(1, 1);

	if (!d_time_rd_visible || !d_voltage_rd_visible) {
		bottomRightFirstRect -= QPoint(1, 0);
	}

	QRect timeRect, voltageRect;

	if (!d_time_rd_visible && d_voltage_rd_visible) {
		timeRect = QRect(d_topLeft, bottomRightFirstRect);
		voltageRect = timeRect;
	} else {
		timeRect = QRect(d_topLeft, bottomRightFirstRect);
		voltageRect = QRect(timeRect.topRight(), bottomRightSecondRect);
	}

	int diff = voltageRect.x() - lastVoltageRect.x();
	if (diff < 10 && diff > -10) diff = voltageRect.y() - lastVoltageRect.y();
	int duration = diff < 0 ? -diff : diff;
	duration = duration > 200 ? 200 : duration;
	if (resize) duration = 0;
	anim->setDuration(duration), anim2->setDuration(duration);

	QRect start = lastVoltageRect;
	QRect final = voltageRect;
	QRect start2 = lastTimeRect;
	QRect final2 = timeRect;

	anim->setStartValue(start), anim2->setStartValue(start2);
	anim->setEndValue(final), anim2->setEndValue(final2);
	anim->start(), anim2->start();

	lastTimeRect = timeRect;
	lastVoltageRect = voltageRect;
}

void CursorReadouts::moveBottomRight(bool resize)
{
	if (!isVisible())
		return;

	QwtScaleDiv vScaleDiv = plot()->axisScaleDiv(QwtPlot::yLeft);
	QwtScaleDiv hScaleDiv = plot()->axisScaleDiv(QwtPlot::xBottom);

	QList<double> vMajorTicks = vScaleDiv.ticks(QwtScaleDiv::MajorTick);
	QList<double> hMajorTicks = hScaleDiv.ticks(QwtScaleDiv::MajorTick);

	int firstPos = hMajorTicks.size() - 4;
	int secondPos = hMajorTicks.size() - 1;

	int vLastTick = vMajorTicks.size() - 1;
	int firstVPos = 0;

	while (plotPointToPixelPoint(QPointF(hMajorTicks[firstPos], vMajorTicks[0])).x() > plot()->width() - 140) firstPos--;
	int topLeftPos = firstPos - 3;
	while (plotPointToPixelPoint(QPointF(hMajorTicks[topLeftPos], vMajorTicks[firstVPos] + 2)).x() > plot()->width() - 320) topLeftPos--;

	while (plotPointToPixelPoint(QPointF(hMajorTicks[firstPos], vMajorTicks[firstVPos])).y() > plot()->height() - 80) firstVPos++;

	QPoint bottomRightFirstRect = plotPointToPixelPoint(QPointF(hMajorTicks[firstPos], vMajorTicks[0]));
	QPoint bottomRightSecondRect = plotPointToPixelPoint(QPointF(hMajorTicks[secondPos], vMajorTicks[0]));

	bottomRightFirstRect -= QPoint(8, 8);
	bottomRightSecondRect -= QPoint(8, 8);

	d_topLeft =  plotPointToPixelPoint(QPointF(hMajorTicks[topLeftPos], vMajorTicks[firstVPos]));

	bottomRightFirstRect -= QPoint(0, 1);
	bottomRightSecondRect -= QPoint(1, 1);

	if (!d_time_rd_visible || !d_voltage_rd_visible) {
		bottomRightFirstRect -= QPoint(1, 0);
	}

	QRect timeRect, voltageRect;

	if (d_time_rd_visible && !d_voltage_rd_visible) {
		voltageRect = QRect(QPoint(bottomRightFirstRect.x(), d_topLeft.y()), bottomRightSecondRect);
		timeRect = voltageRect;

	} else {
		timeRect = QRect(d_topLeft, bottomRightFirstRect);
		voltageRect = QRect(timeRect.topRight(), bottomRightSecondRect);
	}

	int diff = timeRect.x() - lastTimeRect.x();
	if (diff < 10 && diff > -10) diff = timeRect.y() - lastTimeRect.y();
	int duration = diff < 0 ? -diff : diff;
	duration = duration > 200 ? 200 : duration;
	if (resize) duration = 0;
	anim->setDuration(duration), anim2->setDuration(duration);

	QRect start = lastVoltageRect;
	QRect final = voltageRect;
	QRect start2 = lastTimeRect;
	QRect final2 = timeRect;

	anim->setStartValue(start), anim2->setStartValue(start2);
	anim->setEndValue(final), anim2->setEndValue(final2);
	anim->start(), anim2->start();

	lastTimeRect = timeRect;
	lastVoltageRect = voltageRect;
}
