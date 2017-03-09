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

#ifndef SCOPY_NETWORK_ANALYZER_HPP
#define SCOPY_NETWORK_ANALYZER_HPP

#include "apiObject.hpp"
#include "iio_manager.hpp"
#include "signal_sample.hpp"

#include <QWidget>
#include <QtConcurrentRun>

extern "C" {
	struct iio_buffer;
	struct iio_channel;
	struct iio_context;
	struct iio_device;
}

namespace Ui {
	class NetworkAnalyzer;
}

class QPushButton;
class QJSEngine;

namespace adiscope {
	class NetworkAnalyzer_API;
	class Filter;

	class NetworkAnalyzer : public QWidget
	{
		friend class NetworkAnalyzer_API;

		Q_OBJECT

	public:
		explicit NetworkAnalyzer(struct iio_context *ctx, Filter *filt,
				QPushButton *runButton, QJSEngine *engine,
				QWidget *parent = 0);
		~NetworkAnalyzer();

	private:
		Ui::NetworkAnalyzer *ui;
		struct iio_channel *dac1, *dac2;
		struct iio_device *adc;
		boost::shared_ptr<iio_manager> iio;

		QFuture<void> thd;
		bool stop;

		NetworkAnalyzer_API *net_api;

		void run();

		static size_t get_sin_samples_count(
				const struct iio_channel *chn,
				unsigned long rate,
				double frequency);

		static unsigned long get_best_sin_sample_rate(
				const struct iio_channel *chn,
				double frequency);

		static struct iio_buffer * generateSinWave(
				const struct iio_device *dev,
				double frequency,
				double amplitude,
				double offset,
				unsigned long rate,
				size_t samples_count);

	private Q_SLOTS:
		void startStop(bool start);
		void updateNumSamples();

	Q_SIGNALS:
		void sweepDone();
	};

	class NetworkAnalyzer_API : public ApiObject
	{
		Q_OBJECT

		Q_PROPERTY(double min_freq READ getMinFreq WRITE setMinFreq);
		Q_PROPERTY(double max_freq READ getMaxFreq WRITE setMaxFreq);
		Q_PROPERTY(double step_size READ getStepSize WRITE setStepSize);
		Q_PROPERTY(double amplitude READ getAmplitude
				WRITE setAmplitude);
		Q_PROPERTY(double offset READ getOffset WRITE setOffset);

	public:
		explicit NetworkAnalyzer_API(NetworkAnalyzer *net) :
			ApiObject(TOOL_NETWORK_ANALYZER), net(net) {}
		~NetworkAnalyzer_API() {}

		double getMinFreq() const;
		double getMaxFreq() const;
		double getStepSize() const;
		double getAmplitude() const;
		double getOffset() const;

		void setMinFreq(double freq);
		void setMaxFreq(double freq);
		void setStepSize(double step);
		void setAmplitude(double amp);
		void setOffset(double offset);

	private:
		NetworkAnalyzer *net;
	};
} /* namespace adiscope */

#endif /* SCOPY_NETWORK_ANALYZER_HPP */
