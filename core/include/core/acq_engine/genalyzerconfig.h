/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#include "scopy-core_export.h"

#include <cstdint>
#include <string>
#include <QMetaType>
#include <QString>
#include <QStringList>
#include <QVector>

namespace scopy {
namespace acq {

enum class GenalyzerMode
{
	AUTO = 0,
	FIXED_TONE = 1
};

struct SCOPY_CORE_EXPORT GenalyzerConfig
{
	bool enabled = false;

	GenalyzerMode mode = GenalyzerMode::AUTO;

	struct AutoParams
	{
		uint8_t ssb_width = 120;
	} auto_params;

	struct FixedToneParams
	{
		double expected_freq = 1000000.0;
		std::string component_label = "A";
		int harmonic_order = 3;
		int ssb_fundamental = 4;
		int ssb_default = 3;
		double fshift = 0.0;
	} fixed_tone;

	bool isAutoMode() const { return mode == GenalyzerMode::AUTO; }
	bool isFixedToneMode() const { return mode == GenalyzerMode::FIXED_TONE; }
};

// Thread-safe snapshot of genalyzer FFT analysis results, deep-copied so it
// can travel across queued signal/slot connections.
struct SCOPY_CORE_EXPORT GenalyzerResultsSnapshot
{
	QStringList keys;
	QVector<double> values;
};

} // namespace acq
} // namespace scopy

Q_DECLARE_METATYPE(scopy::acq::GenalyzerConfig)
Q_DECLARE_METATYPE(scopy::acq::GenalyzerResultsSnapshot)
