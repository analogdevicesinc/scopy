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

#ifndef RANGEPRESETSCALER_H
#define RANGEPRESETSCALER_H

#include "scopy-gui_export.h"

#include <QVector>

namespace scopy {

struct ScalePreset
{
	double lower;
	double upper;
	int maxMajor;
	int maxMinor;
};

/**
 * @brief Picks a display range from a fixed preset table, widening on demand and
 * tightening when settled.
 */
class SCOPY_GUI_EXPORT RangePresetScaler
{
public:
	RangePresetScaler();

	// Presets must be in ascending order: the scans take the first one that fits.
	void setPresets(const QVector<ScalePreset> &presets);
	QVector<ScalePreset> presets() const;

	// Clamps every preset's lower bound to zero, for magnitude-only readings.
	void setFloorAtZero(bool floorAtZero);
	bool floorAtZero() const;

	int index() const;
	ScalePreset current() const;

	void resetWindow();

	// Only ever widens, returning the new index or -1. settle() is what lets the
	// range come back down.
	int push(double value);

	// Drops to the tightest preset still bracketing the window, then restarts it.
	int settle();

private:
	ScalePreset presetAt(int i) const;
	int select(int i);

	QVector<ScalePreset> m_presets;
	bool m_floorAtZero = false;
	int m_index = 0;
	double m_min = 0.0;
	double m_max = 0.0;
};

} // namespace scopy
#endif // RANGEPRESETSCALER_H
