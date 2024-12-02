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

#ifndef CUSTOM_SCALE_HPP
#define CUSTOM_SCALE_HPP

#include <qwt_thermo.h>

#include "autoScaler.hpp"

namespace adiscope {
	class CustomScale : public QwtThermo
	{
		Q_OBJECT

		public:
			explicit CustomScale(QWidget *parent = nullptr);
			~CustomScale();

		public Q_SLOTS:
			void setValue(double);
			void start();
			void stop();

		private Q_SLOTS:
			void updateScale(const QwtScaleDiv);

		private:
			AutoScaler *scaler;
	};
}

#endif /* CUSTOM_SCALE_HPP */
