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

#include <QMetaType>
#include <QString>

#include <cstring>
#include <quuid.h>
#include <utility>
#include <variant>

namespace scopy {

// std::expected<T,E> analog (C++20 stand-in; swap for std::expected at C++23).
// Success carries a T, failure an E (default: the Error struct below). Errors are
// built through the Unexpected<E> wrapper (the std::unexpected analog):
template <typename E>
struct Unexpected
{
	E error;
};
template <typename E>
Unexpected(E) -> Unexpected<E>; // CTAD

#if !defined(_WIN32)
// Overloads that normalize strerror_r's two return conventions to the filled buffer.
inline const char *strerrorMessage(int /*xsiRet*/, char *buf) { return buf; }	    // XSI: int
inline const char *strerrorMessage(char *gnuRet, char * /*buf*/) { return gnuRet; } // GNU: char*
#endif

struct Error
{
	int code = 0;
	QString message;

	[[nodiscard]] int errorCode() const { return code < 0 ? -code : 0; }

	[[nodiscard]] QString errorString() const
	{
		if(!message.isEmpty()) {
			return message;
		}
		if(code >= 0) {
			return {};
		}
		char buf[128];
		buf[0] = '\0';
#if defined(_WIN32)
		strerror_s(buf, sizeof(buf), errorCode());
		const char *msg = buf;
#else
		// strerror_r has two incompatible signatures: XSI (returns int, fills buf — macOS/POSIX)
		// and GNU (returns char*, may leave buf untouched). Pick the right one by return type.
		const char *msg = strerrorMessage(strerror_r(errorCode(), buf, sizeof(buf)), buf);
#endif
		return QString::fromUtf8(msg) + QStringLiteral(" (%1)").arg(errorCode());
	}
};

template <typename T, typename E = Error>
class Result
{
public:
	Result(const T &value)
		: m_data(std::in_place_index<0>, value)
	{}
	Result(T &&value)
		: m_data(std::in_place_index<0>, std::move(value))
	{}
	Result(const Unexpected<E> &e)
		: m_data(std::in_place_index<1>, e)
	{}
	Result(Unexpected<E> &&e)
		: m_data(std::in_place_index<1>, std::move(e))
	{}

	[[nodiscard]] bool hasValue() const { return m_data.index() == 0; }
	[[nodiscard]] explicit operator bool() const { return hasValue(); }

	[[nodiscard]] T &value() { return std::get<0>(m_data); }
	[[nodiscard]] const T &value() const { return std::get<0>(m_data); }

	[[nodiscard]] E &error() { return std::get<1>(m_data).error; }
	[[nodiscard]] const E &error() const { return std::get<1>(m_data).error; }

	template <class U>
	[[nodiscard]] T valueOr(U &&fallback) const
	{
		return hasValue() ? value() : static_cast<T>(std::forward<U>(fallback));
	}

private:
	std::variant<T, Unexpected<E>> m_data; // tagged: T==E stays legal
};

// void payload — success carries nothing (writes, lifecycle, refill/push,
// triggers, calibrate). Mirrors std::expected<void, E>.
template <typename E>
class Result<void, E>
{
public:
	Result() = default;
	Result(const Unexpected<E> &e)
		: m_error(e.error)
		, m_hasValue(false)
	{}
	Result(Unexpected<E> &&e)
		: m_error(std::move(e.error))
		, m_hasValue(false)
	{}

	[[nodiscard]] bool hasValue() const { return m_hasValue; }
	[[nodiscard]] explicit operator bool() const { return m_hasValue; }

	[[nodiscard]] E &error() { return m_error; }
	[[nodiscard]] const E &error() const { return m_error; }

private:
	E m_error{};
	bool m_hasValue = true;
};

// Result variant that also carries the id of the command it answers.
template <typename T, typename E = Error>
class CommandResponse : public Result<T, E>
{
public:
	CommandResponse(QUuid commandId, const T &value)
		: Result<T, E>(value)
		, m_commandId(commandId)
	{}
	CommandResponse(QUuid commandId, T &&value)
		: Result<T, E>(std::move(value))
		, m_commandId(commandId)
	{}
	CommandResponse(QUuid commandId, const Unexpected<E> &e)
		: Result<T, E>(e)
		, m_commandId(commandId)
	{}
	CommandResponse(QUuid commandId, Unexpected<E> &&e)
		: Result<T, E>(std::move(e))
		, m_commandId(commandId)
	{}
	CommandResponse(QUuid commandId, const Result<T, E> &r)
		: Result<T, E>(r)
		, m_commandId(commandId)
	{}
	CommandResponse(QUuid commandId, Result<T, E> &&r)
		: Result<T, E>(std::move(r))
		, m_commandId(commandId)
	{}

	[[nodiscard]] QUuid commandId() const { return m_commandId; }

private:
	QUuid m_commandId;
};

// void payload specialization.
template <typename E>
class CommandResponse<void, E> : public Result<void, E>
{
public:
	explicit CommandResponse(QUuid commandId)
		: Result<void, E>()
		, m_commandId(commandId)
	{}
	CommandResponse(QUuid commandId, const Unexpected<E> &e)
		: Result<void, E>(e)
		, m_commandId(commandId)
	{}
	CommandResponse(QUuid commandId, Unexpected<E> &&e)
		: Result<void, E>(std::move(e))
		, m_commandId(commandId)
	{}
	CommandResponse(QUuid commandId, const Result<void, E> &r)
		: Result<void, E>(r)
		, m_commandId(commandId)
	{}

	[[nodiscard]] QUuid commandId() const { return m_commandId; }

private:
	QUuid m_commandId;
};

} // namespace scopy

Q_DECLARE_METATYPE(scopy::Error)
