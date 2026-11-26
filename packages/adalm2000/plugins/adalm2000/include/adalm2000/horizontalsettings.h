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

#include "sweepmath.h"

#include <QWidget>

class QTabBar;

namespace scopy::gui {
class MenuSpinbox;
}

namespace scopy::adalm2000 {

// The "Time base | Samples" tabs: two views of one sweep. Edits come out as a requested
// sweep; the tool applies it and shows back what it took with setSweep().
class HorizontalSettings : public QWidget
{
	Q_OBJECT
public:
	enum Tab
	{
		TimeBase,
		Samples,
	};

	HorizontalSettings(const sweep::Limits &limits, int divs, Tab tab, QWidget *parent = nullptr);

	void setSweep(const sweep::Sweep &s);
	void setDelayEnabled(bool on);
	// The samples the time base asks for; changing it requests the sweep again.
	void setTimeBaseSamples(int n);
	int timeBaseSamples() const { return m_limits.timeBaseSamples; }

Q_SIGNALS:
	void sweepRequested(const scopy::adalm2000::sweep::Sweep &s);
	void timeBaseSamplesChanged(int n);

private:
	void request(const sweep::Sweep &s);
	void applyTimeBaseRange();

	sweep::Limits m_limits;
	const int m_divs;
	sweep::Sweep m_sweep;

	QTabBar *m_tabs{nullptr};
	gui::MenuSpinbox *m_timeBase{nullptr};
	gui::MenuSpinbox *m_position{nullptr};
	gui::MenuSpinbox *m_points{nullptr};
	gui::MenuSpinbox *m_rate{nullptr};
	gui::MenuSpinbox *m_buffer{nullptr};
	gui::MenuSpinbox *m_window{nullptr};
	gui::MenuSpinbox *m_delay{nullptr};
};

} // namespace scopy::adalm2000
