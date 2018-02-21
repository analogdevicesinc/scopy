/*
 * Copyright 2016 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef POWER_CONTROLLER_HPP
#define POWER_CONTROLLER_HPP

#include <QPushButton>
#include <QTimer>

#include "apiObject.hpp"
#include "spinbox_a.hpp"
#include "tool.hpp"

extern "C" {
	struct iio_context;
	struct iio_channel;
}

namespace Ui {
	class PowerController;
}

class QJSEngine;
class QShowEvent;
class QHideEvent;

namespace adiscope {
	class PowerController_API;

	class PowerController : public Tool
	{
		friend class PowerController_API;
		friend class ToolLauncher_API;

		Q_OBJECT

	public:
		const int AVERAGE_COUNT = 5;

		explicit PowerController(struct iio_context *ctx,
				QPushButton *runButton, QJSEngine *engine,
				ToolLauncher *parent = 0);
		~PowerController();

	public Q_SLOTS:
		void dac1_set_enabled(bool enabled);
		void dac2_set_enabled(bool enabled);
		void dac1_set_value(double value);
		void dac2_set_value(double value);
		void update_lcd();
		void sync_enabled(bool enabled);

	private Q_SLOTS:
		void startStop(bool start);
		void ratioChanged(int percent);

	private:
		Ui::PowerController *ui;
		struct iio_channel *ch1w, *ch2w, *ch1r, *ch2r, *pd_pos, *pd_neg;
		QTimer timer;
		bool in_sync;
		QList<long long> averageVoltageCh1;
		QList<long long> averageVoltageCh2;

		PositionSpinButton *valuePos, *valueNeg;
		QMap<QString, double> calibrationParam;

		void showEvent(QShowEvent *event);
		void hideEvent(QHideEvent *event);
	};

	class PowerController_API : public ApiObject
	{
		Q_OBJECT

		Q_PROPERTY(bool sync READ syncEnabled WRITE enableSync);
		Q_PROPERTY(int tracking_percent
				READ getTrackingPercent
				WRITE setTrackingPercent);
		Q_PROPERTY(double dac1_value READ valueDac1 WRITE setValueDac1);
		Q_PROPERTY(double dac2_value READ valueDac2 WRITE setValueDac2);

	public:
		explicit PowerController_API(PowerController *pw) :
			ApiObject(), pw(pw) {}
		~PowerController_API() {}

		bool syncEnabled() const;
		void enableSync(bool en);

		int getTrackingPercent() const;
		void setTrackingPercent(int percent);

		double valueDac1() const;
		void setValueDac1(double value);

		double valueDac2() const;
		void setValueDac2(double value);

	private:
		PowerController *pw;
	};
}

#endif /* POWER_CONTROLLER_HPP */
