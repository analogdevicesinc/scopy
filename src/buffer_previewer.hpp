
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

#ifndef BUFFER_PREVIEWER_H
#define BUFFER_PREVIEWER_H

#include <QFrame>

namespace adiscope{

class BufferPreviewer: public QFrame
{
	Q_OBJECT

public:
	explicit BufferPreviewer(QWidget *parent = 0);
	explicit BufferPreviewer(int pixelsPerPeriod, double wavePhase,
		QWidget *parent = 0);
	virtual ~BufferPreviewer();

	double waveformPos() const;
	void setWaveformPos(double);
	double waveformWidth() const;
	void setWaveformWidth(double);
	double highlighPos() const;
	void setHighlightPos(double);
	double highlightWidth() const;
	void setHighlightWidth(double);
	double cursorPos() const;
	void setCursorPos(double);

	int verticalSpacing() const;
	void setVerticalSpacing(int);
	int pixelsPerPeriod() const;
	double wavePhase() const;

protected:
	void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;
	void resizeEvent(QResizeEvent *) Q_DECL_OVERRIDE;
	virtual void buildFullWaveform(QPointF *wavePoints, int numPts) = 0;

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
};

class AnalogBufferPreviewer: public BufferPreviewer
{
public:
	explicit AnalogBufferPreviewer(QWidget *parent = 0);
	explicit AnalogBufferPreviewer(int pixelsPerPeriod, double wavePhase,
		QWidget *parent = 0);

protected:
	virtual void buildFullWaveform(QPointF *wavePoints, int numPts);
};

class DigitalBufferPreviewer: public BufferPreviewer
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

} // namespace adiscope

#endif // BUFFER_PREVIEWER_H
