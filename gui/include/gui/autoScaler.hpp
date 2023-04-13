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

#ifndef AUTO_SCALER_HPP
#define AUTO_SCALER_HPP

#include <QTimer>

#include <qwt_scale_div.h>
#include <qwt_thermo.h>
#include "scopygui_export.h"

namespace scopy {
	class SCOPYGUI_EXPORT AutoScaler : public QObject
	{
		Q_OBJECT;

	public:
		explicit AutoScaler(QObject *parent,
				const QVector<QwtScaleDiv> &divs,
				unsigned int timeout_ms = 3000);
		~AutoScaler();

	Q_SIGNALS:
		void updateScale(const QwtScaleDiv);

	public Q_SLOTS:
		void setValue(double val);
		void startTimer();
		void stopTimer();
		void setTimeout(int timeout_ms);

	private Q_SLOTS:
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
