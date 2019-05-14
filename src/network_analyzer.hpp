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
#include <gnuradio/fft/goertzel_fc.h>
#include <gnuradio/blocks/vector_source_f.h>
#include <gnuradio/blocks/vector_sink_f.h>
#include <gnuradio/blocks/multiply_cc.h>
#include <gnuradio/blocks/multiply_conjugate_cc.h>
#include <gnuradio/blocks/complex_to_arg.h>
#include <gnuradio/blocks/complex_to_mag_squared.h>
#include <gnuradio/filter/dc_blocker_ff.h>
#include <gnuradio/blocks/moving_average_cc.h>
#include <gnuradio/blocks/moving_average_ff.h>
#include <gnuradio/blocks/skiphead.h>
#include "cancel_dc_offset_block.h"

#include "oscilloscope.hpp"

#include "TimeDomainDisplayPlot.h"

#include "networkanalyzerbufferviewer.h"
#include "startstoprangewidget.h"
#include "adc_sample_conv.hpp"

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
class GenericDac;

class NetworkAnalyzer : public Tool
{
	friend class NetworkAnalyzer_API;
	friend class ToolLauncher_API;

	Q_OBJECT

public:
	explicit NetworkAnalyzer(struct iio_context *ctx, Filter *filt,
				 std::shared_ptr<GenericAdc>& adc_dev,
				 QList<std::shared_ptr<GenericDac>> dacs,
				 QPushButton *runButton, QJSEngine *engine,
				 ToolLauncher *parent);
	~NetworkAnalyzer();

	void setOscilloscope(Oscilloscope *osc);

private:
	Ui::NetworkAnalyzer *ui;
	struct iio_channel *amp1, *amp2;
	std::vector<iio_channel *> dac_channels;
	struct iio_device *adc;
	std::shared_ptr<GenericAdc> adc_dev;
	boost::shared_ptr<iio_manager> iio;
	QList<std::shared_ptr<GenericDac>> dacs;

	// Sine generation blocks
	gr::top_block_sptr top_block;
	gr::analog::sig_source_f::sptr source_block;
	gr::blocks::float_to_short::sptr f2s_block;
	gr::blocks::head::sptr head_block;
	gr::blocks::vector_sink_s::sptr vector_block;

	QVector<unsigned long> sampleRates;

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
	} networkIteration;

	typedef struct NetworkAnalyzerIterationStats {
		NetworkAnalyzerIterationStats(double dcVoltage,
					      M2kAdc::GainMode gain,
					      bool hasError):
			dcVoltage(dcVoltage),
			gain(gain),
			hasError(hasError) {}
		NetworkAnalyzerIterationStats():
			dcVoltage(0),
			gain(M2kAdc::LOW_GAIN_MODE),
			hasError(false) {}

		double dcVoltage;
		M2kAdc::GainMode gain;
		bool hasError;
	} NetworkIterationStats;

	QVector<networkIteration> iterations;
	QVector<NetworkIterationStats> iterationStats;

	boost::thread *iterationsThread;
	bool iterationsThreadCanceled;
	bool iterationsThreadReady;

	bool isIterationsThreadReady();
	bool isIterationsThreadCanceled();

	gr::fft::goertzel_fc::sptr goertzel1;
	gr::fft::goertzel_fc::sptr goertzel2;
	gr::blocks::copy::sptr copy1;
	gr::blocks::copy::sptr copy2;
	iio_manager::port_id id1;
	iio_manager::port_id id2;
	gr::blocks::head::sptr head1;
	gr::blocks::head::sptr head2;
	gr::blocks::vector_sink_f::sptr sink1;
	gr::blocks::vector_sink_f::sptr sink2;
	gr::blocks::complex_to_mag_squared::sptr c2m1;
	gr::blocks::complex_to_mag_squared::sptr c2m2;
	gr::blocks::multiply_conjugate_cc::sptr conj;
	gr::filter::dc_blocker_ff::sptr dcBlocker1;
	gr::filter::dc_blocker_ff::sptr dcBlocker2;
	gr::blocks::complex_to_arg::sptr c2a;
	boost::shared_ptr<signal_sample> signal;
	boost::shared_ptr<adc_sample_conv> adc_conv;
	boost::shared_ptr<cancel_dc_offset_block> dc_cancel1;
	boost::shared_ptr<cancel_dc_offset_block> dc_cancel2;
	float mag1, mag2, phase;
	bool captureDone;
	bool filterDc;

	boost::mutex iterationsReadyMutex;
	boost::condition_variable iterationsReadyCv;
	boost::mutex bufferMutex;

	NetworkAnalyzerBufferViewer *bufferPreviewer;
	QVector<Buffer> capturedData;

	StartStopRangeWidget *startStopRange;

	bool justStarted;
	bool autoAdjustGain;

	PlotLineHandleH *d_hCursorHandle1;
	PlotLineHandleH *d_hCursorHandle2;
	FreePlotLineHandleH *d_frequencyHandle;
	bool d_cursorsEnabled;

	ScaleSpinButton *samplesCount;
	ScaleSpinButton *amplitude;
	PositionSpinButton *offset;
	PositionSpinButton *magMax;
	PositionSpinButton *magMin;
	PositionSpinButton *phaseMax;
	PositionSpinButton *phaseMin;
	PositionSpinButton *pushDelay;
	PositionSpinButton *captureDelay;

	void setMinimumDistanceBetween(SpinBoxA *min, SpinBoxA *max, double distance);

	HorizHandlesArea *d_bottomHandlesArea;

	QQueue<QPair<CustomPushButton *, bool>> menuButtonActions;

	MouseWheelWidgetGuard *wheelEventGuard;

	QFuture<void> thd;
	bool stop;
	void goertzel();

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
	void updateGainMode();
	void computeCaptureParams(double frequency, size_t& buffer_size,
				  size_t& adc_rate);

	QPair<double, double> getPhaseInterval();
	void computeIterations();

	double autoUpdateGainMode(double magnitude, double magnitudeGain, float dcVoltage);

	void _configureDacFlowgraph();

	void _configureAdcFlowgraph(size_t bufferSize = 0);
	unsigned long _getBestSampleRate(double frequency, const iio_device *dev);
	size_t _getSamplesCount(double frequency, unsigned long rate, bool perfect = false);
	void computeFrequencyArray();

	bool _checkMagForOverrange(double magnitude);
private Q_SLOTS:
	void startStop(bool start);
	void updateNumSamples(bool force = false);
	void plot(double frequency, double mag, double mag2, double phase, float dcVoltage);
	void _saveChannelBuffers(double frequency, double sample_rate, std::vector<float> data1, std::vector<float> data2);

	void toggleCursors(bool en);
	void onVbar1PixelPosChanged(int pos);
	void onVbar2PixelPosChanged(int pos);
	void readPreferences();
	void onGraphIndexChanged(int);
	void on_btnExport_clicked();
	void rightMenuFinished(bool opened);
	void onMinMaxPhaseChanged(double value);
	void onFrequencyBarMoved(int pos);
	void toggleBufferPreview(bool toggle = false);

public Q_SLOTS:

	void showEvent(QShowEvent *event);

Q_SIGNALS:
	void sweepDone();
	void showTool();
};
} /* namespace adiscope */

#endif /* SCOPY_NETWORK_ANALYZER_HPP */
