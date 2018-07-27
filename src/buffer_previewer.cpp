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

#include "buffer_previewer.hpp"

#include <QtMath>
#include <QResizeEvent>
#include <QPainter>
#include <QPointF>

using namespace adiscope;

/*
 * Class BufferPreviewer implementation
 */

BufferPreviewer::BufferPreviewer(QWidget *parent):
	BufferPreviewer(50, 0.0, parent)
{
}

BufferPreviewer::BufferPreviewer(int pixelsPerPeriod, double wavePhase,
	QWidget *parent):
	QFrame(parent),
	m_waveformPos(0.0),
	m_waveformWidth(1.0),
	m_highlightPos(0.0),
	m_highlightWidth(0.5),
	m_cursorPos(0.0),
	m_verticalSpacing(0),
	m_pixelsPerPeriod(pixelsPerPeriod),
	m_startingPhase(wavePhase),
	m_fullWaveNumPoints(1),
	m_fullWavePoints(new QPointF[m_fullWaveNumPoints]),
	m_rightBtnClick(false),
	m_gatingEnabled(false)
{
}

BufferPreviewer::~BufferPreviewer()
{
	delete[] m_fullWavePoints;
}

double BufferPreviewer::waveformPos() const
{
	return m_waveformPos;
}

void BufferPreviewer::setWaveformPos(double pos)
{
	if (pos < 0.0 || pos > 1.0 || pos == m_waveformPos)
		return;

	m_waveformPos = pos;
	update();
}

double BufferPreviewer::waveformWidth() const
{
	return m_waveformWidth;
}

void BufferPreviewer::setWaveformWidth(double width)
{
	if (width < 0.0 || width > 1.0 || width == m_waveformWidth)
		return;

	m_waveformWidth = width;
	update();
}

double BufferPreviewer::highlighPos() const
{
	return m_highlightPos;
}

void BufferPreviewer::setHighlightPos(double pos)
{
	if (pos < 0.0 || pos > 1.0 || pos == m_highlightPos)
		return;

	m_highlightPos = pos;
	update();
}

double BufferPreviewer::highlightWidth() const
{
	return m_highlightPos;
}

void BufferPreviewer::setHighlightWidth(double width)
{
	if (width < 0.0 || width > 1.0 || width == m_highlightWidth)
		return;

	m_highlightWidth = width;
	update();
}

double BufferPreviewer::cursorPos() const
{
	return m_cursorPos;
}
void BufferPreviewer::setCursorPos(double pos)
{
	if (pos < 0.0 || pos > 1.0 || pos == m_cursorPos)
		return;

	m_cursorPos = pos;
	update();
}

int BufferPreviewer::verticalSpacing() const
{
	return m_verticalSpacing;
}

void BufferPreviewer::setVerticalSpacing(int spacing)
{
	if (spacing  < 0)
		spacing = 0;

	if (m_verticalSpacing != spacing) {
		m_verticalSpacing = spacing;

		buildFullWaveform(m_fullWavePoints, m_fullWaveNumPoints);
		update();
	}
}

int BufferPreviewer::pixelsPerPeriod() const
{
	return m_pixelsPerPeriod;
}

double BufferPreviewer::wavePhase() const
{
	return m_startingPhase;
}

void BufferPreviewer::setGatingEnabled(bool enabled)
{
	m_gatingEnabled = enabled;
	update();
}

void BufferPreviewer::setLeftGateWidth(double width)
{
	m_leftGateWidth = width;
	update();
}

void BufferPreviewer::setRightGateWidth(double width)
{
	m_rightGateWidth = width;
	update();
}

void BufferPreviewer::paintEvent(QPaintEvent *)
{
	QPainter p(this);

	p.translate(contentsRect().topLeft());
	int w = contentsRect().width();
	int h = contentsRect().height();

	p.setClipRect(0, 0, w, h);

	int hlight_start = qRound(m_highlightPos * w);
	int hlight_width = qRound(m_highlightWidth * w);
	int wave_start = qRound(m_waveformPos * w);
	int wave_width = qRound(m_waveformWidth * w);
	int cursor_start = qRound(m_cursorPos * w);

	// Limit width of movable elements so they don't exceed the widget width
	if (hlight_start + hlight_width > w)
		hlight_width = w - hlight_start;
	if (wave_start + wave_width > w)
		wave_width = w - wave_start;

	// Get intersection between the visible wave and the highlight rectangle
	int hlightedWaveStartPos = qMax(hlight_start, wave_start);
	int hlightedWaveEndPos = qMin((hlight_start + hlight_width),
		(wave_start + wave_width));
	int hlightedWaveWidth = hlightedWaveEndPos - hlightedWaveStartPos;

	/* Start drawing */
	QPen rectPen = p.pen();
	QPen linePen = p.pen();

	rectPen.setStyle(Qt::NoPen);
	linePen.setWidthF(1.5);

	// Draw the visible part of the entire wave
	p.setRenderHint(QPainter::Antialiasing, true);
	p.setPen(linePen);
	p.drawPolyline(m_fullWavePoints + wave_start, wave_width);

	// Draw the highlight rectangle
	p.setRenderHint(QPainter::Antialiasing, false);
	p.setPen(rectPen);
	p.setBrush(palette().color(QPalette::Highlight));
	p.drawRect(hlight_start, 0, hlight_width, h);

	// Highlight the visible wave that intersects the highlight rectangle
	p.setRenderHint(QPainter::Antialiasing, true);
	linePen.setColor(palette().color(QPalette::HighlightedText));
	p.setPen(linePen);
	p.drawPolyline(m_fullWavePoints + hlightedWaveStartPos , hlightedWaveWidth);

	//Draw two vertical lines at the start and end of the highlight;
	if (hlight_start + hlight_width > wave_start
			&& wave_start + wave_width > hlight_start) {
		int line_w = 2;
		p.setPen(rectPen);
		p.setBrush(palette().color(QPalette::HighlightedText));
		p.drawRect(hlight_start - line_w, 0, line_w, h);
		p.drawRect(hlight_start + hlight_width, 0, line_w, h);
	}

	//Draw Cursor
	p.setRenderHint(QPainter::Antialiasing, false);
	int cur_head_w = 8;
	int cur_head_h = 4;
	p.setPen(rectPen);
	p.setBrush(palette().color(QPalette::AlternateBase));
	p.drawRect((cursor_start - 1) - cur_head_w / 2 + 1, 0, cur_head_w, cur_head_h);
	p.drawRect((cursor_start - 1), cur_head_h, 2, h - cur_head_h);

	//Draw gatings if enabled
	if(m_gatingEnabled){
		int leftGateWidth = hlight_width * m_leftGateWidth + hlight_start;
		int rightGateWidth = hlight_width * m_rightGateWidth;
		QBrush gateBrush(QColor(0,15,150,130));
		p.setRenderHint(QPainter::Antialiasing,true);
		//draw left gate
		p.fillRect(0, 0, leftGateWidth,h, gateBrush);
		//draw right gate
		p.fillRect(hlight_start+hlight_width-rightGateWidth, 0,rightGateWidth+(w-hlight_start+hlight_width),h, gateBrush);
	}
}

void BufferPreviewer::resizeEvent(QResizeEvent *)
{
	delete[] m_fullWavePoints;
	m_fullWaveNumPoints = contentsRect().width();
	m_fullWavePoints = new QPointF[m_fullWaveNumPoints];
	buildFullWaveform(m_fullWavePoints, m_fullWaveNumPoints);
}

void BufferPreviewer::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::RightButton) {
			m_rightBtnClick = true;
	} else {
		setCursor(Qt::ClosedHandCursor);
		m_offset = event->pos();
	}
}

void BufferPreviewer::mouseMoveEvent(QMouseEvent *event)
{
	if (!m_rightBtnClick) {
		int value = (event->pos() - m_offset).x();
		Q_EMIT bufferMovedBy(value);
	}
}

void BufferPreviewer::mouseReleaseEvent(QMouseEvent *event)
{
	if (m_rightBtnClick) {
		Q_EMIT bufferResetPosition();
		m_rightBtnClick = false;
	} else {
		setCursor(Qt::OpenHandCursor);
		Q_EMIT bufferStopDrag();
	}
}

void BufferPreviewer::enterEvent(QEvent *event)
{
	setCursor(Qt::OpenHandCursor);
}

void BufferPreviewer::leaveEvent(QEvent *event)
{
	setCursor(Qt::ArrowCursor);
}

/*
 * Class AnalogBufferPrevier implementation
 */

AnalogBufferPreviewer::AnalogBufferPreviewer(QWidget *parent):
	BufferPreviewer(parent)
{
}

AnalogBufferPreviewer::AnalogBufferPreviewer(int pixelsPerPeriod,
	double wavePhase, QWidget *parent):
	BufferPreviewer(pixelsPerPeriod, wavePhase, parent)
{
}

void AnalogBufferPreviewer::buildFullWaveform(QPointF *wavePoints, int numPts)
{
	int middle = contentsRect().height() / 2;
	int amplitude = middle - verticalSpacing() / 2;

	for (int i = 0; i < numPts; i++) {
		qreal y = middle + amplitude * qSin(2 * M_PI * i /
			pixelsPerPeriod() + wavePhase());
		wavePoints[i] = QPointF(i, y);
	}
}

/*
 * Class DigitalBufferPreviewer implementation
 */

DigitalBufferPreviewer::DigitalBufferPreviewer(QWidget *parent):
	BufferPreviewer(parent), m_noOfSteps(0)
{
}

DigitalBufferPreviewer::DigitalBufferPreviewer(int pixelsPerPeriod,
	QWidget *parent):
	BufferPreviewer(pixelsPerPeriod, M_PI / 2, parent), m_noOfSteps(0)
{
}

void DigitalBufferPreviewer::setNoOfSteps(double val)
{
	m_noOfSteps = val;
}

double DigitalBufferPreviewer::noOfSteps()
{
	return m_noOfSteps;
}

void DigitalBufferPreviewer::buildFullWaveform(QPointF *wavePoints, int numPts)
{
	for (int i = 0; i < numPts; i++) {
		qreal y;
		int pos = i % pixelsPerPeriod();
		if( pos < pixelsPerPeriod() / 2) {
			y = contentsRect().height() - verticalSpacing() / 2;
		}
		else {
			y = verticalSpacing() / 2;
		}
		wavePoints[i] = QPointF(i, y);
	}
}

