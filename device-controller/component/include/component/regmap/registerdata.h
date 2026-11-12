/*
 * Copyright (c) 2026 Analog Devices Inc.
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
 *
 */

#pragma once

#include "core/result.h"

#include <QDateTime>

#include <cstdint>
#include <optional>

namespace scopy::component {

enum class RegisterAccessStatus
{
	Pending, // a call is running
	Ok,
	Error
};

// One register access: the input of a read or write strategy and its result.
// address is always the template address (no address space bit).
struct RegisterData
{
	uint32_t address = 0;
	std::optional<uint32_t> value; // null on Error, and on Pending with no earlier value
	RegisterAccessStatus status = RegisterAccessStatus::Pending;
	std::optional<scopy::Error> error; // set only on Error
	QDateTime completedAt;		   // when the source answered, set by the strategy

	static RegisterData ok(uint32_t address, uint32_t value, const QDateTime &completedAt)
	{
		return RegisterData{address, value, RegisterAccessStatus::Ok, std::nullopt, completedAt};
	}
	static RegisterData failed(uint32_t address, const scopy::Error &error, const QDateTime &completedAt)
	{
		return RegisterData{address, std::nullopt, RegisterAccessStatus::Error, error, completedAt};
	}

	bool isOk() const { return status == RegisterAccessStatus::Ok; }
	bool isError() const { return status == RegisterAccessStatus::Error; }
};

} // namespace scopy::component
