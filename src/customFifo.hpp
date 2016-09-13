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

#ifndef CUSTOM_FIFO_HPP
#define CUSTOM_FIFO_HPP

#include <QQueue>
#include <QVector>

namespace adiscope {
	template <typename T>
	class CustomFifo
	{
	public:
		explicit CustomFifo();
		~CustomFifo();

		void push(T& data);
		T& pop();

		void reserve(int size);
		int size() const;
		void clear();

		T *data();

	private:
#if HAS_CONSTEXPR
		static constexpr bool useQQueue = sizeof(T) <= sizeof(void *);
#else
		static const bool useQQueue = false;
#endif

		QQueue<T> *queue;
		QVector<T> *vector;
	};
}

#endif
