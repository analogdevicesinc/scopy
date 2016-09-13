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

#ifndef AUTO_SCALER_HPP
#define AUTO_SCALER_HPP

#include <QTimer>

#include <qwt_scale_div.h>
#include <qwt_thermo.h>

namespace adiscope {
	class AutoScaler : public QObject
	{
		Q_OBJECT;

	public:
		explicit AutoScaler(QObject *parent,
				const QVector<QwtScaleDiv> &divs,
				unsigned int timeout_ms = 3000);
		~AutoScaler();

	signals:
		void updateScale(const QwtScaleDiv);

	public slots:
		void setValue(double val);
		void startTimer();
		void stopTimer();
		void setTimeout(int timeout_ms);

	private slots:
		void timeout();

	private:
		QVector<QwtScaleDiv> divs;
		QTimer timer;
		double min, max, scale_min, scale_max;
		const QwtScaleDiv *current_div;

		void changeScaleDiv(const QwtScaleDiv *div);
	};
};

#endif /* AUTO_SCALER_HPP */
