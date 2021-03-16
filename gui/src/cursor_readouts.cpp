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

#include "ui_cursor_readouts.h"

#include <QGridLayout>
#include <QIcon>
#include <QLabel>
#include <qevent.h>
#include <qwt_plot.h>
#include <qwt_scale_div.h>
#include <qwt_scale_map.h>

#include <scopy/gui/cursor_readouts.hpp>

using namespace scopy::gui;

CursorReadouts::CursorReadouts(QwtPlot* plot)
	: QWidget(plot)
	, m_ui(new Ui::CursorReadouts)
	, m_DVoltageRdVisible(true)
	, m_DTimeRdVisible(true)
	, m_topLeft(QPoint(0, 0))
	, m_currentPosition(CustomPlotPositionButton::topLeft)
	, m_hAxis(QwtPlot::xBottom)
	, m_vAxis(QwtPlot::yLeft)
{
	m_ui->setupUi(this);
	m_ui->widgetTimeCursors->setParent(plot->canvas());
	m_ui->widgetVoltageCursors->setParent(plot->canvas());
	this->setGeometry(0, 0, 0, 0);

	plot->canvas()->installEventFilter(this);

	m_ui->widgetTimeCursors->setAttribute(Qt::WA_TransparentForMouseEvents);
	m_ui->widgetVoltageCursors->setAttribute(Qt::WA_TransparentForMouseEvents);

	m_anim = new CustomAnimation(m_ui->widgetVoltageCursors, "geometry");
	m_anim2 = new CustomAnimation(m_ui->widgetTimeCursors, "geometry");

	setTransparency(0);
}

CursorReadouts::~CursorReadouts()
{
	delete m_anim;
	delete m_anim2;
	delete m_ui;
}

void CursorReadouts::setTransparency(int value)
{
	double percent = (100 - value) / 100.0;

	QString color = "";
	if (QIcon::themeName() == "scopy-default") {
		color += "rgba(20, 20, 22, ";
	} else {
		color += "rgba(197, 197, 197, ";
	}

	color += QString::number(percent);

	m_ui->widgetTimeCursors->setStyleSheet("QWidget {"
					       "background-color: " +
					       color +
					       ");"
					       "}");
	m_ui->widgetVoltageCursors->setStyleSheet("QWidget {"
						  "background-color: " +
						  color +
						  ");"
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

	m_currentPosition = position;
}

CustomPlotPositionButton::ReadoutsPosition CursorReadouts::getCurrentPosition() { return m_currentPosition; }

void CursorReadouts::showEvent(QShowEvent* event)
{
	updateSizeAndPosition(true);

	QWidget::showEvent(event);
}

QwtPlot* CursorReadouts::plot() { return static_cast<QwtPlot*>(parent()); }

const QwtPlot* CursorReadouts::plot() const { return static_cast<const QwtPlot*>(parent()); }

void CursorReadouts::setVoltageReadoutVisible(bool on)
{
	if (m_DVoltageRdVisible != on) {
		m_DVoltageRdVisible = on;
		m_ui->widgetVoltageCursors->setVisible(on);
		updateSizeAndPosition();
	}
}

bool CursorReadouts::isVoltageReadoutVisible() const { return m_DVoltageRdVisible; }

void CursorReadouts::setTimeReadoutVisible(bool on)
{
	if (m_DTimeRdVisible != on) {
		m_DTimeRdVisible = on;
		m_ui->widgetTimeCursors->setVisible(on);
		updateSizeAndPosition();
	}
}

bool CursorReadouts::isTimeReadoutVisible() const { return m_DTimeRdVisible; }

void CursorReadouts::setTopLeftStartingPoint(QPoint point) { m_topLeft = point; }

QPoint CursorReadouts::topLeftStartingPoint() const { return m_topLeft; }

void CursorReadouts::setTimeCursor1Text(const QString& text) { m_ui->lblCursorT1->setText(text); }

QString CursorReadouts::timeCursor1Text() const { return m_ui->lblCursorT1->text(); }

void CursorReadouts::setTimeCursor2Text(const QString& text) { m_ui->lblCursorT2->setText(text); }

QString CursorReadouts::timeCursor2Text() const { return m_ui->lblCursorT2->text(); }

void CursorReadouts::setTimeDeltaText(const QString& text) { m_ui->lblTimeDelta->setText(text); }

QString CursorReadouts::timeDeltaText() const { return m_ui->lblTimeDelta->text(); }

void CursorReadouts::setFreqDeltaText(const QString& text) { m_ui->lblFrequencyDelta->setText(text); }

QString CursorReadouts::freqDeltaText() const { return m_ui->lblFrequencyDelta->text(); }

void CursorReadouts::setVoltageCursor1Text(const QString& text) { m_ui->lblCursorV1->setText(text); }

QString CursorReadouts::voltageCursor1Text() const { return m_ui->lblCursorV1->text(); }

void CursorReadouts::setVoltageCursor2Text(const QString& text) { m_ui->lblCursorV2->setText(text); }

QString CursorReadouts::voltageCursor2Text() const { return m_ui->lblCursorV2->text(); }

void CursorReadouts::setVoltageDeltaText(const QString& text) { m_ui->lblVoltageDelta->setText(text); }

QString CursorReadouts::voltageDeltaText() const { return m_ui->lblVoltageDelta->text(); }

void CursorReadouts::setTimeDeltaVisible(bool visible)
{
	m_ui->lblTimeDeltaLabel->setVisible(visible);
	m_ui->lblTimeDelta->setVisible(visible);
}

void CursorReadouts::setFrequencyDeltaVisible(bool visible)
{
	m_ui->lblFrequencyDeltaLabel->setVisible(visible);
	m_ui->lblFrequencyDelta->setVisible(visible);
}

void CursorReadouts::setTimeCursor1LabelText(const QString& text) { m_ui->lblCursorT1label->setText(text); }

QString CursorReadouts::timeCursor1LabelText() const { return m_ui->lblCursorT1label->text(); }

void CursorReadouts::setTimeCursor2LabelText(const QString& text) { m_ui->lblCursorT2label->setText(text); }

QString CursorReadouts::timeCursor2LabelText() const { return m_ui->lblCursorT2label->text(); }

void CursorReadouts::setVoltageCursor1LabelText(const QString& text) { m_ui->lblCursorV1label->setText(text); }

QString CursorReadouts::voltageCursor1LabelText() const { return m_ui->lblCursorV2label->text(); }

void CursorReadouts::setVoltageCursor2LabelText(const QString& text) { m_ui->lblCursorV2label->setText(text); }

QString CursorReadouts::voltageCursor2LabelText() const { return m_ui->lblCursorV2label->text(); }

void CursorReadouts::setDeltaVoltageLabelText(const QString& text) { m_ui->lblDeltaVlabel->setText(text); }

QString CursorReadouts::deltaVoltageLabelText() const { return m_ui->lblDeltaVlabel->text(); }

void CursorReadouts::setAxis(QwtAxisId hAxis, QwtAxisId vAxis)
{
	this->m_hAxis = hAxis;
	this->m_vAxis = vAxis;
}

bool CursorReadouts::eventFilter(QObject* object, QEvent* event)
{
	if (object == plot()->canvas()) {
		switch (event->type()) {
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

QPoint CursorReadouts::plotPointToPixelPoint(const QPointF& point) const
{
	const QwtScaleMap xMap = plot()->canvasMap(m_hAxis);
	const QwtScaleMap yMap = plot()->canvasMap(m_vAxis);

	return QwtScaleMap::transform(xMap, yMap, point).toPoint();
}

void CursorReadouts::updateSizeAndPosition(bool resize)
{
	switch (m_currentPosition) {

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

	m_topLeft = QPoint(8, 8);

	QRect timeRect, voltageRect;

	if (!m_DTimeRdVisible && m_DVoltageRdVisible) {
		voltageRect = QRect(m_topLeft,
				    QPoint(m_topLeft.x() + m_ui->widgetVoltageCursors->width(),
					   m_topLeft.y() + m_ui->widgetVoltageCursors->height()));
		timeRect = QRect(0, 0, 0, 0);
	} else {
		timeRect = QRect(m_topLeft,
				 QPoint(m_ui->widgetTimeCursors->width() + m_topLeft.x(),
					m_ui->widgetTimeCursors->height() + m_topLeft.y()));
		voltageRect = QRect(QPoint(m_topLeft.x() + timeRect.width(), m_topLeft.y()),
				    QPoint(m_topLeft.x() + timeRect.width() + m_ui->widgetVoltageCursors->width(),
					   m_ui->widgetVoltageCursors->height() + m_topLeft.y()));
	}

	int diff = voltageRect.x() - m_lastVoltageRect.x();
	if (diff < 10 && diff > -10)
		diff = voltageRect.y() - m_lastVoltageRect.y();
	int duration = diff < 0 ? -diff : diff;
	duration = duration > 200 ? 200 : duration;
	if (resize)
		duration = 0;
	m_anim->setDuration(duration), m_anim2->setDuration(duration);

	QRect start = m_lastVoltageRect;
	QRect final = voltageRect;
	QRect start2 = m_lastTimeRect;
	QRect final2 = timeRect;

	m_anim->setStartValue(start), m_anim2->setStartValue(start2);
	m_anim->setEndValue(final), m_anim2->setEndValue(final2);
	m_anim->start(), m_anim2->start();

	m_lastTimeRect = timeRect;
	m_lastVoltageRect = voltageRect;
}

void CursorReadouts::moveTopRight(bool resize)
{
	if (!isVisible())
		return;

	m_topLeft.setY(8);
	m_topLeft.setX(plot()->canvas()->width() - 8);

	QRect timeRect, voltageRect;

	if (m_DTimeRdVisible && !m_DVoltageRdVisible) {
		voltageRect = QRect(0, 0, 0, 0);
		timeRect = QRect(m_topLeft.x() - m_ui->widgetTimeCursors->width(), m_topLeft.y(), m_topLeft.x(),
				 m_topLeft.y() + m_ui->widgetTimeCursors->height());
	} else {
		voltageRect = QRect(m_topLeft.x() - m_ui->widgetVoltageCursors->width(), m_topLeft.y(), m_topLeft.x(),
				    m_topLeft.y() + m_ui->widgetVoltageCursors->height());
		timeRect = QRect(voltageRect.x() - m_ui->widgetTimeCursors->width(), m_topLeft.y(), voltageRect.x(),
				 m_topLeft.y() + m_ui->widgetVoltageCursors->height());
	}

	int diff = timeRect.x() - m_lastTimeRect.x();
	if (diff < 10 && diff > -10)
		diff = timeRect.y() - m_lastTimeRect.y();
	int duration = diff < 0 ? -diff : diff;
	duration = duration > 200 ? 200 : duration;
	if (resize)
		duration = 0;
	m_anim->setDuration(duration), m_anim2->setDuration(duration);

	QRect start = m_lastVoltageRect;
	QRect final = voltageRect;
	QRect start2 = m_lastTimeRect;
	QRect final2 = timeRect;

	m_anim->setStartValue(start), m_anim2->setStartValue(start2);
	m_anim->setEndValue(final), m_anim2->setEndValue(final2);
	m_anim->start(), m_anim2->start();

	m_lastTimeRect = timeRect;
	m_lastVoltageRect = voltageRect;
}

void CursorReadouts::moveBottomLeft(bool resize)
{
	if (!isVisible())
		return;

	QRect timeRect, voltageRect;

	m_topLeft.setY(plot()->height() - 8);
	m_topLeft.setX(8);

	if (!m_DTimeRdVisible && m_DVoltageRdVisible) {
		voltageRect = QRect(QPoint(m_topLeft.x(), m_topLeft.y() - m_ui->widgetVoltageCursors->height() - 20),
				    QPoint(m_topLeft.x() + m_ui->widgetVoltageCursors->width(), m_topLeft.y()));
		timeRect = QRect(0, 0, 0, 0);
	} else {
		timeRect = QRect(QPoint(m_topLeft.x(), m_topLeft.y() - m_ui->widgetTimeCursors->height() - 20),
				 QPoint(m_ui->widgetTimeCursors->width() + m_topLeft.x(), m_topLeft.y()));
		voltageRect = QRect(
			QPoint(m_topLeft.x() + timeRect.width(),
			       m_topLeft.y() - m_ui->widgetVoltageCursors->height() - 20),
			QPoint(m_topLeft.x() + timeRect.width() + m_ui->widgetVoltageCursors->width(), m_topLeft.y()));
	}

	int diff = voltageRect.x() - m_lastVoltageRect.x();
	if (diff < 10 && diff > -10)
		diff = voltageRect.y() - m_lastVoltageRect.y();
	int duration = diff < 0 ? -diff : diff;
	duration = duration > 200 ? 200 : duration;
	if (resize)
		duration = 0;
	m_anim->setDuration(duration), m_anim2->setDuration(duration);

	QRect start = m_lastVoltageRect;
	QRect final = voltageRect;
	QRect start2 = m_lastTimeRect;
	QRect final2 = timeRect;

	m_anim->setStartValue(start), m_anim2->setStartValue(start2);
	m_anim->setEndValue(final), m_anim2->setEndValue(final2);
	m_anim->start(), m_anim2->start();

	m_lastTimeRect = timeRect;
	m_lastVoltageRect = voltageRect;
}

void CursorReadouts::moveBottomRight(bool resize)
{
	if (!isVisible())
		return;

	QRect timeRect, voltageRect;

	m_topLeft.setY(plot()->height() - 8);
	m_topLeft.setX(plot()->canvas()->width() - 8);

	if (m_DTimeRdVisible && !m_DVoltageRdVisible) {
		voltageRect = QRect(0, 0, 0, 0);
		timeRect = QRect(m_topLeft.x() - m_ui->widgetTimeCursors->width(),
				 m_topLeft.y() - m_ui->widgetTimeCursors->height() - 20, m_topLeft.x(), m_topLeft.y());
	} else {
		voltageRect =
			QRect(m_topLeft.x() - m_ui->widgetVoltageCursors->width(),
			      m_topLeft.y() - m_ui->widgetVoltageCursors->height() - 20, m_topLeft.x(), m_topLeft.y());
		timeRect = QRect(voltageRect.x() - m_ui->widgetTimeCursors->width(), voltageRect.y(), voltageRect.x(),
				 voltageRect.y() + m_ui->widgetTimeCursors->height());
	}

	int diff = timeRect.x() - m_lastTimeRect.x();
	if (diff < 10 && diff > -10)
		diff = timeRect.y() - m_lastTimeRect.y();
	int duration = diff < 0 ? -diff : diff;
	duration = duration > 200 ? 200 : duration;
	if (resize)
		duration = 0;
	m_anim->setDuration(duration), m_anim2->setDuration(duration);

	QRect start = m_lastVoltageRect;
	QRect final = voltageRect;
	QRect start2 = m_lastTimeRect;
	QRect final2 = timeRect;

	m_anim->setStartValue(start), m_anim2->setStartValue(start2);
	m_anim->setEndValue(final), m_anim2->setEndValue(final2);
	m_anim->start(), m_anim2->start();

	m_lastTimeRect = timeRect;
	m_lastVoltageRect = voltageRect;
}
