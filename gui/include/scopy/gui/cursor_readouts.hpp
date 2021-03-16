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

#ifndef CURSOR_READOUTS_H
#define CURSOR_READOUTS_H

#include "custom_animation.hpp"

#include <QShowEvent>
#include <QWidget>
#include <qwt_axis_id.h>

#include <scopy/gui/custom_plot_position_button.hpp>

class QwtPlot;

namespace Ui {
class CursorReadouts;
}

namespace scopy {
namespace gui {

class CursorReadouts : public QWidget
{
public:
	CursorReadouts(QwtPlot* plot);
	~CursorReadouts();

	const QwtPlot* plot() const;
	QwtPlot* plot();

	void setVoltageReadoutVisible(bool on);
	bool isVoltageReadoutVisible() const;

	void setTimeReadoutVisible(bool on);
	bool isTimeReadoutVisible() const;

	void setTopLeftStartingPoint(QPoint point);
	QPoint topLeftStartingPoint() const;

	void setTimeCursor1Text(const QString&);
	QString timeCursor1Text() const;

	void setTimeCursor2Text(const QString&);
	QString timeCursor2Text() const;

	void setTimeDeltaText(const QString&);
	QString timeDeltaText() const;

	void setFreqDeltaText(const QString&);
	QString freqDeltaText() const;

	void setVoltageCursor1Text(const QString&);
	QString voltageCursor1Text() const;

	void setVoltageCursor2Text(const QString&);
	QString voltageCursor2Text() const;

	void setVoltageDeltaText(const QString&);
	QString voltageDeltaText() const;

	void setTimeDeltaVisible(bool);
	void setFrequencyDeltaVisible(bool);

	void setTimeCursor1LabelText(const QString&);
	QString timeCursor1LabelText() const;

	void setTimeCursor2LabelText(const QString&);
	QString timeCursor2LabelText() const;

	void setVoltageCursor1LabelText(const QString&);
	QString voltageCursor1LabelText() const;

	void setVoltageCursor2LabelText(const QString&);
	QString voltageCursor2LabelText() const;

	void setDeltaVoltageLabelText(const QString&);
	QString deltaVoltageLabelText() const;

	void setAxis(QwtAxisId m_hAxis, QwtAxisId m_vAxis);

	virtual bool eventFilter(QObject*, QEvent*);

	void setTransparency(int value);
	void moveToPosition(CustomPlotPositionButton::ReadoutsPosition position);

	CustomPlotPositionButton::ReadoutsPosition getCurrentPosition();

	void showEvent(QShowEvent* event);

private:
	QPoint plotPointToPixelPoint(const QPointF& point) const;
	void updateSizeAndPosition(bool resize = false);

private:
	Ui::CursorReadouts* m_ui;
	bool m_DVoltageRdVisible;
	bool m_DTimeRdVisible;

	QPoint m_topLeft;

	void moveTopLeft(bool resize = false);
	void moveTopRight(bool resize = false);
	void moveBottomLeft(bool resize = false);
	void moveBottomRight(bool resize = false);

	CustomPlotPositionButton::ReadoutsPosition m_currentPosition;

	CustomAnimation *m_anim, *m_anim2;
	QRect m_lastTimeRect, m_lastVoltageRect;
	QwtAxisId m_hAxis, m_vAxis;
};
} // namespace gui
} // namespace scopy

#endif // CURSOR_READOUTS_H
