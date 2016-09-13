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
	d_topLeft(QPoint(0, 0))
{
	ui->setupUi(this);

	ui->TimeCursors->setParent(plot->canvas());
	ui->VoltageCursors->setParent(plot->canvas());
	this->setGeometry(0, 0, 0, 0);

	plot->canvas()->installEventFilter(this);
}

CursorReadouts::~CursorReadouts()
{
	delete ui;
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
				updateSizeAndPosition();
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

void CursorReadouts::updateSizeAndPosition()
{
	QwtScaleDiv vScaleDiv = plot()->axisScaleDiv(QwtPlot::yLeft);
	QwtScaleDiv hScaleDiv = plot()->axisScaleDiv(QwtPlot::xBottom);

	QList<double> vMajorTicks = vScaleDiv.ticks(QwtScaleDiv::MajorTick);
	QList<double> hMajorTicks = hScaleDiv.ticks(QwtScaleDiv::MajorTick);

	int vLastTick = vMajorTicks.size() - 1;

	QPoint bottomRightFirstRect = plotPointToPixelPoint(QPointF(hMajorTicks[3], vMajorTicks[vLastTick - 2]));
	QPoint bottomRightSecondRect = plotPointToPixelPoint(QPointF(hMajorTicks[6], vMajorTicks[vLastTick - 2]));

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

	if (d_voltage_rd_visible)
		ui->VoltageCursors->setGeometry(voltageRect);
	if (d_time_rd_visible)
		ui->TimeCursors->setGeometry(timeRect);
}
