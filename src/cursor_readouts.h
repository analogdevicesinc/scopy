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

#ifndef CURSOR_READOUTS_H
#define CURSOR_READOUTS_H

#include <QWidget>

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

		virtual bool eventFilter(QObject *, QEvent *);

	private:
		QPoint plotPointToPixelPoint(const QPointF &point) const;
		void updateSizeAndPosition();

	private:
		Ui::CursorReadouts *ui;
		bool d_voltage_rd_visible;
		bool d_time_rd_visible;
		QPoint d_topLeft;
	};
}


#endif // CURSOR_READOUTS_H
