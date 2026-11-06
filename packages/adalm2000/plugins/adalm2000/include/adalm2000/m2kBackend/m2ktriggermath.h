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

#include <QString>
#include <QStringList>

#include <cstddef>

namespace scopy::adalm2000::trigger {

inline constexpr const char *MODE_OPTIONS[] = {
	"ALWAYS",
	"ANALOG",
	"EXTERNAL",
	"DIGITAL_OR_ANALOG",
	"DIGITAL_AND_ANALOG",
	"DIGITAL_XOR_ANALOG",
	"N_DIGITAL_OR_ANALOG",
	"N_DIGITAL_AND_ANALOG",
	"N_DIGITAL_XOR_ANALOG",
};

inline constexpr const char *ANALOG_CONDITION_OPTIONS[] = {
	"RISING_EDGE_ANALOG",
	"FALLING_EDGE_ANALOG",
	"LOW_LEVEL_ANALOG",
	"HIGH_LEVEL_ANALOG",
};

inline constexpr const char *DIGITAL_CONDITION_OPTIONS[] = {
	"RISING_EDGE_DIGITAL", "FALLING_EDGE_DIGITAL", "LOW_LEVEL_DIGITAL",
	"HIGH_LEVEL_DIGITAL",  "ANY_EDGE_DIGITAL",     "NO_TRIGGER_DIGITAL",
};

inline constexpr int DIGITAL_CONDITION_NONE = 5;

inline constexpr const char *ANALOG_SOURCE_OPTIONS[] = {
	"CHANNEL_1",
	"CHANNEL_2",
	"CHANNEL_1_OR_CHANNEL_2",
	"CHANNEL_1_AND_CHANNEL_2",
	"CHANNEL_1_XOR_CHANNEL_2",
	"SRC_DIGITAL_IN",
	"CHANNEL_1_OR_SRC_LOGIC_ANALYZER",
	"CHANNEL_2_OR_SRC_LOGIC_ANALYZER",
	"CHANNEL_1_OR_CHANNEL_2_OR_SRC_LOGIC_ANALYZER",
	"NO_SOURCE",
};

inline constexpr const char *DIGITAL_SOURCE_OPTIONS[] = {
	"SRC_TRIGGER_IN",
	"SRC_ANALOG_IN",
	"SRC_NONE",
	"SRC_DISABLED",
};

inline constexpr const char *OUT_SELECT_OPTIONS[] = {
	"SELECT_NONE", "SELECT_TRIGGER_I_SAME_CHAN", "SELECT_TRIGGER_IN", "SELECT_ANALOG_IN", "SELECT_DIGITAL_IN",
};

inline constexpr const char *DIGITAL_MODE_OPTIONS[] = {
	"DIO_OR",
	"DIO_AND",
};

inline constexpr int DELAY_MIN = -8192;

template <size_t N>
constexpr int optionCount(const char *const (&)[N])
{
	return static_cast<int>(N);
}

template <size_t N>
QString toName(const char *const (&table)[N], int value)
{
	if(value < 0 || value >= static_cast<int>(N)) {
		return QString();
	}
	return QString::fromLatin1(table[value]);
}

template <size_t N>
int fromName(const char *const (&table)[N], const QString &name)
{
	for(size_t i = 0; i < N; ++i) {
		if(name == QLatin1String(table[i])) {
			return static_cast<int>(i);
		}
	}
	return -1;
}

template <size_t N>
QStringList toOptions(const char *const (&table)[N], int count = static_cast<int>(N))
{
	QStringList out;
	for(int i = 0; i < count && i < static_cast<int>(N); ++i) {
		out.append(QString::fromLatin1(table[i]));
	}
	return out;
}

} // namespace scopy::adalm2000::trigger
