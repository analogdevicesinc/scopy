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

#include "spinbox_a.hpp"
#include "apiObject.hpp"
#include "iio_manager.hpp"
#include "signal_sample.hpp"
#include "tool.hpp"
#include "dbgraph.hpp"
#include "handles_area.hpp"
#include <QtConcurrentRun>
#include "customPushButton.hpp"
#include "scroll_filter.hpp"
#include <gnuradio/top_block.h>
#include <gnuradio/blocks/head.h>
#include <gnuradio/blocks/vector_sink_s.h>
#include <gnuradio/analog/sig_source_f.h>
#include <gnuradio/blocks/float_to_short.h>



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
class GenericAdc;

class NetworkAnalyzer : public Tool
{
	friend class NetworkAnalyzer_API;
	friend class ToolLauncher_API;

	Q_OBJECT

public:
	explicit NetworkAnalyzer(struct iio_context *ctx, Filter *filt,
				 std::shared_ptr<GenericAdc>& adc_dev,
				 QPushButton *runButton, QJSEngine *engine,
				 ToolLauncher *parent);
	~NetworkAnalyzer();

private:
	Ui::NetworkAnalyzer *ui;
	struct iio_channel *amp1, *amp2;
	std::vector<iio_channel *> dac_channels;
	struct iio_device *adc;
	std::shared_ptr<GenericAdc> adc_dev;
	boost::shared_ptr<iio_manager> iio;

	// Sine generation blocks
	gr::top_block_sptr top_block;
	gr::analog::sig_source_f::sptr source_block;
	gr::blocks::float_to_short::sptr f2s_block;
	gr::blocks::head::sptr head_block;
	gr::blocks::vector_sink_s::sptr vector_block;

	QVector<unsigned long> sampleRates;
	size_t fixedRate;



	dBgraph m_dBgraph;
	dBgraph m_phaseGraph;
	bool wasChecked;

	typedef struct NetworkAnalyzerIteration {
		NetworkAnalyzerIteration():
			frequency(0),
			rate(0),
			bufferSize(0) {}
		NetworkAnalyzerIteration(double frequency,
					 size_t rate,
					 size_t bufferSize):
			frequency(frequency),
			rate(rate),
			bufferSize(bufferSize) {}

		double frequency;
		size_t rate;
		size_t bufferSize;
		size_t adcRate;
		size_t adcBuffer;
	} networkIteration;

	QVector<networkIteration> iterations;


	PlotLineHandleH *d_hCursorHandle1;
	PlotLineHandleH *d_hCursorHandle2;
	bool d_cursorsEnabled;

	ScaleSpinButton *samplesCount;
	ScaleSpinButton *minFreq;
	ScaleSpinButton *maxFreq;
	ScaleSpinButton *amplitude;
	PositionSpinButton *offset;
	PositionSpinButton *magMax;
	PositionSpinButton *magMin;
	PositionSpinButton *phaseMax;
	PositionSpinButton *phaseMin;

	void setMinimumDistanceBetween(SpinBoxA *min, SpinBoxA *max, double distance);

	HorizHandlesArea *d_bottomHandlesArea;

	QQueue<QPair<CustomPushButton *, bool>> menuButtonActions;

	MouseWheelWidgetGuard *wheelEventGuard;

	QFuture<void> thd;
	bool stop;

	void run();

	struct iio_buffer *generateSinWave(
		const struct iio_device *dev,
		double frequency,
		double amplitude,
		double offset,
		unsigned long rate,
		size_t samples_count);

	void configHwForNetworkAnalyzing();

	void triggerRightMenuToggle(CustomPushButton *btn, bool checked);
	void toggleRightMenu(CustomPushButton *btn, bool checked);

	void computeFrequencyArray();
	void updateGainMode();
	void computeCaptureParams(double frequency, size_t& buffer_size,
				  size_t& adc_rate);

private Q_SLOTS:
	void startStop(bool start);
	void updateNumSamples(bool force = false);
	void plot(double frequency, double mag, double mag2, double phase);

	void toggleCursors(bool en);
	void onVbar1PixelPosChanged(int pos);
	void onVbar2PixelPosChanged(int pos);
	void readPreferences();
	void onGraphIndexChanged(int);
	void on_btnExport_clicked();
	void rightMenuFinished(bool opened);

public Q_SLOTS:

	void showEvent(QShowEvent *event);

Q_SIGNALS:
	void sweepDone();
	void showTool();
};
} /* namespace adiscope */

#endif /* SCOPY_NETWORK_ANALYZER_HPP */
