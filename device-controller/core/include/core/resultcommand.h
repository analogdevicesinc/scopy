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

#include "core/command.h"
#include "core/result.h"

#include <cerrno>

namespace scopy {

template <typename T>
class ResultCommand : public Command
{
public:
	using ResultType = Result<T>;
	using ResponseType = CommandResponse<T>;

	ResultCommand(void *resource, QObject *parent = nullptr)
		: Command(resource, parent)
	{}

	ResponseType result() const { return ResponseType(m_id, m_result); }

protected:
	void setResult(ResultType result) { m_result = std::move(result); }

	ResultType m_result{Unexpected{Error{-ECANCELED, "command cancelled"}}};
};

} // namespace scopy
