/*
 * Copyright 2017 Analog Devices, Inc.
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

#ifndef SPECTRUM_ANALYZER_HPP
#define SPECTRUM_ANALYZER_HPP

#include <gnuradio/top_block.h>
#include <gnuradio/fft/window.h>
#include <gnuradio/blocks/complex_to_mag_squared.h>

#include "apiObject.hpp"
#include "iio_manager.hpp"
#include "scope_sink_f.h"
#include "fft_block.hpp"
#include "FftDisplayPlot.h"
#include "osc_adc.h"
#include "tool.hpp"
#include "plot_utils.hpp"

#include <QWidget>

extern "C" {
	struct iio_buffer;
	struct iio_channel;
	struct iio_context;
	struct iio_device;
}

namespace Ui {
class SpectrumAnalyzer;
}

namespace adiscope {
class SpectrumChannel;
class Filter;
class ChannelWidget;
class DbClickButtons;
}

class QPushButton;
class QButtonGroup;
class QGridLayout;

namespace adiscope {
class SpectrumAnalyzer_API;
class SpectrumChannel_API;
class SpectrumMarker_API;

class SpectrumAnalyzer: public Tool
{
	friend class SpectrumChannel_API;
	friend class SpectrumAnalyzer_API;
	friend class SpectrumMarker_API;
	friend class ToolLauncher_API;

	Q_OBJECT

public:

	enum FftWinType {
		FLAT_TOP = 0,
		RECTANGULAR = 1,
		TRIANGULAR = 2,
		HAMMING = 3,
		HANN = 4,
		BLACKMAN_HARRIS = 5,
		KAISER = 6,
	};

	typedef boost::shared_ptr<SpectrumChannel> channel_sptr;

	explicit SpectrumAnalyzer(struct iio_context *iio, Filter *filt,
	                          std::shared_ptr<GenericAdc> adc, QPushButton *runButton,
	                          QJSEngine *engine, ToolLauncher *parent);
	~SpectrumAnalyzer();

Q_SIGNALS:
	void isRunning(bool);

private Q_SLOTS:
	void on_btnToolSettings_toggled(bool checked);
	void on_btnSettings_pressed();
	void on_btnSweep_toggled(bool checked);
	void on_btnMarkers_toggled(bool checked);
	void on_comboBox_type_currentIndexChanged(const QString&);
	void on_comboBox_window_currentIndexChanged(const QString&);
	void on_spinBox_averaging_valueChanged(int);
	void runStopToggled(bool);
	void onChannelSettingsToggled(bool);
	void onChannelSelected(bool);
	void onChannelEnabled(bool);
	void onStartStopChanged();
	void onCenterSpanChanged();
	void onMarkerToggled(int, bool);
	void onMarkerSelected(int);
	void on_btnLeftPeak_clicked();
	void on_btnRightPeak_clicked();
	void on_btnUpAmplPeak_clicked();
	void on_btnDnAmplPeak_clicked();
	void on_btnMaxPeak_clicked();
	void on_cmb_rbw_currentIndexChanged(int index);
	void on_cmb_units_currentIndexChanged(const QString&);
	void onPlotNewMarkerData();
	void onPlotMarkerSelected(uint chIdx, uint mkIdx);
	void onMarkerFreqPosChanged(double);
	void onPlotSampleRateUpdated(double);
	void onPlotSampleCountUpdated(uint);
	void singleCaptureDone();
	void on_btnMarkerTable_toggled(bool checked);
	void onTopValueChanged(double);
	void onRangeValueChanged(double);

private:
	void build_gnuradio_block_chain();
	void build_gnuradio_block_chain_no_ctx();
	void start_blockchain_flow();
	void stop_blockchain_flow();
	void writeAllSettingsToHardware();
	int channelIdOfOpenedSettings() const;
	void setSampleRate(double sr);
	void setFftSize(uint size);
	void setMarkerEnabled(int ch_idx, int mrk_idx, bool en);
	void updateWidgetsRelatedToMarker(int mrk_idx);
	void setCurrentMarkerLabelData(int chIdx, int mkIdx);
	void updateCrtMrkLblVisibility();
	void updateMrkFreqPosSpinBtnLimits();
	void updateMrkFreqPosSpinBtnValue();

	QList<SpectrumChannel_API *> ch_api;
	QList<SpectrumMarker_API *> marker_api;

	QPair<int, int> getGridLayoutPosFromIndex(QGridLayout *layout,
	                int index) const;

private:
	Ui::SpectrumAnalyzer *ui;
	adiscope::DbClickButtons *marker_selector;

	QButtonGroup *settings_group;
	QButtonGroup *channels_group;
	FftDisplayPlot *fft_plot;

	QList<channel_sptr> channels;

	adiscope::scope_sink_f::sptr fft_sink;
	iio_manager::port_id *fft_ids;

	boost::shared_ptr<iio_manager> iio;
	std::shared_ptr<GenericAdc> adc;
	const std::string adc_name;
	int num_adc_channels;
	int adc_bits_count;
	int crt_channel_id;
	bool searchVisiblePeaks;
	int crt_peak;
	uint max_peak_count;
	double sample_rate;
	int sample_rate_divider;
	uint fft_size;
	QList<uint> bin_sizes;
	MetricPrefixFormatter freq_formatter;

	gr::top_block_sptr top_block;

	static std::vector<std::pair<QString,
	       FftDisplayPlot::MagnitudeType>> mag_types;
	static std::vector<std::pair<QString,
	       FftDisplayPlot::AverageType>> avg_types;
	static std::vector<std::pair<QString, FftWinType>> win_types;
	static std::vector<QString> markerTypes;
};

class SpectrumChannel: public QObject
{
	Q_OBJECT
	friend class SpectrumChannel_API;

public:
	boost::shared_ptr<adiscope::fft_block> fft_block;
	gr::blocks::complex_to_mag_squared::sptr ctm_block;

	SpectrumChannel(int id, const QString& name, FftDisplayPlot *plot);

	int id() const
	{
		return m_id;
	}
	QString name() const
	{
		return m_name;
	}
	ChannelWidget *widget() const
	{
		return m_widget;
	}

	bool isSettingsOn() const;
	void setSettingsOn(bool on);

	float lineWidth() const;
	void setLinewidth(float);

	QColor color() const;
	void setColor(const QColor&);

	uint averaging() const;
	void setAveraging(uint);

	FftDisplayPlot::AverageType averageType() const;
	void setAverageType(FftDisplayPlot::AverageType);

	SpectrumAnalyzer::FftWinType fftWindow() const;
	void setFftWindow(SpectrumAnalyzer::FftWinType win, int taps);

private:
	int m_id;
	QString m_name;
	float m_line_width;
	QColor m_color;
	uint m_averaging;
	FftDisplayPlot::AverageType m_avg_type;
	SpectrumAnalyzer::FftWinType m_fft_win;
	FftDisplayPlot *m_plot;
	ChannelWidget *m_widget;

	float calcCoherentPowerGain(const std::vector<float>& win) const;
	void scaletFftWindow(std::vector<float>& win, float gain);

	static std::vector<float> build_win(SpectrumAnalyzer::FftWinType type,
	                                    int ntaps);
};

class SpectrumChannel_API : public ApiObject
{
	Q_OBJECT
	Q_PROPERTY(bool enabled READ enabled WRITE enable);
	Q_PROPERTY(int type READ type WRITE setType);
	Q_PROPERTY(int window READ window WRITE setWindow);
	Q_PROPERTY(int averaging READ averaging WRITE setAveraging);

public:
	explicit SpectrumChannel_API(SpectrumAnalyzer *sp,
	                             boost::shared_ptr<SpectrumChannel> spch) :
		ApiObject(), spch(spch),sp(sp) {}
	~SpectrumChannel_API() {}

	bool enabled();
	int type();
	int window();
	int averaging();

	void enable(bool);
	void setType(int);
	void setWindow(int);
	void setAveraging(int);

private:
	SpectrumAnalyzer *sp;
	boost::shared_ptr<SpectrumChannel> spch;
};

class SpectrumMarker_API :public ApiObject
{
	Q_OBJECT

	Q_PROPERTY(int chId READ chId WRITE setChId);
	Q_PROPERTY(int mkId READ mkId WRITE setMkId);
	Q_PROPERTY(bool en READ enabled WRITE setEnabled);
	Q_PROPERTY(int type READ type WRITE setType);
	Q_PROPERTY(double freq READ freq WRITE setFreq);

	int m_chid;
	int m_mkid;
	int m_type;
public:
	explicit SpectrumMarker_API(SpectrumAnalyzer *sp,int chid, int mkid) :
		ApiObject(), sp(sp), m_mkid(mkid), m_chid(chid) {}
	~SpectrumMarker_API() {}

	int chId();
	void setChId(int);

	int mkId();
	void setMkId(int);

	int type();
	void setType(int);

	double freq();
	void setFreq(double);

	bool enabled();
	void setEnabled(bool);

	SpectrumAnalyzer *sp;

};

class SpectrumAnalyzer_API : public ApiObject
{
	Q_OBJECT
	Q_PROPERTY(bool running READ running WRITE run STORED false);
	Q_PROPERTY(double startFreq READ startFreq WRITE setStartFreq);
	Q_PROPERTY(double stopFreq  READ stopFreq  WRITE setStopFreq);
	Q_PROPERTY(QString units READ units WRITE setUnits);
	Q_PROPERTY(QString resBW READ resBW WRITE setResBW);
	Q_PROPERTY(double topScale READ topScale WRITE setTopScale);
	Q_PROPERTY(double range READ range WRITE setRange);
	Q_PROPERTY(QVariantList channels READ getChannels);
	Q_PROPERTY(int currentChannel READ currentChannel WRITE setCurrentChannel);
	Q_PROPERTY(bool markerTableVisible READ markerTableVisible WRITE
	           setMarkerTableVisible);
	Q_PROPERTY(QVariantList markers READ getMarkers);
public:
	explicit SpectrumAnalyzer_API(SpectrumAnalyzer *sp) :
		ApiObject(), sp(sp) {}
	~SpectrumAnalyzer_API() {}

private:
	SpectrumAnalyzer *sp;
	bool running();
	void run(bool);
	QVariantList getChannels();
	QVariantList getMarkers();

	int currentChannel();
	void setCurrentChannel(int);

	double startFreq();
	void setStartFreq(double);

	double stopFreq();
	void setStopFreq(double);

	QString units();
	void setUnits(QString);

	QString resBW();
	void setResBW(QString);

	double topScale();
	void setTopScale(double);

	double range();
	void setRange(double);

	bool markerTableVisible();
	void setMarkerTableVisible(bool);
};

} // namespace adiscope

#endif // SPECTRUM_ANALYZER_HPP
