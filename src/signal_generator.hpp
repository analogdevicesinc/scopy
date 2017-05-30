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

#include "apiObject.hpp"
#include "filter.hpp"
#include "oscilloscope_plot.hpp"
#include "scope_sink_f.h"

#include "ui_channel.h"

extern "C" {
	struct iio_buffer;
	struct iio_context;
	struct iio_device;
	struct iio_channel;
}

namespace Ui {
	class SignalGenerator;
}

class QJSEngine;

namespace adiscope {
	struct signal_generator_data;
	struct time_block_data;
	class SignalGenerator_API;

	enum sg_waveform {
		SG_SIN_WAVE = gr::analog::GR_SIN_WAVE,
		SG_SQR_WAVE = gr::analog::GR_SQR_WAVE,
		SG_TRI_WAVE = gr::analog::GR_TRI_WAVE,
		SG_SAW_WAVE = gr::analog::GR_SAW_WAVE,
		SG_INV_SAW_WAVE,
	};

	class SignalGenerator : public QWidget
	{
		friend class SignalGenerator_API;
		friend class ToolLauncher_API;

		Q_OBJECT

	public:
		explicit SignalGenerator(struct iio_context *ctx,
				Filter *filt, QPushButton *runButton,
				QJSEngine *engine, QWidget *parent = 0);
		~SignalGenerator();

		double vlsb_of_channel(const char *channel,
			const char *dev_parent);
		void set_vlsb_of_channel(const char *channel,
			const char *dev_parent, double vlsb);

		static const size_t min_buffer_size = 1024;

		static QVector<unsigned long> get_available_sample_rates(
				const struct iio_device *dev);
		static unsigned long get_max_sample_rate(
				const struct iio_device *dev);

		static double get_best_ratio(double ratio,
				double max, double *fract);

	private:
		Ui::SignalGenerator *ui;
		OscilloscopePlot *plot;
		gr::top_block_sptr top_block;
		struct iio_context *ctx;
		struct time_block_data *time_block_data;

		unsigned int currentChannel;
		unsigned long sample_rate;

		QButtonGroup *settings_group;
		QPushButton *menuRunButton;

		QVector<struct iio_buffer *> buffers;
		QVector<QPair<QWidget, Ui::Channel> *> channels;
		QVector<QPair<struct iio_channel *, double>> channels_vlsb;

		SignalGenerator_API *sg_api;

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
		static int sg_waveform_to_idx(enum sg_waveform wave);

		size_t get_samples_count(const struct iio_device *dev,
				unsigned long sample_rate, bool perfect = false);
		unsigned long get_best_sample_rate(
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

	class SignalGenerator_API : public ApiObject
	{
		Q_OBJECT

		Q_PROPERTY(bool running READ running WRITE run STORED false);

		Q_PROPERTY(QList<int> mode READ getMode WRITE setMode);
		Q_PROPERTY(QList<double> constant_volts
				READ getConstantValue WRITE setConstantValue);
		Q_PROPERTY(QList<int> waveform_type
				READ getWaveformType WRITE setWaveformType);
		Q_PROPERTY(QList<double> waveform_amplitude
				READ getWaveformAmpl WRITE setWaveformAmpl);
		Q_PROPERTY(QList<double> waveform_frequency
				READ getWaveformFreq WRITE setWaveformFreq);
		Q_PROPERTY(QList<double> waveform_offset
				READ getWaveformOfft WRITE setWaveformOfft);
		Q_PROPERTY(QList<double> waveform_phase
				READ getWaveformPhase WRITE setWaveformPhase);
		Q_PROPERTY(QList<double> math_frequency
				READ getMathFreq WRITE setMathFreq);
		Q_PROPERTY(QList<QString> math_function
				READ getMathFunction WRITE setMathFunction);

	public:
		bool running() const;
		void run(bool en);

		QList<int> getMode() const;
		void setMode(const QList<int>& list);

		QList<double> getConstantValue() const;
		void setConstantValue(const QList<double>& list);

		QList<int> getWaveformType() const;
		void setWaveformType(const QList<int>& list);

		QList<double> getWaveformAmpl() const;
		void setWaveformAmpl(const QList<double>& list);

		QList<double> getWaveformFreq() const;
		void setWaveformFreq(const QList<double>& list);

		QList<double> getWaveformOfft() const;
		void setWaveformOfft(const QList<double>& list);

		QList<double> getWaveformPhase() const;
		void setWaveformPhase(const QList<double>& list);

		QList<double> getMathFreq() const;
		void setMathFreq(const QList<double>& list);

		QList<QString> getMathFunction() const;
		void setMathFunction(const QList<QString>& list);

		explicit SignalGenerator_API(SignalGenerator *gen) :
			ApiObject(), gen(gen) {}
		~SignalGenerator_API() {}

	private:
		SignalGenerator *gen;
	};
}

#endif /* M2K_SIGNAL_GENERATOR_H */
