/*
 * Copyright (c) 2025 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "iiocpp/iioresult.h"

using namespace scopy;

IIO_RESULT_ERROR::IIO_RESULT_ERROR(char const *const message) throw()
	: std::runtime_error(message)
{
}

char const *IIO_RESULT_ERROR::what() const throw() { return std::runtime_error::what(); }

template <typename T>
IIOResult<T>::IIOResult(T ptr)
	: m_result(ptr)
{
}

template <typename T>
IIOResult<T>::IIOResult(int error_code)
	: m_result(error_code)
{
}

template <typename T>
bool IIOResult<T>::ok() const
{
	return std::holds_alternative<T>(m_result);
}

template <typename T>
T IIOResult<T>::data()
{
	if(!ok()) {
		throw IIO_RESULT_ERROR("IIOResult does not contain a valid pointer.");
	}

	return std::get<T>(m_result);
}

template <typename T>
int IIOResult<T>::error() const
{
	if(ok()) {
		throw IIO_RESULT_ERROR("IIOResult does not contain an error code.");
	}

	return std::get<int>(m_result);
}
