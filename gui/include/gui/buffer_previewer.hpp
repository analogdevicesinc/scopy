
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

#ifndef BUFFER_PREVIEWER_H
#define BUFFER_PREVIEWER_H

#include "scopy-gui_export.h"

#include <QFrame>

namespace scopy {

class SCOPY_GUI_EXPORT BufferPreviewer : public QFrame
{
	Q_OBJECT

public:
	explicit BufferPreviewer(QWidget *parent = 0);
	explicit BufferPreviewer(int pixelsPerPeriod, double wavePhase, QWidget *parent = 0);
	virtual ~BufferPreviewer();

	double waveformPos() const;
	void setWaveformPos(double);
	double waveformWidth() const;
	void setWaveformWidth(double);
	double highlightPos() const;
	void setHighlightPos(double);
	double highlightWidth() const;
	void setHighlightWidth(double);
	double cursorPos() const;
	void setCursorPos(double);

	int verticalSpacing() const;
	void setVerticalSpacing(int);
	int pixelsPerPeriod() const;
	double wavePhase() const;

	void setGatingEnabled(bool);
	void setLeftGateWidth(double);
	void setRightGateWidth(double);

	void setCursorVisible(bool visible);

Q_SIGNALS:
	void bufferStartDrag();
	void bufferMovedBy(int);
	void bufferStopDrag();
	void bufferResetPosition();

protected:
	void paintEvent(QPaintEvent *) override;
	void resizeEvent(QResizeEvent *) override;
	virtual void buildFullWaveform(QPointF *wavePoints, int numPts) = 0;

	void mouseDoubleClickEvent(QMouseEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void enterEvent(QEvent *event) override;
	void leaveEvent(QEvent *event) override;

private:
	double m_waveformPos;
	double m_waveformWidth;
	double m_highlightPos;
	double m_highlightWidth;
	double m_cursorPos;

	int m_verticalSpacing;
	int m_pixelsPerPeriod;
	double m_startingPhase;

	int m_fullWaveNumPoints;
	QPointF *m_fullWavePoints;

	QPoint m_offset;
	int m_pixelLeft;
	int m_pixelRight;
	bool m_rightBtnClick;

	bool m_gatingEnabled;
	double m_leftGateWidth;
	double m_rightGateWidth;

	bool m_cursorVisible;
};

class SCOPY_GUI_EXPORT AnalogBufferPreviewer : public BufferPreviewer
{
public:
	explicit AnalogBufferPreviewer(QWidget *parent = 0);
	explicit AnalogBufferPreviewer(int pixelsPerPeriod, double wavePhase, QWidget *parent = 0);

protected:
	virtual void buildFullWaveform(QPointF *wavePoints, int numPts);
};

class SCOPY_GUI_EXPORT DigitalBufferPreviewer : public BufferPreviewer
{
public:
	explicit DigitalBufferPreviewer(QWidget *parent = 0);
	explicit DigitalBufferPreviewer(int pixelsPerPeriod, QWidget *parent = 0);
	void setNoOfSteps(double val);
	double noOfSteps();

protected:
	virtual void buildFullWaveform(QPointF *wavePoints, int numPts);

private:
	double m_noOfSteps;
};

} // namespace scopy

#endif // BUFFER_PREVIEWER_H
