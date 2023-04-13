/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef POWER_CONTROLLER_HPP
#define POWER_CONTROLLER_HPP

#include <QPushButton>
#include <QTimer>

#include "pluginbase/apiobject.h"
#include "gui/spinbox_a.hpp"
#include "m2ktool.hpp"

extern "C" {
	struct iio_context;
}

namespace libm2k {
namespace context {
	class M2k;
}
namespace analog {
	class M2kPowerSupply;
}
}
namespace Ui {
	class PowerController;
}

class QJSEngine;
class QShowEvent;
class QHideEvent;

namespace scopy::m2k {
	class PowerController_API;

	class PowerController : public M2kTool
	{
		friend class PowerController_API;
		friend class ToolLauncher_API;

		Q_OBJECT

	public:
		const int AVERAGE_COUNT = 5;

		explicit PowerController(struct iio_context *ctx,
				ToolMenuEntry *tme, QJSEngine *engine,
				QWidget *parent = 0);
		~PowerController();

	public Q_SLOTS:
		void dac1_set_enabled(bool enabled);
		void dac2_set_enabled(bool enabled);
		void dac1_set_value(double value);
		void dac2_set_value(double value);
		void update_lcd();
		void sync_enabled(bool enabled);
		void run() override;
		void stop() override;

	private Q_SLOTS:
		void startStop(bool start);
		void ratioChanged(int percent);
		void toggleRunButton(bool enabled);
		void readPreferences();

	private:
		Ui::PowerController *ui;
		PositionSpinButton *valuePos;
		PositionSpinButton *valueNeg;
		QTimer timer;
		bool in_sync;
		QList<double> averageVoltageCh1;
		QList<double> averageVoltageCh2;
		libm2k::context::M2k* m_m2k_context;
		libm2k::analog::M2kPowerSupply* m_m2k_powersupply;

		void showEvent(QShowEvent *event);
		void hideEvent(QHideEvent *event);

	Q_SIGNALS:
		void showTool();
	};
}
#endif /* POWER_CONTROLLER_HPP */
