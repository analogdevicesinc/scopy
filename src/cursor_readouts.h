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

#include <QWidget>
#include "customanimation.h"
#include <QShowEvent>
#include <qwt_axis_id.h>
#include "customplotpositionbutton.h"

class QwtPlot;

namespace Ui {
	class CursorReadouts;
}

namespace adiscope {
	class CursorReadouts: public QWidget
	{
	public:
		CursorReadouts(QwtPlot *plot);
		~CursorReadouts();

		const QwtPlot *plot() const;
		QwtPlot *plot();

		void setVoltageReadoutVisible(bool on);
		bool isVoltageReadoutVisible();

		void setTimeReadoutVisible(bool on);
		bool isTimeReadoutVisible();

		void setTopLeftStartingPoint(QPoint point);
		QPoint topLeftStartingPoint();

		void setTimeCursor1Text(const QString &);
		QString timeCursor1Text();
		void setTimeCursor2Text(const QString &);
		QString timeCursor2Text();
		void setTimeDeltaText(const QString &);
		QString timeDeltaText();
		void setFreqDeltaText(const QString &);
		QString freqDeltaText();
		void setVoltageCursor1Text(const QString &);
		QString voltageCursor1Text();
		void setVoltageCursor2Text(const QString &);
		QString voltageCursor2Text();
		void setVoltageDeltaText(const QString &);
		QString voltageDeltaText();
		void setTimeDeltaVisible(bool);
		void setFrequencyDeltaVisible(bool);
		void setTimeCursor1LabelText(const QString &);
		QString timeCursor1LabelText();
		void setTimeCursor2LabelText(const QString &);
		QString timeCursor2LabelText();
		void setTimeDeltaLabelText(const QString &text);
		void setVoltageCursor1LabelText(const QString &);
		QString voltageCursor1LabelText();
		void setVoltageCursor2LabelText(const QString &);
		QString voltageCursor2LabelText();
		void setDeltaVoltageLabelText(const QString &);
		QString deltaVoltageLabelText();
		void setAxis(QwtAxisId hAxis,QwtAxisId vAxis);
		virtual bool eventFilter(QObject *, QEvent *);

		void setTransparency(int value);
		void moveToPosition(CustomPlotPositionButton::ReadoutsPosition position);
		CustomPlotPositionButton::ReadoutsPosition getCurrentPosition();

		void showEvent(QShowEvent *event);
	private:
		QPoint plotPointToPixelPoint(const QPointF &point) const;
		void updateSizeAndPosition(bool resize = false);

	private:
		Ui::CursorReadouts *ui;
		bool d_voltage_rd_visible;
		bool d_time_rd_visible;
		bool freq_delta_visible;
		QPoint d_topLeft;
		void moveTopLeft(bool resize = false);
		void moveTopRight(bool resize = false);
		void moveBottomLeft(bool resize = false);
		void moveBottomRight(bool resize = false);
		CustomPlotPositionButton::ReadoutsPosition currentPosition;
		CustomAnimation *anim, *anim2;
		QRect lastTimeRect, lastVoltageRect;
		QwtAxisId hAxis,vAxis;
	};
}


#endif // CURSOR_READOUTS_H
