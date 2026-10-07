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
 */

#pragma once

#include "core/resultcommand.h"

#include <QString>

#include <cerrno>
#include <exception>
#include <functional>
#include <type_traits>
#include <utility>

namespace scopy::adalm2000 {

template <typename T>
class M2kCallCommand : public scopy::ResultCommand<T>
{
public:
	using Fn = std::function<T()>;

	M2kCallCommand(void *resource, Fn fn, QObject *parent = nullptr)
		: scopy::ResultCommand<T>(resource, parent)
		, m_fn(std::move(fn))
	{}

protected:
	void run() override
	{
		if(!m_fn) {
			this->setResult(scopy::Unexpected{scopy::Error{-EINVAL, QStringLiteral("no callable")}});
			return;
		}
		try {
			if constexpr(std::is_void_v<T>) {
				m_fn();
				this->setResult(scopy::Result<void>{});
			} else {
				this->setResult(scopy::Result<T>{m_fn()});
			}
		} catch(const std::exception &e) {
			this->setResult(scopy::Unexpected{scopy::Error{
				-EIO, QStringLiteral("libm2k call failed: ") + QString::fromUtf8(e.what())}});
		} catch(...) {
			this->setResult(scopy::Unexpected{
				scopy::Error{-EIO, QStringLiteral("libm2k call failed: unknown exception")}});
		}
	}

private:
	Fn m_fn;
};

} // namespace scopy::adalm2000
