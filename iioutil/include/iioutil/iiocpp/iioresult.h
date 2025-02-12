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

#ifndef IIORESULT_H
#define IIORESULT_H

#include "../scopy-iioutil_export.h"
#include <stdexcept>
#include <type_traits>
#include <variant>

#include <QObject>

#include <iio/iio.h>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT IIO_RESULT_ERROR : public std::runtime_error
{
public:
	IIO_RESULT_ERROR(char const *const message)
	throw()
		: std::runtime_error(message)
	{
	}

	virtual char const *what() const throw() override { return std::runtime_error::what(); }
};

template <typename T>
class SCOPY_IIOUTIL_EXPORT IIOResult
{
	// Ensure T is a pointer type
	static_assert(std::is_pointer_v<T>, "IIOResult must hold a pointer type");

public:
	explicit IIOResult() { m_result = nullptr; }
	explicit IIOResult(T ptr) { m_result = ptr; }
	explicit IIOResult(int error_code) { m_result = error_code; }

	/**
	 * @brief Check if the result is valid, should be used everytime before accessing the data.
	 * @return true if the result is valid, false otherwise
	 */
	bool ok() const { return std::holds_alternative<T>(m_result); }

	/**
	 * @brief Get the data from the result. You should not attempt to catch the exception,
	 * but rather use the ok() function first. The exception is there to signal a misuse of the class.
	 * @return The data if the result is valid.
	 * @throw IIO_RESULT_ERROR If the data() does not exist, representing a misuse of the class.
	 */
	T data() const
	{
		if(!ok()) {
			throw IIO_RESULT_ERROR("IIOResult does not contain a valid pointer.");
		}

		return std::get<T>(m_result);
	}

	/**
	 * @brief Get the error code from the result. You should not attempt to catch the exception,
	 * but rather use the ok() function first. The exception is there to signal a misuse of the class.
	 * @return The error code if the result is invalid.
	 * @throw IIO_RESULT_ERROR If the error() does not exist, representing a misuse of the class.
	 */
	int error() const
	{
		if(ok()) {
			throw IIO_RESULT_ERROR("IIOResult does not contain an error code.");
		}

		return std::get<int>(m_result);
	}

	/**
	 * @brief This function is similar to data(), but it should be used for a different case. It
	 * should be used when the result is most likely valid, such as when iterating over the list of
	 * attributes.
	 * @param message The message to be thrown if the result is invalid. The message should not say
	 * that "something bad happened", but rather what was expected to happen, e.g. ("Channel should
	 * have a name").
	 * @return The data if the result is valid.
	 * @throw IIO_RESULT_ERROR If the result is invalid.
	 */
	T expect(const QString &message) const
	{
		if(!ok()) {
			throw IIO_RESULT_ERROR(message.toStdString().c_str());
		}

		return std::get<T>(m_result);
	}

private:
	std::variant<T, int> m_result;
};
} // namespace scopy

#endif // IIORESULT_H
