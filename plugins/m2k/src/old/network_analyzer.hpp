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

#ifndef SCOPY_NETWORK_ANALYZER_HPP
#define SCOPY_NETWORK_ANALYZER_HPP

#include "TimeDomainDisplayPlot.h"
#include "cancel_dc_offset_block.h"
#include "dbgraph.hpp"
#include "frequency_compensation_filter.h"
#include "gui/customPushButton.h"
#include "gui/mousewheelwidgetguard.h"
#include "gui/spinbox_a.hpp"
#include "gui/startstoprangewidget.h"
#include "handles_area.hpp"
#include "iio_manager.hpp"
#include "m2ktool.hpp"
#include "networkanalyzerbufferviewer.h"
#include "oscilloscope.hpp"
#include "pluginbase/apiobject.h"
#include "signal_sample.hpp"

#include <gnuradio/analog/sig_source.h>
#include <gnuradio/blocks/complex_to_arg.h>
#include <gnuradio/blocks/complex_to_mag_squared.h>
#include <gnuradio/blocks/float_to_short.h>
#include <gnuradio/blocks/head.h>
#include <gnuradio/blocks/moving_average.h>
#include <gnuradio/blocks/multiply.h>
#include <gnuradio/blocks/multiply_conjugate_cc.h>
#include <gnuradio/blocks/skiphead.h>
#include <gnuradio/blocks/vector_sink.h>
#include <gnuradio/blocks/vector_source.h>
#include <gnuradio/fft/goertzel_fc.h>
#include <gnuradio/filter/dc_blocker_ff.h>
#include <gnuradio/scopy/goertzel_scopy_fc.h>
#include <gnuradio/top_block.h>

#include <QStackedWidget>
#include <QtConcurrentRun>

extern "C"
{
	struct iio_context;
}

namespace Ui {
class NetworkAnalyzer;
}

class QPushButton;
class QJSEngine;

namespace scopy::m2k {
class NetworkAnalyzer_API;
class Filter;

class NetworkAnalyzer : public M2kTool
{
	friend class NetworkAnalyzer_API;
	friend class ToolLauncher_API;

	Q_OBJECT

public:
	explicit NetworkAnalyzer(struct iio_context *ctx, Filter *filt, ToolMenuEntry *toolMenuItem,
				 m2k_iio_manager *m2k_man, QJSEngine *engine, QWidget *parent = nullptr);
	~NetworkAnalyzer();
	QPushButton *getRunButton();
	void setOscilloscope(Oscilloscope *osc);

private:
	Ui::NetworkAnalyzer *ui;
	libm2k::context::M2k *m_m2k_context;
	libm2k::analog::M2kAnalogOut *m_m2k_analogout;
	libm2k::analog::M2kAnalogIn *m_m2k_analogin;
	unsigned int m_adc_nb_channels, m_dac_nb_channels;
	std::shared_ptr<iio_manager> iio;
	bool m_initFlowgraph;

	// Sine generation blocks
	gr::top_block_sptr top_block;
	gr::analog::sig_source_f::sptr source_block;
	gr::blocks::head::sptr head_block;
	gr::blocks::vector_sink_f::sptr vector_block;

	std::vector<double> sampleRates;

	dBgraph m_dBgraph;
	dBgraph m_phaseGraph;
	bool wasChecked;

	typedef struct NetworkAnalyzerIteration
	{
		NetworkAnalyzerIteration()
			: frequency(0)
			, rate(0)
			, bufferSize(0)
		{}
		NetworkAnalyzerIteration(double frequency, size_t rate, size_t bufferSize)
			: frequency(frequency)
			, rate(rate)
			, bufferSize(bufferSize)
		{}

		double frequency;
		size_t rate;
		size_t bufferSize;
	} networkIteration;

	typedef struct NetworkAnalyzerIterationStats
	{
		NetworkAnalyzerIterationStats(double dcVoltage, libm2k::analog::M2K_RANGE gain, bool hasError)
			: dcVoltage(dcVoltage)
			, gain(gain)
			, hasError(hasError)
		{}
		NetworkAnalyzerIterationStats()
			: dcVoltage(0)
			, gain(libm2k::analog::PLUS_MINUS_25V)
			, hasError(false)
		{}

		double dcVoltage;
		libm2k::analog::M2K_RANGE gain;
		bool hasError;
	} NetworkIterationStats;

	QVector<networkIteration> iterations;
	QVector<NetworkIterationStats> iterationStats;

	std::thread *iterationsThread;
	bool iterationsThreadCanceled;
	bool iterationsThreadReady;

	bool isIterationsThreadReady();
	bool isIterationsThreadCanceled();

	gr::top_block_sptr capture_top_block;
	gr::blocks::vector_source_s::sptr capture1;
	gr::blocks::vector_source_s::sptr capture2;
	scopy::frequency_compensation_filter::sptr f11, f12, f21, f22;
	gr::blocks::short_to_float::sptr s2f1;
	gr::blocks::short_to_float::sptr s2f2;
	gr::scopy::goertzel_scopy_fc::sptr goertzel1;
	gr::scopy::goertzel_scopy_fc::sptr goertzel2;
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
	std::shared_ptr<signal_sample> signal;
	gr::blocks::multiply_const_ff::sptr adc_conv1, adc_conv2;
	std::shared_ptr<cancel_dc_offset_block> dc_cancel1;
	std::shared_ptr<cancel_dc_offset_block> dc_cancel2;
	float mag1, mag2, phase;
	bool captureDone;
	bool filterDc;

	std::mutex iterationsReadyMutex;
	std::condition_variable iterationsReadyCv;
	std::mutex bufferMutex;

	NetworkAnalyzerBufferViewer *bufferPreviewer;
	QVector<Buffer> capturedData;

	StartStopRangeWidget *startStopRange;

	bool justStarted;
	bool autoAdjustGain;

	FreePlotLineHandleH *d_frequencyHandle;
	bool d_cursorsEnabled;

	ScaleSpinButton *samplesCount;
	ScaleSpinButton *samplesPerDecadeCount;
	ScaleSpinButton *samplesStepSize;
	QStackedWidget *sampleStackedWidget;
	ScaleSpinButton *amplitude;
	PositionSpinButton *offset;
	PositionSpinButton *magMax;
	PositionSpinButton *magMin;
	PositionSpinButton *phaseMax;
	PositionSpinButton *phaseMin;
	PositionSpinButton *pushDelay;
	PositionSpinButton *captureDelay;

	void setMinimumDistanceBetween(SpinBoxA *min, SpinBoxA *max, double distance);

	QQueue<QPair<CustomPushButton *, bool>> menuButtonActions;

	MouseWheelWidgetGuard *wheelEventGuard;

	QFuture<void> thd;

	bool m_stop;

	bool m_hasReference;
	bool m_importDataLoaded;
	QVector<QVector<double>> m_importData;
	unsigned int m_nb_averaging;
	unsigned int m_nb_periods;

	void goertzel();
	void setFilterParameters();

	std::vector<double> generateSinWave(unsigned int chn_idx, double frequency, double amplitude, double offset,
					    unsigned long rate, size_t samples_count);

	void configHwForNetworkAnalyzing();

	void triggerRightMenuToggle(CustomPushButton *btn, bool checked);
	void toggleRightMenu(CustomPushButton *btn, bool checked);
	void updateGainMode();
	void computeCaptureParams(double frequency, size_t &buffer_size, size_t &adc_rate);

	QPair<double, double> getPhaseInterval();
	void computeIterations();

	double autoUpdateGainMode(double magnitude, double magnitudeGain, float dcVoltage);

	void _configureDacFlowgraph();

	void _configureAdcFlowgraph(size_t bufferSize = 0);
	unsigned long _getBestSampleRate(double frequency, unsigned int chn_idx);
	size_t _getSamplesCount(double frequency, unsigned long rate, bool perfect = false);
	void computeFrequencyArray();

	bool _checkMagForOverrange(double magnitude);
private Q_SLOTS:
	void startStop(bool start);
	void updateNumSamples(bool force = false);
	void updateNumSamplesPerDecade(bool force = false);
	void updateSampleStepSize(bool force = false);
	void plot(double frequency, double mag, double mag2, double phase, float dcVoltage);
	void _saveChannelBuffers(double frequency, double sample_rate, std::vector<float> data1,
				 std::vector<float> data2);

	void toggleCursors(bool en);
	void readPreferences() override;
	void onGraphIndexChanged(int);
	void on_btnExport_clicked();
	void rightMenuFinished(bool opened);
	void onMinMaxPhaseChanged(double value);
	void onFrequencyBarMoved(int pos);
	void toggleBufferPreview(bool toggle = false);

	void validateSpinboxAveraging();
	void on_spinBox_averaging_valueChanged(int n);
	void on_spinBox_periods_valueChanged(int n);
	void validateSpinboxPeriods();
public Q_SLOTS:

	void showEvent(QShowEvent *event) override;
	void run() override;
	void stop() override;

Q_SIGNALS:
	void sweepDone();
	void sweepStart();
	void showTool();
};
} // namespace scopy::m2k

#endif /* SCOPY_NETWORK_ANALYZER_HPP */
