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
    currentPosition(CustomPlotPositionButton::topLeft),
    hAxis(QwtPlot::xBottom),
    vAxis(QwtPlot::yLeft)
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

void CursorReadouts::setTimeDeltaVisible(bool visible){
    ui->timeDeltaLabel->setVisible(visible);
    ui->timeDelta->setVisible(visible);
}

void CursorReadouts::setFrequencyDeltaVisible(bool visible){
    ui->frequencyDeltaLabel->setVisible(visible);
    ui->frequencyDelta->setVisible(visible);
}

void CursorReadouts::setTimeCursor1LabelText(const QString &text){
    ui->cursorT1label->setText(text);
}

QString CursorReadouts::timeCursor1LabelText(){
    return ui->cursorT1label->text();
}

void CursorReadouts::setTimeCursor2LabelText(const QString &text){
    ui->cursorT2label->setText(text);
}

QString CursorReadouts::timeCursor2LabelText(){
    return ui->cursorT2label->text();
}

void CursorReadouts::setVoltageCursor1LabelText(const QString &text){
    ui->cursorV1label->setText(text);
}

QString CursorReadouts::voltageCursor1LabelText(){
    return ui->cursorV2label->text();
}

void CursorReadouts::setVoltageCursor2LabelText(const QString &text){
    ui->cursorV2label->setText(text);
}

QString CursorReadouts::voltageCursor2LabelText(){
    return ui->cursorV2label->text();
}

void CursorReadouts::setDeltaVoltageLabelText(const QString &text){
    ui->deltaVlabel->setText(text);
}

QString CursorReadouts::deltaVoltageLabelText(){
    return ui->deltaVlabel->text();
}

void CursorReadouts::setAxis(QwtAxisId hAxis,QwtAxisId vAxis){
    this->hAxis = hAxis;
    this->vAxis = vAxis;
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
    const QwtScaleMap xMap = plot()->canvasMap(hAxis);
    const QwtScaleMap yMap = plot()->canvasMap(vAxis);

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

	d_topLeft = QPoint(8, 8);

    QRect timeRect, voltageRect;

    if (!d_time_rd_visible && d_voltage_rd_visible) {
            voltageRect = QRect(d_topLeft, QPoint(d_topLeft.x()+ui->VoltageCursors->width(),d_topLeft.y()+ui->VoltageCursors->height()));
            timeRect = QRect(0,0,0,0);
    } else {
        timeRect = QRect(d_topLeft, QPoint(ui->TimeCursors->width()+d_topLeft.x(),ui->TimeCursors->height()+d_topLeft.y()));
        voltageRect = QRect(QPoint(d_topLeft.x()+timeRect.width(),d_topLeft.y()), QPoint(d_topLeft.x()+timeRect.width()+ui->VoltageCursors->width(),ui->VoltageCursors->height()+d_topLeft.y()));
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

	d_topLeft.setY(8);
    d_topLeft.setX(plot()->canvas()->width()-8);


	QRect timeRect, voltageRect;

	if (d_time_rd_visible && !d_voltage_rd_visible) {
        voltageRect = QRect(0,0,0,0);
        timeRect = QRect(d_topLeft.x() - ui->TimeCursors->width(),d_topLeft.y(),d_topLeft.x(),d_topLeft.y()+ui->TimeCursors->height());
	} else {
        voltageRect = QRect(d_topLeft.x() - ui->VoltageCursors->width(),d_topLeft.y(),d_topLeft.x(),d_topLeft.y()+ui->VoltageCursors->height());
        timeRect = QRect(voltageRect.x()-ui->TimeCursors->width(),d_topLeft.y(),voltageRect.x(),d_topLeft.y()+ui->VoltageCursors->height());

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

	QRect timeRect, voltageRect;

    d_topLeft.setY(plot()->height()-8);
    d_topLeft.setX(8);

    if (!d_time_rd_visible && d_voltage_rd_visible) {
            voltageRect = QRect(QPoint(d_topLeft.x(),d_topLeft.y()-ui->VoltageCursors->height()-20), QPoint(d_topLeft.x()+ui->VoltageCursors->width(),d_topLeft.y()));
            timeRect = QRect(0,0,0,0);
    } else {
        timeRect = QRect(QPoint(d_topLeft.x(),d_topLeft.y()-ui->TimeCursors->height()-20), QPoint(ui->TimeCursors->width()+d_topLeft.x(),d_topLeft.y()));
        voltageRect = QRect(QPoint(d_topLeft.x()+timeRect.width(),d_topLeft.y()-ui->VoltageCursors->height()-20), QPoint(d_topLeft.x()+timeRect.width()+ui->VoltageCursors->width(),d_topLeft.y()));
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

	QRect timeRect, voltageRect;

    d_topLeft.setY(plot()->height()-8);
    d_topLeft.setX(plot()->canvas()->width()-8);

    if (d_time_rd_visible && !d_voltage_rd_visible) {
        voltageRect = QRect(0,0,0,0);
        timeRect = QRect(d_topLeft.x() - ui->TimeCursors->width(),d_topLeft.y()-ui->TimeCursors->height()-20,d_topLeft.x(),d_topLeft.y());
    } else {
        voltageRect = QRect(d_topLeft.x() - ui->VoltageCursors->width(),d_topLeft.y()-ui->VoltageCursors->height()-20,d_topLeft.x(),d_topLeft.y());
        timeRect = QRect(voltageRect.x()-ui->TimeCursors->width(),voltageRect.y(),voltageRect.x(),voltageRect.y()+ui->TimeCursors->height());

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
