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
#include "tool.hpp"
#include "plot_utils.hpp"
#include "spinbox_a.hpp"
#include "customPushButton.hpp"
#include "startstoprangewidget.h"

#include <QWidget>
#include <QQueue>

/* libm2k includes */
#include <libm2k/analog/genericanalogin.hpp>
#include <libm2k/analog/m2kanalogin.hpp>
#include <libm2k/m2k.hpp>
#include <libm2k/generic.hpp>

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
				  ToolMenuItem *toolMenuItem,
	                          QJSEngine *engine, ToolLauncher *parent);
	~SpectrumAnalyzer();

	void setNativeDialogs(bool nativeDialogs) override;

public Q_SLOTS:
	void readPreferences();	
	void run() override;
	void stop() override;

Q_SIGNALS:
	void started(bool);
	void showTool();

private Q_SLOTS:
	void on_btnToolSettings_toggled(bool checked);
	void on_btnSettings_clicked(bool checked);
	void on_btnSweep_toggled(bool checked);
	void on_btnMarkers_toggled(bool checked);
	void on_comboBox_type_currentIndexChanged(const QString&);
	void on_comboBox_window_currentIndexChanged(const QString&);
	void on_spinBox_averaging_valueChanged(int);
	void runStopToggled(bool);
	void onChannelSettingsToggled(bool);
	void onChannelSelected(bool);
	void onChannelEnabled(bool);
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
	void rightMenuFinished(bool opened);	
	void btnExportClicked();
	void updateRunButton(bool);
	void on_btnAddRef_toggled(bool checked);
	void on_btnBrowseFile_clicked();
	void on_btnImport_clicked();
	void onReferenceChannelDeleted();

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
	void add_ref_waveform(QVector<double> xData, QVector<double> yData);
	void add_ref_waveform(unsigned int chIdx);
	QString getReferenceChannelName() const;

	QList<SpectrumChannel_API *> ch_api;
	QList<SpectrumMarker_API *> marker_api;

	QPair<int, int> getGridLayoutPosFromIndex(QGridLayout *layout,
	                int index) const;

	QQueue<QPair<CustomPushButton *, bool>> menuButtonActions;
	QList<CustomPushButton *> menuOrder;

	QVector<QVector<double>> import_data;
	unsigned int nb_ref_channels;
	QVector<ChannelWidget *> referenceChannels;
	unsigned int selected_ch_settings;
	QVector<QStringList> importedChannelDetails;

private:
	libm2k::context::M2k* m_m2k_context;
	libm2k::analog::M2kAnalogIn* m_m2k_analogin;
	libm2k::context::Generic* m_generic_context;
	libm2k::analog::GenericAnalogIn* m_generic_analogin;
	Ui::SpectrumAnalyzer *ui;
	adiscope::DbClickButtons *marker_selector;

	QButtonGroup *settings_group;
	QButtonGroup *channels_group;
	FftDisplayPlot *fft_plot;

	PositionSpinButton *range;
	PositionSpinButton *top;
	PositionSpinButton *marker_freq_pos;

	StartStopRangeWidget *startStopRange;

	QList<channel_sptr> channels;

	adiscope::scope_sink_f::sptr fft_sink;
	iio_manager::port_id *fft_ids;

	boost::shared_ptr<iio_manager> iio;
	const std::string adc_name;
	unsigned int m_adc_nb_channels;
	int adc_bits_count;
	int crt_channel_id;
	bool searchVisiblePeaks;
	int crt_peak;
	uint max_peak_count;
	double sample_rate;
	double m_max_sample_rate;
	int sample_rate_divider;
	uint fft_size;
	QList<uint> bin_sizes;
	MetricPrefixFormatter freq_formatter;

	gr::top_block_sptr top_block;

	bool marker_menu_opened;

	static std::vector<std::pair<QString,
	       FftDisplayPlot::MagnitudeType>> mag_types;
	static std::vector<std::pair<QString,
	       FftDisplayPlot::AverageType>> avg_types;
	static std::vector<std::pair<QString, FftWinType>> win_types;
	static std::vector<QString> markerTypes;
	void triggerRightMenuToggle(CustomPushButton *btn, bool checked);
	void toggleRightMenu(CustomPushButton *btn, bool checked);
	void updateChannelSettingsPanel(unsigned int id);
	ChannelWidget *getChannelWidgetAt(unsigned int id);
	void updateMarkerMenu(unsigned int id);
	bool isIioManagerStarted() const;
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
} // namespace adiscope

#endif // SPECTRUM_ANALYZER_HPP
