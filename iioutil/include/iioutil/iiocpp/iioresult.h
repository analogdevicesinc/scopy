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

#include "scopy-iioutil_export.h"
#include <stdexcept>
#include <type_traits>
#include <variant>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT IIO_RESULT_ERROR : public std::runtime_error
{
public:
	IIO_RESULT_ERROR(char const *const message) throw();
	virtual char const *what() const throw() override;
};

template <typename T>
class SCOPY_IIOUTIL_EXPORT IIOResult
{
	// Ensure T is a pointer type
	static_assert(std::is_pointer_v<T>, "IIOResult must hold a pointer type");

public:
	explicit IIOResult(T ptr);
	explicit IIOResult(int error_code);

	/**
	 * @brief Check if the result is valid, should be used everytime before accessing the data.
	 * @return true if the result is valid, false otherwise
	 */
	bool ok() const;

	/**
	 * @brief Get the data from the result. You should not attempt to catch the exception,
	 * but rather use the ok() function first. The exception is there to signal a misuse of the class.
	 * @return The data if the result is valid.
	 * @throw IIO_RESULT_ERROR If the data() does not exist, representing a misuse of the class.
	 */
	T data();

	/**
	 * @brief Get the error code from the result. You should not attempt to catch the exception,
	 * but rather use the ok() function first. The exception is there to signal a misuse of the class.
	 * @return The error code if the result is invalid.
	 * @throw IIO_RESULT_ERROR If the error() does not exist, representing a misuse of the class.
	 */
	int error() const;

private:
	std::variant<T, int> m_result;
};
} // namespace scopy

#endif // IIORESULT_H
