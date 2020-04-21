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

#include "customFifo.hpp"

using namespace adiscope;

template <typename T>
CustomFifo<T>::CustomFifo() {
	if (useQQueue)
		queue = new QQueue<T>;
	else
		vector = new QVector<T>;
}

template <typename T>
CustomFifo<T>::~CustomFifo() {
	if (useQQueue)
		delete queue;
	else
		delete vector;
}

template <typename T>
void CustomFifo<T>::push(T &data) {
	if (useQQueue)
		queue->enqueue(data);
	else
		vector->append(data);
}

template <typename T>
T &CustomFifo<T>::pop() {
	if (useQQueue) {
		T &last = queue->last();
		queue->dequeue();
		return last;
	} else {
		T &last = vector->last();
		vector->removeFirst();
		return last;
	}
}

template <typename T>
void CustomFifo<T>::reserve(int size) {
	if (useQQueue)
		queue->reserve(size);
	else
		vector->reserve(size);
}

template <typename T>
int CustomFifo<T>::size() const {
	if (useQQueue)
		return queue->size();
	else
		return vector->size();
}

template <typename T>
void CustomFifo<T>::clear() {
	if (useQQueue)
		queue->clear();
	else
		vector->clear();
}

template <typename T>
T *CustomFifo<T>::data() {
	/* quick hack: we can deference queue.head(), because we know that
	 * QQueue (since it inherits QList) stores the data sequentially
	 * if the data elements are not bigger than the size of a pointer. */
	if (useQQueue)
		return &queue->head();
	else
		return vector->data();
}

namespace adiscope {
template class CustomFifo<double>;
}
