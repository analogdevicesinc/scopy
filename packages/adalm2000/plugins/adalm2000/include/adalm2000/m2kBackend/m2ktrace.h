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

#ifndef M2KTRACE_H
#define M2KTRACE_H

#include "core/calltrace.h"
#include "core/command.h"

#include <QList>
#include <QString>
#include <QVariant>

#include <cerrno>
#include <cstdlib>
#include <exception>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <libm2k/analog/enums.hpp>
#include <libm2k/digital/enums.hpp>
#include <libm2k/enums.hpp>
#include <libm2k/m2kexceptions.hpp>

// Call trace of the libm2k API for the command log and the code generator: like DC_TRACE (see
// core/calltrace.h) for a C++ library that throws. One CallRecord per call:
// - library "libm2k", function "<Class>::<method>" (M2k, M2kAnalogIn, M2kAnalogOut,
//   M2kPowerSupply, M2kDigital, M2kHardwareTrigger), handle = the object it is called on;
// - args in call order (M2K_ARG): integers / doubles as Int, bools, strings, enums as their symbol
//   (kind String, name "enum:<C++ type>"), vectors as their size (OutLen);
// - out = the returned value (handles for objects, symbols for enums, sizes for vectors);
// - a libm2k exception: rc -EIO (-ETIMEDOUT for a timeout), out = its message; then rethrown.
// Outside a recording command it is the call alone (one relaxed atomic load).
namespace scopy::adalm2000::m2ktrace {

inline constexpr auto LIBRARY = "libm2k";
inline constexpr auto ENUM_PREFIX = "enum:";

template <typename E>
struct EnumInfo;

template <>
struct EnumInfo<libm2k::CALIBRATION_MODE>
{
	static constexpr const char *type = "libm2k::CALIBRATION_MODE";
	static QString symbol(libm2k::CALIBRATION_MODE v)
	{
		switch(v) {
		case libm2k::ADC_REF1:
			return QStringLiteral("ADC_REF1");
		case libm2k::ADC_REF2:
			return QStringLiteral("ADC_REF2");
		case libm2k::ADC_GND:
			return QStringLiteral("ADC_GND");
		case libm2k::DAC:
			return QStringLiteral("DAC");
		case libm2k::NONE:
			return QStringLiteral("NONE");
		}
		return QString::number(int(v));
	}
};

template <>
struct EnumInfo<libm2k::GAIN_MODE>
{
	static constexpr const char *type = "libm2k::GAIN_MODE";
	static QString symbol(libm2k::GAIN_MODE v)
	{
		switch(v) {
		case libm2k::LOW_GAIN:
			return QStringLiteral("LOW_GAIN");
		case libm2k::HIGH_GAIN:
			return QStringLiteral("HIGH_GAIN");
		}
		return QString::number(int(v));
	}
};

template <>
struct EnumInfo<libm2k::M2K_TRIGGER_CONDITION_ANALOG>
{
	static constexpr const char *type = "libm2k::M2K_TRIGGER_CONDITION_ANALOG";
	static QString symbol(libm2k::M2K_TRIGGER_CONDITION_ANALOG v)
	{
		switch(v) {
		case libm2k::RISING_EDGE_ANALOG:
			return QStringLiteral("RISING_EDGE_ANALOG");
		case libm2k::FALLING_EDGE_ANALOG:
			return QStringLiteral("FALLING_EDGE_ANALOG");
		case libm2k::LOW_LEVEL_ANALOG:
			return QStringLiteral("LOW_LEVEL_ANALOG");
		case libm2k::HIGH_LEVEL_ANALOG:
			return QStringLiteral("HIGH_LEVEL_ANALOG");
		}
		return QString::number(int(v));
	}
};

template <>
struct EnumInfo<libm2k::M2K_TRIGGER_CONDITION_DIGITAL>
{
	static constexpr const char *type = "libm2k::M2K_TRIGGER_CONDITION_DIGITAL";
	static QString symbol(libm2k::M2K_TRIGGER_CONDITION_DIGITAL v)
	{
		switch(v) {
		case libm2k::RISING_EDGE_DIGITAL:
			return QStringLiteral("RISING_EDGE_DIGITAL");
		case libm2k::FALLING_EDGE_DIGITAL:
			return QStringLiteral("FALLING_EDGE_DIGITAL");
		case libm2k::LOW_LEVEL_DIGITAL:
			return QStringLiteral("LOW_LEVEL_DIGITAL");
		case libm2k::HIGH_LEVEL_DIGITAL:
			return QStringLiteral("HIGH_LEVEL_DIGITAL");
		case libm2k::ANY_EDGE_DIGITAL:
			return QStringLiteral("ANY_EDGE_DIGITAL");
		case libm2k::NO_TRIGGER_DIGITAL:
			return QStringLiteral("NO_TRIGGER_DIGITAL");
		}
		return QString::number(int(v));
	}
};

template <>
struct EnumInfo<libm2k::M2K_TRIGGER_MODE>
{
	static constexpr const char *type = "libm2k::M2K_TRIGGER_MODE";
	static QString symbol(libm2k::M2K_TRIGGER_MODE v)
	{
		switch(v) {
		case libm2k::ALWAYS:
			return QStringLiteral("ALWAYS");
		case libm2k::ANALOG:
			return QStringLiteral("ANALOG");
		case libm2k::EXTERNAL:
			return QStringLiteral("EXTERNAL");
		case libm2k::DIGITAL_OR_ANALOG:
			return QStringLiteral("DIGITAL_OR_ANALOG");
		case libm2k::DIGITAL_AND_ANALOG:
			return QStringLiteral("DIGITAL_AND_ANALOG");
		case libm2k::DIGITAL_XOR_ANALOG:
			return QStringLiteral("DIGITAL_XOR_ANALOG");
		case libm2k::N_DIGITAL_OR_ANALOG:
			return QStringLiteral("N_DIGITAL_OR_ANALOG");
		case libm2k::N_DIGITAL_AND_ANALOG:
			return QStringLiteral("N_DIGITAL_AND_ANALOG");
		case libm2k::N_DIGITAL_XOR_ANALOG:
			return QStringLiteral("N_DIGITAL_XOR_ANALOG");
		}
		return QString::number(int(v));
	}
};

template <>
struct EnumInfo<libm2k::M2K_TRIGGER_SOURCE_ANALOG>
{
	static constexpr const char *type = "libm2k::M2K_TRIGGER_SOURCE_ANALOG";
	static QString symbol(libm2k::M2K_TRIGGER_SOURCE_ANALOG v)
	{
		switch(v) {
		case libm2k::CHANNEL_1:
			return QStringLiteral("CHANNEL_1");
		case libm2k::CHANNEL_2:
			return QStringLiteral("CHANNEL_2");
		case libm2k::CHANNEL_1_OR_CHANNEL_2:
			return QStringLiteral("CHANNEL_1_OR_CHANNEL_2");
		case libm2k::CHANNEL_1_AND_CHANNEL_2:
			return QStringLiteral("CHANNEL_1_AND_CHANNEL_2");
		case libm2k::CHANNEL_1_XOR_CHANNEL_2:
			return QStringLiteral("CHANNEL_1_XOR_CHANNEL_2");
		case libm2k::SRC_DIGITAL_IN:
			return QStringLiteral("SRC_DIGITAL_IN");
		case libm2k::CHANNEL_1_OR_SRC_LOGIC_ANALYZER:
			return QStringLiteral("CHANNEL_1_OR_SRC_LOGIC_ANALYZER");
		case libm2k::CHANNEL_2_OR_SRC_LOGIC_ANALYZER:
			return QStringLiteral("CHANNEL_2_OR_SRC_LOGIC_ANALYZER");
		case libm2k::CHANNEL_1_OR_CHANNEL_2_OR_SRC_LOGIC_ANALYZER:
			return QStringLiteral("CHANNEL_1_OR_CHANNEL_2_OR_SRC_LOGIC_ANALYZER");
		case libm2k::NO_SOURCE:
			return QStringLiteral("NO_SOURCE");
		}
		return QString::number(int(v));
	}
};

template <>
struct EnumInfo<libm2k::M2K_TRIGGER_SOURCE_OUT>
{
	static constexpr const char *type = "libm2k::M2K_TRIGGER_SOURCE_OUT";
	static QString symbol(libm2k::M2K_TRIGGER_SOURCE_OUT v)
	{
		switch(v) {
		case libm2k::TRIGGER_NONE:
			return QStringLiteral("TRIGGER_NONE");
		case libm2k::TRIGGER_TI:
			return QStringLiteral("TRIGGER_TI");
		case libm2k::TRIGGER_ADC:
			return QStringLiteral("TRIGGER_ADC");
		case libm2k::TRIGGER_LA:
			return QStringLiteral("TRIGGER_LA");
		}
		return QString::number(int(v));
	}
};

template <>
struct EnumInfo<libm2k::M2K_TRIGGER_CONDITION_OUT>
{
	static constexpr const char *type = "libm2k::M2K_TRIGGER_CONDITION_OUT";
	static QString symbol(libm2k::M2K_TRIGGER_CONDITION_OUT v)
	{
		switch(v) {
		case libm2k::NONE_OUT:
			return QStringLiteral("NONE_OUT");
		case libm2k::LOW_LEVEL_OUT:
			return QStringLiteral("LOW_LEVEL_OUT");
		case libm2k::HIGH_LEVEL_OUT:
			return QStringLiteral("HIGH_LEVEL_OUT");
		case libm2k::ANY_EDGE_OUT:
			return QStringLiteral("ANY_EDGE_OUT");
		case libm2k::RISING_EDGE_OUT:
			return QStringLiteral("RISING_EDGE_OUT");
		case libm2k::FALLING_EDGE_OUT:
			return QStringLiteral("FALLING_EDGE_OUT");
		}
		return QString::number(int(v));
	}
};

template <>
struct EnumInfo<libm2k::M2K_TRIGGER_STATUS_ANALOG_OUT>
{
	static constexpr const char *type = "libm2k::M2K_TRIGGER_STATUS_ANALOG_OUT";
	static QString symbol(libm2k::M2K_TRIGGER_STATUS_ANALOG_OUT v)
	{
		switch(v) {
		case libm2k::DISABLED:
			return QStringLiteral("DISABLED");
		case libm2k::START:
			return QStringLiteral("START");
		case libm2k::STOP:
			return QStringLiteral("STOP");
		}
		return QString::number(int(v));
	}
};

template <>
struct EnumInfo<libm2k::M2K_TRIGGER_SOURCE_DIGITAL>
{
	static constexpr const char *type = "libm2k::M2K_TRIGGER_SOURCE_DIGITAL";
	static QString symbol(libm2k::M2K_TRIGGER_SOURCE_DIGITAL v)
	{
		switch(v) {
		case libm2k::SRC_TRIGGER_IN:
			return QStringLiteral("SRC_TRIGGER_IN");
		case libm2k::SRC_ANALOG_IN:
			return QStringLiteral("SRC_ANALOG_IN");
		case libm2k::SRC_NONE:
			return QStringLiteral("SRC_NONE");
		case libm2k::SRC_DISABLED:
			return QStringLiteral("SRC_DISABLED");
		}
		return QString::number(int(v));
	}
};

template <>
struct EnumInfo<libm2k::M2K_TRIGGER_OUT_SELECT>
{
	static constexpr const char *type = "libm2k::M2K_TRIGGER_OUT_SELECT";
	static QString symbol(libm2k::M2K_TRIGGER_OUT_SELECT v)
	{
		switch(v) {
		case libm2k::SELECT_NONE:
			return QStringLiteral("SELECT_NONE");
		case libm2k::SELECT_TRIGGER_I_SAME_CHAN:
			return QStringLiteral("SELECT_TRIGGER_I_SAME_CHAN");
		case libm2k::SELECT_TRIGGER_IN:
			return QStringLiteral("SELECT_TRIGGER_IN");
		case libm2k::SELECT_ANALOG_IN:
			return QStringLiteral("SELECT_ANALOG_IN");
		case libm2k::SELECT_DIGITAL_IN:
			return QStringLiteral("SELECT_DIGITAL_IN");
		}
		return QString::number(int(v));
	}
};

template <>
struct EnumInfo<libm2k::analog::ANALOG_IN_CHANNEL>
{
	static constexpr const char *type = "libm2k::analog::ANALOG_IN_CHANNEL";
	static QString symbol(libm2k::analog::ANALOG_IN_CHANNEL v)
	{
		switch(v) {
		case libm2k::analog::ANALOG_IN_CHANNEL_1:
			return QStringLiteral("ANALOG_IN_CHANNEL_1");
		case libm2k::analog::ANALOG_IN_CHANNEL_2:
			return QStringLiteral("ANALOG_IN_CHANNEL_2");
		}
		return QString::number(int(v));
	}
};

template <>
struct EnumInfo<libm2k::analog::M2K_RANGE>
{
	static constexpr const char *type = "libm2k::analog::M2K_RANGE";
	static QString symbol(libm2k::analog::M2K_RANGE v)
	{
		switch(v) {
		case libm2k::analog::PLUS_MINUS_25V:
			return QStringLiteral("PLUS_MINUS_25V");
		case libm2k::analog::PLUS_MINUS_2_5V:
			return QStringLiteral("PLUS_MINUS_2_5V");
		}
		return QString::number(int(v));
	}
};

template <>
struct EnumInfo<libm2k::digital::DIO_DIRECTION>
{
	static constexpr const char *type = "libm2k::digital::DIO_DIRECTION";
	static QString symbol(libm2k::digital::DIO_DIRECTION v)
	{
		switch(v) {
		case libm2k::digital::DIO_INPUT:
			return QStringLiteral("DIO_INPUT");
		case libm2k::digital::DIO_OUTPUT:
			return QStringLiteral("DIO_OUTPUT");
		}
		return QString::number(int(v));
	}
};

template <>
struct EnumInfo<libm2k::digital::DIO_LEVEL>
{
	static constexpr const char *type = "libm2k::digital::DIO_LEVEL";
	static QString symbol(libm2k::digital::DIO_LEVEL v)
	{
		switch(v) {
		case libm2k::digital::LOW:
			return QStringLiteral("LOW");
		case libm2k::digital::HIGH:
			return QStringLiteral("HIGH");
		}
		return QString::number(int(v));
	}
};

template <>
struct EnumInfo<libm2k::digital::DIO_CHANNEL>
{
	static constexpr const char *type = "libm2k::digital::DIO_CHANNEL";
	static QString symbol(libm2k::digital::DIO_CHANNEL v)
	{
		switch(v) {
		case libm2k::digital::DIO_CHANNEL_0:
			return QStringLiteral("DIO_CHANNEL_0");
		case libm2k::digital::DIO_CHANNEL_1:
			return QStringLiteral("DIO_CHANNEL_1");
		case libm2k::digital::DIO_CHANNEL_2:
			return QStringLiteral("DIO_CHANNEL_2");
		case libm2k::digital::DIO_CHANNEL_3:
			return QStringLiteral("DIO_CHANNEL_3");
		case libm2k::digital::DIO_CHANNEL_4:
			return QStringLiteral("DIO_CHANNEL_4");
		case libm2k::digital::DIO_CHANNEL_5:
			return QStringLiteral("DIO_CHANNEL_5");
		case libm2k::digital::DIO_CHANNEL_6:
			return QStringLiteral("DIO_CHANNEL_6");
		case libm2k::digital::DIO_CHANNEL_7:
			return QStringLiteral("DIO_CHANNEL_7");
		case libm2k::digital::DIO_CHANNEL_8:
			return QStringLiteral("DIO_CHANNEL_8");
		case libm2k::digital::DIO_CHANNEL_9:
			return QStringLiteral("DIO_CHANNEL_9");
		case libm2k::digital::DIO_CHANNEL_10:
			return QStringLiteral("DIO_CHANNEL_10");
		case libm2k::digital::DIO_CHANNEL_11:
			return QStringLiteral("DIO_CHANNEL_11");
		case libm2k::digital::DIO_CHANNEL_12:
			return QStringLiteral("DIO_CHANNEL_12");
		case libm2k::digital::DIO_CHANNEL_13:
			return QStringLiteral("DIO_CHANNEL_13");
		case libm2k::digital::DIO_CHANNEL_14:
			return QStringLiteral("DIO_CHANNEL_14");
		case libm2k::digital::DIO_CHANNEL_15:
			return QStringLiteral("DIO_CHANNEL_15");
		}
		return QString::number(int(v));
	}
};

template <>
struct EnumInfo<libm2k::digital::DIO_MODE>
{
	static constexpr const char *type = "libm2k::digital::DIO_MODE";
	static QString symbol(libm2k::digital::DIO_MODE v)
	{
		switch(v) {
		case libm2k::digital::DIO_OPENDRAIN:
			return QStringLiteral("DIO_OPENDRAIN");
		case libm2k::digital::DIO_PUSHPULL:
			return QStringLiteral("DIO_PUSHPULL");
		}
		return QString::number(int(v));
	}
};

template <>
struct EnumInfo<libm2k::digital::DIO_TRIGGER_MODE>
{
	static constexpr const char *type = "libm2k::digital::DIO_TRIGGER_MODE";
	static QString symbol(libm2k::digital::DIO_TRIGGER_MODE v)
	{
		switch(v) {
		case libm2k::digital::DIO_OR:
			return QStringLiteral("DIO_OR");
		case libm2k::digital::DIO_AND:
			return QStringLiteral("DIO_AND");
		}
		return QString::number(int(v));
	}
};

template <typename T>
inline constexpr bool isVector = false;
template <typename U, typename A>
inline constexpr bool isVector<std::vector<U, A>> = true;

template <typename T>
CallArg arg(const char *name, const T &v)
{
	using D = std::decay_t<T>;
	if constexpr(std::is_enum_v<D>) {
		return {QString::fromLatin1(ENUM_PREFIX) + QString::fromLatin1(EnumInfo<D>::type),
			EnumInfo<D>::symbol(v), CallArg::String};
	} else if constexpr(std::is_same_v<D, bool>) {
		return CallArg::boolean(name, v);
	} else if constexpr(std::is_integral_v<D>) {
		return CallArg::integer(name, qint64(v));
	} else if constexpr(std::is_floating_point_v<D>) {
		return {QString::fromLatin1(name), double(v), CallArg::Int};
	} else if constexpr(std::is_same_v<D, std::string>) {
		return CallArg::string(name, QString::fromStdString(v));
	} else if constexpr(std::is_same_v<D, QString>) {
		return CallArg::string(name, v);
	} else if constexpr(std::is_convertible_v<D, const char *>) {
		return CallArg::string(name, QString::fromUtf8(v));
	} else if constexpr(isVector<D>) {
		return CallArg::outLen(name, qint64(v.size()));
	} else {
		static_assert(sizeof(D) == 0, "m2ktrace::arg: unsupported argument type");
	}
}

template <typename T>
void setOut(CallRecord &rec, const T &v)
{
	using D = std::decay_t<T>;
	if constexpr(std::is_pointer_v<D>) {
		rec.out = QVariant::fromValue(quintptr(v));
		rec.outKind = CallArg::Handle;
	} else if constexpr(std::is_enum_v<D>) {
		rec.out = EnumInfo<D>::symbol(v);
		rec.outKind = CallArg::String;
	} else if constexpr(std::is_same_v<D, bool>) {
		rec.out = v;
		rec.outKind = CallArg::Bool;
	} else if constexpr(std::is_arithmetic_v<D>) {
		rec.out = QVariant::fromValue(v);
		rec.outKind = CallArg::Int;
	} else if constexpr(std::is_same_v<D, std::string>) {
		rec.out = QString::fromStdString(v);
		rec.outKind = CallArg::String;
	} else if constexpr(isVector<D>) {
		rec.out = qint64(v.size());
		rec.outKind = CallArg::OutLen;
	} else if constexpr(std::is_same_v<D, std::pair<double, double>>) {
		rec.out = QStringLiteral("%1, %2").arg(v.first, 0, 'g', 12).arg(v.second, 0, 'g', 12);
		rec.outKind = CallArg::String;
	}
	// Other types: no value in the log.
}

// Body of M2K_CALL; use the macro.
template <typename Call, typename Args>
auto traced(const void *object, const char *function, Call &&call, Args &&args) -> decltype(call())
{
	Command *cmd = Command::current();
	if(!cmd || !cmd->isRecording()) {
		return call();
	}
	CallRecord rec;
	rec.library = QString::fromLatin1(LIBRARY);
	rec.function = QString::fromLatin1(function);
	rec.handle = quintptr(object);
	rec.args = args();
	rec.startNs = CallTrace::nowNs();
	const qint64 t0 = CallTrace::steadyNs();
	try {
		if constexpr(std::is_void_v<decltype(call())>) {
			call();
			rec.durationNs = CallTrace::steadyNs() - t0;
			cmd->trace(std::move(rec));
		} else {
			auto ret = call();
			rec.durationNs = CallTrace::steadyNs() - t0;
			setOut(rec, ret);
			cmd->trace(std::move(rec));
			return ret;
		}
	} catch(const libm2k::m2k_exception &e) {
		const bool timeout = e.type() == libm2k::EXC_TIMEOUT || std::abs(e.iioCode()) == ETIMEDOUT;
		rec.durationNs = CallTrace::steadyNs() - t0;
		rec.rc = timeout ? -ETIMEDOUT : -EIO;
		rec.out = QString::fromUtf8(e.what());
		rec.outKind = CallArg::String;
		cmd->trace(std::move(rec));
		throw;
	} catch(const std::exception &e) {
		rec.durationNs = CallTrace::steadyNs() - t0;
		rec.rc = -EIO;
		rec.out = QString::fromUtf8(e.what());
		rec.outKind = CallArg::String;
		cmd->trace(std::move(rec));
		throw;
	}
}

} // namespace scopy::adalm2000::m2ktrace

// M2K_CALL(object, "M2kAnalogIn::setRange", ain->setRange(ch, range), M2K_ARG(ch), M2K_ARG(range))
//   Runs the libm2k call `expr` on `object`; inside a recording command it also records it.
#define M2K_CALL(object, function, expr, ...)                                                                          \
	(Q_LIKELY(!::scopy::CallTrace::active())                                                                       \
		 ? (expr)                                                                                              \
		 : ::scopy::adalm2000::m2ktrace::traced((object), (function), [&]() { return expr; },                  \
							[&]() { return QList<::scopy::CallArg>{__VA_ARGS__}; }))

// An argument named after the expression: M2K_ARG(ch) -> "ch=0".
#define M2K_ARG(x) ::scopy::adalm2000::m2ktrace::arg(#x, (x))
// An argument with an explicit name: M2K_ARGN("chn", e.channel).
#define M2K_ARGN(name, x) ::scopy::adalm2000::m2ktrace::arg((name), (x))

#endif // M2KTRACE_H
