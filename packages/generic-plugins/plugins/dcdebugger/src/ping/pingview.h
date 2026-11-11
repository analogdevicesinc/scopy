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

#ifndef PINGVIEW_H
#define PINGVIEW_H

#include "scopy-dcdebugger_export.h"

#include <QPointer>
#include <QWidget>

class QLabel;
class QPushButton;

namespace scopy {
class MenuOnOffSwitch;
namespace gui {
class MenuSpinbox;
}
namespace component {
class Ping;
}
} // namespace scopy

namespace scopy::dcdebugger {

// Ping tab of a Ping node: start / stop the periodic reachability check, its period, a one-off
// check and the outcome of the last one. Backend-agnostic (component::Ping only). It never
// imposes a state: it shows the Ping as the backend or another plugin left it and follows its
// changes live; it only acts on the Ping when the user uses the controls.
class SCOPY_DCDEBUGGER_EXPORT PingView : public QWidget
{
	Q_OBJECT
public:
	static constexpr int MIN_PERIOD_MS = 100;
	static constexpr int MAX_PERIOD_MS = 60000;
	static constexpr int PERIOD_STEP_MS = 100;

	explicit PingView(component::Ping *ping, QWidget *parent = nullptr);

	component::Ping *ping() const { return m_ping; }

public Q_SLOTS:
	// Re-reads the Ping's state (monitoring, interval, last result).
	void refresh();

private:
	QWidget *buildControlSection(QWidget *parent);
	QWidget *buildStatusSection(QWidget *parent);
	void pingNow();
	void updateControls();
	void updateStatus();

	QPointer<component::Ping> m_ping;
	MenuOnOffSwitch *m_monitoring;
	gui::MenuSpinbox *m_period; // in ms
	QPushButton *m_pingNowBtn;
	QLabel *m_result;
	QLabel *m_checkedAt;
	QLabel *m_lost;
	QLabel *m_unavailable;
	bool m_checking = false;
};

} // namespace scopy::dcdebugger

#endif // PINGVIEW_H
