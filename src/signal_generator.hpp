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
#include <gnuradio/analog/noise_type.h>
#include <gnuradio/top_block.h>

#include <QButtonGroup>
#include <QPushButton>
#include <QTreeWidgetItem>
#include <QSharedPointer>
#include <QWidget>
#include <QQueue>

#include "apiObject.hpp"
#include "filter.hpp"
#include "oscilloscope_plot.hpp"
#include "scope_sink_f.h"
#include "tool.hpp"
#include "hw_dac.h"

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
class GenericDac;
class ChannelWidget;

enum sg_noise {
	SG_NO_NOISE = 0,
	SG_UNIFORM_NOISE = 1,
	SG_GAUSSIAN_NOISE = 2,
	SG_LAPLACIAN_NOISE = 3,
	SG_IMPULSE_NOISE = 4,
};

enum sg_waveform {
	SG_SIN_WAVE = gr::analog::GR_SIN_WAVE,
	SG_SQR_WAVE = gr::analog::GR_SQR_WAVE,
	SG_TRI_WAVE = gr::analog::GR_TRI_WAVE,
	SG_TRA_WAVE = gr::analog::GR_TRA_WAVE,
	SG_SAW_WAVE = gr::analog::GR_SAW_WAVE,
	SG_INV_SAW_WAVE = 108,

};

enum sg_file_format {
	FORMAT_NO_FILE,
	FORMAT_BIN_FLOAT,
	FORMAT_CSV,
	FORMAT_WAVE,
	FORMAT_MAT
};

typedef union {
	struct {
		uint16_t	format;    // Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
		uint16_t	noChan;      // Number of channels 1=Mono 2=Sterio
		uint32_t	SamplesPerSec;  // Sampling Frequency in Hz
		uint32_t	bytesPerSec;    // bytes per second
		uint16_t	blockAlign;     // 2=16-bit mono, 4=16-bit stereo
		uint16_t	bitsPerSample;  // Number of bits per sample
	};
	char header_data[16];
} wav_header_t;

typedef union {
	struct {
		uint8_t riff[4];
		uint32_t size;
		uint8_t id[4];
	};
	char data[12];
} riff_header_t;

typedef union {
	struct {
		uint8_t  id[4];
		uint32_t size;
	};
	char data[8];
} chunk_header_t;

class SignalGenerator : public Tool
{
	friend class SignalGenerator_API;
	friend class ToolLauncher_API;

	Q_OBJECT

public:
	explicit SignalGenerator(struct iio_context *ctx,
	                         QList<std::shared_ptr<GenericDac>> dacs,
	                         Filter *filt, QPushButton *runButton,
	                         QJSEngine *engine, ToolLauncher *parent);
	~SignalGenerator();

	static const size_t min_buffer_size = 1024;
	static const unsigned long default_sample_rate = 1000000;

	static QVector<unsigned long> get_available_sample_rates(
	        const struct iio_device *dev);
	static unsigned long get_max_sample_rate(
	        const struct iio_device *dev);

	static double get_best_ratio(double ratio,
	                             double max, double *fract);

	void settingsLoaded();

private:
	Ui::SignalGenerator *ui;
	OscilloscopePlot *plot;
	gr::top_block_sptr top_block;
	struct time_block_data *time_block_data;
	struct iio_channel *amp1, *amp2;
	QList<std::shared_ptr<GenericDac>> dacs;

	unsigned int currentChannel;
	unsigned long sample_rate;
	unsigned long max_sample_rate;
	unsigned long nb_points;
	double nr_of_periods;

	QButtonGroup *settings_group;
	QQueue<QPair<int, bool>> menuButtonActions;

	QVector<struct iio_buffer *> buffers;
	QVector<ChannelWidget *> channels;
	QVector<QPair<struct iio_channel *,
		        std::shared_ptr<adiscope::GenericDac>>> channel_dac;

	QSharedPointer<signal_generator_data> getData(QWidget *obj);
	QSharedPointer<signal_generator_data> getCurrentData();
	void renameConfigPanel();

	void start();
	void stop();
	void resetZoom();

	void updatePreview();
	void updateRightMenuForChn(int chIdx);
	void updateAndToggleMenu(int chIdx, bool open);
	void triggerRightMenuToggle(int chIdx, bool checked);

	gr::basic_block_sptr getSignalSource(
	        gr::top_block_sptr top,
	        unsigned long sample_rate,
	        struct signal_generator_data& data, double phase_correction=0.0);

	gr::basic_block_sptr getNoise(QWidget *obj,gr::top_block_sptr top);
	gr::basic_block_sptr getSource(QWidget *obj,
	                               unsigned long sample_rate,
	                               gr::top_block_sptr top, bool     phase_correction=false);

	static void reduceFraction(double input,long *numerator, long *denominator, long precision=1000000);
	static size_t gcd(size_t a, size_t b);
	static size_t lcm(size_t a, size_t b);
	static int sg_waveform_to_idx(enum sg_waveform wave);

	size_t get_samples_count(const struct iio_device *dev,
	                         unsigned long sample_rate, bool perfect = false);
	unsigned long get_best_sample_rate(
	        const struct iio_device *dev);
	//int set_sample_rate(const struct iio_device *dev,
	//		unsigned long sample_rate);
	void calc_sampling_params(const struct iio_device *dev,
	                          unsigned long sample_rate,
	                          unsigned long& out_sample_rate,
	                          unsigned long& out_oversampling_ratio);
	bool use_oversampling(const struct iio_device *dev);

	bool sample_rate_forced(const struct iio_device *dev);
	unsigned long get_forced_sample_rate(const struct iio_device *dev);

	double zoomT1;
	double zoomT2;

	double zoomT1OnScreen;
	double zoomT2OnScreen;

	enum sg_file_format getFileFormat(QString filePath);
	void loadParametersFromFile(QSharedPointer<signal_generator_data> ptr,
	                            QString filePath);
	void loadFileChannelData(QWidget *obj);
	bool riffCompare(riff_header_t& ptr, const char *id2);
	bool chunkCompare(chunk_header_t& ptr, const char *id2);
private Q_SLOTS:
	void constantValueChanged(double val);
	void amplitudeChanged(double val);
	void offsetChanged(double val);
	void fileSampleRateChanged(double val);
	void fileChannelChanged(int val);
	void fileAmplitudeChanged(double val);
	void fileOffsetChanged(double val);
	void filePhaseChanged(double val);
	void frequencyChanged(double val);
	void dutyChanged(double val);
	void phaseChanged(double val);
	void noiseAmplitudeChanged(double val);
	void noiseTypeChanged(int val);
	void trapezoidalComputeFrequency();
	void riseChanged(double value);
	void fallChanged(double value);
	void holdHighChanged(double value);
	void holdLowChanged(double value);


	void mathFreqChanged(double val);

	void waveformUpdateUi(int val);
	void waveformTypeChanged(int val);
	void tabChanged(int index);
	void resizeTabWidget(int index);
	void channelWidgetEnabled(bool);
	void channelWidgetMenuToggled(bool);
	void rightMenuFinished(bool opened);
	void loadFile();
	void rescale();

	void startStop(bool start);
	void setFunction(const QString& function);
	void readPreferences();
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
	Q_PROPERTY(QList<double> waveform_duty
		   READ getWaveformDuty WRITE setWaveformDuty);

	Q_PROPERTY(QList<int> noise_type
		   READ getNoiseType WRITE setNoiseType);
	Q_PROPERTY(QList<double> noise_amplitude
		   READ getNoiseAmpl WRITE setNoiseAmpl);
	Q_PROPERTY(QList<double> waveform_rise
		   READ getWaveformRise WRITE setWaveformRise);
	Q_PROPERTY(QList<double> waveform_fall
		   READ getWaveformFall WRITE setWaveformFall);
	Q_PROPERTY(QList<double> waveform_holdhigh
		   READ getWaveformHoldHigh WRITE setWaveformHoldHigh);
	Q_PROPERTY(QList<double> waveform_holdlow
		   READ getWaveformHoldLow WRITE setWaveformHoldLow);



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

	QList<double> getWaveformDuty() const;
	void setWaveformDuty(const QList<double>& list);

	QList<int> getNoiseType() const;
	void setNoiseType(const QList<int>& list);
	QList<double> getNoiseAmpl() const;
	void setNoiseAmpl(const QList<double>& list);

	QList<double> getWaveformHoldLow() const;
	void setWaveformHoldLow(const QList<double>& list);
	QList<double> getWaveformHoldHigh() const;
	void setWaveformHoldHigh(const QList<double>& list);
	QList<double> getWaveformFall() const;
	void setWaveformFall(const QList<double>& list);
	QList<double> getWaveformRise() const;
	void setWaveformRise(const QList<double>& list);

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
