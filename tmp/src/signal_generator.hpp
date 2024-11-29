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
#include <QSharedPointer>

#include "apiObject.hpp"
#include "filter.hpp"
#include "oscilloscope_plot.hpp"
#include "scope_sink_f.h"
#include "tool.hpp"
#include "filemanager.h"

#include "gnuradio/analog/noise_type.h"

/* libm2k includes */
#include <libm2k/analog/m2kanalogout.hpp>
#include <libm2k/m2k.hpp>

#include <gui/externalloadlineedit.h>


extern "C" {
	struct iio_context;
}

namespace Ui {
class SignalGenerator;
}

class QJSEngine;

namespace adiscope {
struct signal_generator_data;
struct time_block_data;
class SignalGenerator_API;
class ChannelWidget;
class PhaseSpinButton;
class PositionSpinButton;
class ScaleSpinButton;

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
	SG_TRA_WAVE = 109,
	SG_SAW_WAVE = gr::analog::GR_SAW_WAVE,
	SG_INV_SAW_WAVE = 108,
	SG_STAIR_WAVE =110,

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
	                         Filter *filt, ToolMenuItem *toolMenuItem,
	                         QJSEngine *engine, ToolLauncher *parent);
	~SignalGenerator();

	static const size_t min_buffer_size = 1024;
	static const unsigned long default_sample_rate = 1000000;
	static constexpr float max_frequency = 30000000;

	static double get_best_ratio(double ratio, double max, double *fract);

	void settingsLoaded();

private:
	const size_t m_maxNbOfSamples;
	libm2k::context::M2k* m_m2k_context;
	libm2k::analog::M2kAnalogOut* m_m2k_analogout;

	Ui::SignalGenerator *ui;
	CapturePlot *m_plot;
	gr::top_block_sptr top_block;
	struct time_block_data *time_block_data;

	PhaseSpinButton *phase;
	PositionSpinButton  *filePhase, *stairPhase;
	PositionSpinButton *offset, *fileOffset;
	PositionSpinButton *constantValue, *dutycycle;
	ScaleSpinButton *amplitude, *frequency;
	ScaleSpinButton *riseTime, *fallTime;
	PositionSpinButton *stepsUp, *stepsDown;
	ScaleSpinButton *holdHighTime, *holdLowTime;
	ScaleSpinButton *fileSampleRate, *fileAmplitude;
	ScaleSpinButton *mathRecordLength, *noiseAmplitude, *mathSampleRate;
	ExternalLoadLineEdit *load;

	FileManager *fileManager;

	int currentChannel;
	double sample_rate;
	double max_sample_rate;
	unsigned long nb_points;
	double nr_of_periods;

	QButtonGroup *settings_group;
	QButtonGroup *channels_group;
	QQueue<QPair<int, bool>> menuButtonActions;

	std::vector<std::vector<double>> buffers;
	QVector<ChannelWidget *> channels;

	QSharedPointer<signal_generator_data> getData(QWidget *obj);
	QSharedPointer<signal_generator_data> getCurrentData();

	void start();	
	void resetZoom();

	void updatePreview();
	void updateRightMenuForChn(int chIdx);
	void updateAndToggleMenu(int chIdx, bool open);
	void triggerRightMenuToggle(int chIdx, bool checked);
	void loadFileFromPath(QString filename);
	void reloadFileFromPath();

	gr::basic_block_sptr getSignalSource(
	        gr::top_block_sptr top,
		double sample_rate,
	        struct signal_generator_data& data, double phase_correction=0.0);

	gr::basic_block_sptr getNoise(QWidget *obj,gr::top_block_sptr top);
	gr::basic_block_sptr getSource(QWidget *obj,
				       double sample_rate,
	                               gr::top_block_sptr top, bool     phase_correction=false);
	gr::basic_block_sptr displayResampler(double samp_rate,
					      double freq,
					      gr::top_block_sptr top,
					      gr::basic_block_sptr generated_wave,
					      gr::basic_block_sptr noiseSrc,
					      gr::basic_block_sptr noiseAdd);

	static void reduceFraction(double input,long *numerator, long *denominator, long precision=1000000);
	static size_t gcd(size_t a, size_t b);
	static size_t lcm(size_t a, size_t b);
	static int sg_waveform_to_idx(enum sg_waveform wave);

	size_t get_samples_count(unsigned int chnIdx, double sample_rate, bool perfect = false);
	double get_best_sample_rate(unsigned int chnIdx);
	void calc_sampling_params(unsigned int chnIdx,
				  double sample_rate,
	                          unsigned long& out_sample_rate,
	                          unsigned long& out_oversampling_ratio);
	bool use_oversampling(unsigned int chnIdx);

	bool sample_rate_forced(unsigned int chnIdx);
	double get_forced_sample_rate(unsigned int chnIdx);

	double zoomT1;
	double zoomT2;

	double zoomT1OnScreen;
	double zoomT2OnScreen;

	std::vector<float>get_stairstep(int rise, int fall, float amplitude, float offset, int phase);

	enum sg_file_format getFileFormat(QString filePath);
	bool loadParametersFromFile(QSharedPointer<signal_generator_data> ptr,
	                            QString filePath);
	void loadFileChannelData(int chIdx);
	bool riffCompare(riff_header_t& ptr, const char *id2);
	bool chunkCompare(chunk_header_t& ptr, const char *id2);

public Q_SLOTS:
	void run() override;
	void stop() override;

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
	void phaseIndexChanged();
	void noiseAmplitudeChanged(double val);
	void noiseTypeChanged(int val);
	void lineThicknessChanged(int index);
	void externalLoadChanged(double val);
	void trapezoidalComputeFrequency();
	void riseChanged(double value);
	void fallChanged(double value);
	void holdHighChanged(double value);
	void holdLowChanged(double value);
	void loadFileCurrentChannelData();
	void stepsUpChanged(double value);
	void stepsDownChanged(double value);
	void stairPhaseChanged(double value);


	void mathRecordLengthChanged(double val);
	void mathSampleRateChanged(double value);

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
Q_SIGNALS:
	void showTool();
};

enum SIGNAL_TYPE {
	SIGNAL_TYPE_CONSTANT	= 0,
	SIGNAL_TYPE_WAVEFORM	= 1,
	SIGNAL_TYPE_BUFFER	= 2,
	SIGNAL_TYPE_MATH	= 3,
};

struct signal_generator_data {
	enum SIGNAL_TYPE type;
	unsigned int id;
	bool enabled;
	// SIGNAL_TYPE_CONSTANT
	float constant;
	// SIGNAL_TYPE_WAVEFORM
	double amplitude;
	float offset;
	double frequency;
	double phase;
	int indexValue;
	double dutycycle;
	enum sg_waveform waveform;

	double rise;
	double holdh;
	double fall;
	double holdl;

	int steps_up;
	int steps_down;
	int stairphase;
	// SIGNAL_TYPE_BUFFER
	double file_sr;
	double file_amplitude;
	double file_offset;
	unsigned long file_phase;
	unsigned long file_nr_of_channels;
	unsigned long file_channel;
	std::vector<uint32_t> file_nr_of_samples;
	std::vector<float> file_data; // vector for each channel
	std::vector<float> stairdata;
	QString file;
	QString file_message;
	QStringList file_channel_names;
	enum sg_file_format file_type;
	wav_header_t file_wav_hdr;
	//bool file_loaded;
	// SIGNAL_TYPE_MATH
	QString function;
	double math_record_length;
	double math_sr;
	// NOISE
	gr::analog::noise_type_t noiseType;
	float noiseAmplitude;
	float lineThickness;
	double load;
};

struct time_block_data {
	scope_sink_f::sptr time_block;
	unsigned long nb_channels;
};
}
Q_DECLARE_METATYPE(gr::analog::noise_type_t)

#endif /* M2K_SIGNAL_GENERATOR_H */
