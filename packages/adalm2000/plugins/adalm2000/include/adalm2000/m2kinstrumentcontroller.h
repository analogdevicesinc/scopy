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

#include <core/acq_ui/acqinstrumentcontroller.h>

#include <pluginbase/resourcemanager.h>

#include <QString>

#include <functional>

class QLabel;
class QWidget;

namespace scopy {
class MenuControlButton;
class SemiExclusiveButtonGroup;
namespace component {
class Ping;
}
} // namespace scopy

namespace scopy::adalm2000 {
class M2kContext;

// What every ADALM2000 acquisition tool shares: claiming the hardware for a run, pausing
// the connection ping meanwhile, and refusing to run while the board calibrates.
class M2kInstrumentController : public scopy::adc::AcqInstrumentController, public scopy::ResourceUser
{
	Q_OBJECT
public:
	M2kInstrumentController(ToolMenuEntry *tme, M2kContext *ctx, const QString &resourceKey, const QString &title,
				QObject *parent = nullptr);
	~M2kInstrumentController() override;

	void stop() override;

protected:
	// Call once the run buttons exist.
	void setupCalibrationGate();
	virtual void updateRunEnabled();

	// A bottom-rail toggle showing `content` as a hover over the plot.
	void addHoverButton(const QString &label, QWidget *content);
	// A status line under the row's name, keeping the row narrow.
	static QLabel *addRowStatus(MenuControlButton *row);
	void restartIfRunning(const std::function<void()> &change);

	M2kContext *m_ctx{nullptr};
	bool m_calibrating{false};

private:
	bool claim();
	void release();
	scopy::component::Ping *ping() const;

	const QString m_resourceKey;
	const QString m_title;
	bool m_claimed{false};
	bool m_pingPaused{false};
	SemiExclusiveButtonGroup *m_hoverGroup{nullptr};
};
} // namespace scopy::adalm2000
