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

#ifndef M2K_SIGNAL_GENERATOR_H
#define M2K_SIGNAL_GENERATOR_H

#include <gnuradio/analog/sig_source_waveform.h>
#include <gnuradio/top_block.h>

#include <QButtonGroup>
#include <QPushButton>
#include <QTreeWidgetItem>
#include <QSharedPointer>
#include <QWidget>

#include "filter.hpp"
#include "oscilloscope_plot.hpp"
#include "scope_sink_f.h"
#include "spinbox_a.hpp"

#include "ui_channel.h"

extern "C" {
	struct iio_buffer;
	struct iio_context;
	struct iio_device;
}

namespace Ui {
	class SignalGenerator;
}

namespace adiscope {
	struct signal_generator_data;
	struct time_block_data;

	enum sg_waveform {
		SG_SIN_WAVE = gr::analog::GR_SIN_WAVE,
		SG_SQR_WAVE = gr::analog::GR_SQR_WAVE,
		SG_TRI_WAVE = gr::analog::GR_TRI_WAVE,
		SG_SAW_WAVE = gr::analog::GR_SAW_WAVE,
		SG_INV_SAW_WAVE,
	};

	class SignalGenerator : public QWidget
	{
	    Q_OBJECT

	public:
		explicit SignalGenerator(struct iio_context *ctx,
				Filter *filt, QPushButton *runButton,
				QWidget *parent = 0);
		~SignalGenerator();

		static const size_t min_buffer_size = 1024;

		static QVector<unsigned long> get_available_sample_rates(
				const struct iio_device *dev);

	private:
		Ui::SignalGenerator *ui;
		OscilloscopePlot *plot;
		gr::top_block_sptr top_block;
		struct iio_context *ctx;
		struct time_block_data *time_block_data;

		bool menuOpened;
		unsigned int currentChannel;
		unsigned long sample_rate;

		QButtonGroup *settings_group;
		QPushButton *menuRunButton;

		QVector<struct iio_buffer *> buffers;

		PositionSpinButton *constantValue;
		PositionSpinButton *amplitude, *offset, *phase;
		ScaleSpinButton *frequency, *mathFrequency;

		QVector<QPair<QWidget, Ui::Channel> *> channels;

		QSharedPointer<signal_generator_data> getData(QWidget *obj);
		QSharedPointer<signal_generator_data> getCurrentData();
		void renameConfigPanel();

		void start();
		void stop();

		void updatePreview();
		void toggleRightMenu(QPushButton *btn);

		gr::basic_block_sptr getSignalSource(
				gr::top_block_sptr top,
				unsigned long sample_rate,
				struct signal_generator_data &data);

		gr::basic_block_sptr getSource(QWidget *obj,
				unsigned long sample_rate,
				gr::top_block_sptr top);

		static size_t gcd(size_t a, size_t b);
		static size_t lcm(size_t a, size_t b);

		size_t get_samples_count(const struct iio_device *dev,
				unsigned long sample_rate);
		unsigned long get_best_sample_rate(
				const struct iio_device *dev);
		static unsigned long get_max_sample_rate(
				const struct iio_device *dev);
		int set_sample_rate(const struct iio_device *dev,
				unsigned long sample_rate);
		bool use_oversampling(const struct iio_device *dev);

	private Q_SLOTS:
		void constantValueChanged(double val);
		void amplitudeChanged(double val);
		void offsetChanged(double val);
		void frequencyChanged(double val);
		void phaseChanged(double val);
		void mathFreqChanged(double val);
		void waveformTypeChanged(int val);
		void tabChanged(int index);
		void channel_box_toggled(bool);
		void toggleRightMenu();
		void rightMenuFinished(bool opened);
		void loadFile();

		void startStop(bool start);
		void setFunction(const QString& function);
	};
}

#endif /* M2K_SIGNAL_GENERATOR_H */
