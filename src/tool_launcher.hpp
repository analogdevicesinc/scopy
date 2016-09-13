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

#ifndef M2K_TOOL_LAUNCHER_H
#define M2K_TOOL_LAUNCHER_H

#include <QMainWindow>
#include <QPair>
#include <QVector>

#include "dmm.hpp"
#include "filter.hpp"
#include "calibration.hpp"
#include "oscilloscope.hpp"
#include "pulseview/pv/widgets/popup.hpp"
#include "power_controller.hpp"
#include "signal_generator.hpp"
#include "logic_analyzer.hpp"

extern "C" {
	struct iio_context;
}

namespace Ui {
	class ToolLauncher;
	class Device;
}

namespace adiscope {
	class ToolLauncher : public QMainWindow
	{
	    Q_OBJECT

	public:
		explicit ToolLauncher(QWidget *parent = 0);
		~ToolLauncher();

	Q_SIGNALS:
		void calibrationDone(float gain_ch1, float gain_ch2);

	private Q_SLOTS:
		void on_btnOscilloscope_clicked();
		void on_btnSignalGenerator_clicked();
		void on_btnDMM_clicked();
		void on_btnPowerControl_clicked();
		void on_btnLogicAnalyzer_clicked();

		void window_destroyed();

		void on_btnHome_clicked();
		void on_btnConnect_clicked(bool pressed);

		void device_btn_clicked(bool pressed);
		void addRemoteContext();
		void destroyPopup();

		void enableCalibTools(float gain_ch1, float gain_ch2);

	private:
		Ui::ToolLauncher *ui;
		QList<QMainWindow *> windows;
		struct iio_context *ctx;

		QVector<QPair<QWidget, Ui::Device> *> devices;

		DMM *dmm;
		PowerController *power_control;
		SignalGenerator *signal_generator;
		Oscilloscope *oscilloscope;
		LogicAnalyzer *logic_analyzer;
		QWidget *current;

		Filter *filter;

		void swapMenu(QWidget *menu);
		void destroyContext();
		bool switchContext(QString &uri);
		void resetStylesheets();
		void calibrate();
		void addContext(const QString& hostname);

		static void apply_m2k_fixes(struct iio_context *ctx);
	};
}

#endif // M2K_TOOL_LAUNCHER_H
