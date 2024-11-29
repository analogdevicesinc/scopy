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
#include "waterfall_sink_f.h"
#include "fft_block.hpp"
#include "FftDisplayPlot.h"
#include "tool.hpp"
#include "plot_utils.hpp"
#include "WaterfallDisplayPlot.h"
#include "gui/spinbox_a.hpp"
#include "gui/customPushButton.hpp"
#include "gui/startstoprangewidget.h"

#ifdef SPECTRAL_MSR
#include "gui/measure.h"
#endif

#include <QWidget>
#include <QQueue>

/* libm2k includes */
#include <libm2k/analog/genericanalogin.hpp>
#include <libm2k/analog/m2kanalogin.hpp>
#include <libm2k/m2k.hpp>
#include <libm2k/generic.hpp>
#include <gui/smallOnOffSwitch.hpp>

extern "C" {
	struct iio_buffer;
	struct iio_channel;
	struct iio_context;
	struct iio_device;
}

namespace Ui {
class SpectrumAnalyzer;
class CursorReadouts;
class CursorsSettings;

#ifdef SPECTRAL_MSR
class MeasurementsPanel;
#endif
}

namespace adiscope {
class SpectrumChannel;
class Filter;
class ChannelWidget;
class DbClickButtons;

#ifdef SPECTRAL_MSR
class MeasurementData;
class MeasurementGui;
#endif
}

class QPushButton;
class QButtonGroup;
class QGridLayout;

namespace adiscope {
class SpectrumAnalyzer_API;
class SpectrumChannel_API;
class SpectrumMarker_API;

#ifdef SPECTRAL_MSR
class MeasureSettings;
#endif

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

	typedef std::shared_ptr<SpectrumChannel> channel_sptr;

	explicit SpectrumAnalyzer(struct iio_context *iio, Filter *filt,
				  ToolMenuItem *toolMenuItem,
	                          QJSEngine *engine, ToolLauncher *parent);
	~SpectrumAnalyzer();

	void setNativeDialogs(bool nativeDialogs) override;

	void setCurrentAverageIndexLabel(uint chnIdx);
public Q_SLOTS:
	void readPreferences();	
	void run() override;
	void stop() override;
	void setWaterfallWindow(std::vector<float> window, int channel_id);

Q_SIGNALS:
	void started(bool);
	void showTool();
	void selectedChannelChanged(int);

#ifdef SPECTRAL_MSR
	void measurementsAvailable();
#endif

protected:
	void connectCursorHandles();
	void connectZoomers();

private Q_SLOTS:
	void on_btnHistory_toggled(bool checked);
	void onCurrentAverageIndexChanged(uint chnIdx, uint avgIdx);
	void on_btnToolSettings_toggled(bool checked);
	void on_btnSettings_clicked(bool checked);
	void on_btnSweep_toggled(bool checked);
	void on_btnMarkers_toggled(bool checked);

#ifdef SPECTRAL_MSR
	void on_btnMeasure_toggled(bool);
	void on_boxMeasure_toggled(bool);

	void onMeasuremetsAvailable();
	void onMeasurementActivated(int id, int chnIdx);
	void onMeasurementDeactivated(int id, int chnIdx);
	void onMeasurementSelectionListChanged();

	void setMeasuremensEnabled(bool en);
	void onChannelAdded(int);
	void onNewDataReceived();
#endif

	void on_boxCursors_toggled(bool on);
	void on_btnCursors_toggled(bool);
	void onCursorReadoutsChanged(struct cursorReadoutsText);
	void toggleCursorsMode(bool toggled);

	void on_comboBox_type_currentIndexChanged(const QString&);
	void on_comboBox_window_currentIndexChanged(const QString&);
    void on_comboBox_line_thickness_currentIndexChanged(int index);
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
	void onScalePerDivValueChanged(double);
	void onBottomValueChanged(double);
	void rightMenuFinished(bool opened);	
	void btnExportClicked();
	void updateRunButton(bool);
	void on_btnAddRef_toggled(bool checked);
	void on_btnBrowseFile_clicked();
	void on_btnImport_clicked();
	void onReferenceChannelDeleted();
	void refreshCurrentSampleLabel();
	void validateSpinboxAveraging();
	void onWaterfallSizeChanged(double);

private:
	void initInstrumentStrings();
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
	void setYAxisUnit(const QString& type);

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
	adiscope::SmallOnOffSwitch *btnLockHPlots;
	QHBoxLayout *horizontalLockLayout;

#ifdef SPECTRAL_MSR
	QWidget *measurePanel;
	Ui::MeasurementsPanel *measure_panel_ui;
	adiscope::MeasureSettings *measure_settings;
	QList<std::shared_ptr<MeasurementData>> measurements_data;
	QList<std::shared_ptr<MeasurementGui>> measurements_gui;
	QList<Measure *> d_measureObjs;
	bool d_measurementsEnabled;
#endif

	Ui::CursorReadouts *cursor_readouts_ui;
	QWidget *cursorReadouts;
	Ui::CursorsSettings *cr_ui;
	CustomPlotPositionButton *cursorsPositionButton;
	bool hCursorsEnabled;
	bool vCursorsEnabled;

	adiscope::DbClickButtons *marker_selector;
	unsigned int m_nb_overlapping_avg;

	QButtonGroup *settings_group;
	QButtonGroup *channels_group;
	FftDisplayPlot *fft_plot;
	WaterfallDisplayPlot *waterfall_plot;

	ScaleSpinButton *top_scale;
	ScaleSpinButton *bottom_scale;
	PositionSpinButton *unit_per_div;
	PositionSpinButton *waterfall_size;
	PositionSpinButton *top;
	PositionSpinButton *bottom;
	PositionSpinButton *marker_freq_pos;

	StartStopRangeWidget *startStopRange;

	QList<channel_sptr> channels;
	QTimer *sample_timer;
	std::chrono::time_point<std::chrono::system_clock>  m_time_start;

	adiscope::scope_sink_f::sptr fft_sink;
	adiscope::waterfall_sink_f::sptr waterfall_sink;
	iio_manager::port_id *fft_ids;
	iio_manager::port_id *waterfall_ids;

	std::shared_ptr<iio_manager> iio;
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
	QPair<double, double> m_mag_min_max;

	bool marker_menu_opened;

	std::vector<std::pair<QString,
	       FftDisplayPlot::MagnitudeType>> mag_types;
	std::vector<std::pair<QString,
	       FftDisplayPlot::AverageType>> avg_types;
	std::vector<std::pair<QString, FftWinType>> win_types;
	std::vector<QString> markerTypes;
	void triggerRightMenuToggle(CustomPushButton *btn, bool checked);
	void toggleRightMenu(CustomPushButton *btn, bool checked);
	void updateChannelSettingsPanel(unsigned int id);
	ChannelWidget *getChannelWidgetAt(unsigned int id);
	void updateMarkerMenu(unsigned int id);
	bool isIioManagerStarted() const;
	void setCurrentSampleLabel(double);

	void cursor_panel_init();
	void fillCursorReadouts(const struct cursorReadoutsText &);

	bool canSwitchAverageHistory(FftDisplayPlot::AverageType avg_type);

	bool receivedFFTData;
	bool receivedWaterfallData;

	// dockers
	QMainWindow *mainWindow;
	QDockWidget *fftDocker;
	QDockWidget *waterfallDocker;

#ifdef SPECTRAL_MSR
	//din capture plot
	QList<std::shared_ptr<MeasurementData>> measurements(int chnIdx);
	std::shared_ptr<MeasurementData> measurement(int id, int chnIdx);
	void measure();
	int activeMeasurementsCount(int chnIdx);
	Measure* measureOfChannel(int chnIdx) const;
	bool measurementsEnabled();
	void computeMeasurementsForChannel(unsigned int chnIdx, unsigned int sampleRate);

	void cleanUpMeasurementsBeforeChannelRemoval(int chnIdx);

	//functii normale
	void settings_panel_update(int id);
	void settings_panel_size_adjust();
	void update_measure_for_channel(int ch_idx);

	void measure_panel_init();
	void measure_settings_init();
	void init_selected_measurements(int, std::vector<int>);
	void measureUpdateValues();
	void measureLabelsRearrange();
	void measureCreateAndAppendGuiFrom(const MeasurementData&);
#endif
};

class SpectrumChannel: public QObject
{
	Q_OBJECT
	friend class SpectrumChannel_API;

public:
	std::shared_ptr<adiscope::fft_block> fft_block;
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

	uint averageIdx() const;
	void setAverageIdx(uint);

	FftDisplayPlot::AverageType averageType() const;
	void setAverageType(FftDisplayPlot::AverageType);

	SpectrumAnalyzer::FftWinType fftWindow() const;
	void setFftWindow(SpectrumAnalyzer::FftWinType win, int taps);

	bool isAverageHistoryEnabled() const;
	void setAverageHistoryEnabled(bool enabled);
	bool canStoreAverageHistory() const;

	void setGainMode(int index);
	libm2k::analog::M2K_RANGE getGainMode();
	static double win_overlap_factor(SpectrumAnalyzer::FftWinType type);

	static std::vector<float> build_win(SpectrumAnalyzer::FftWinType type,
					    int ntaps);

	std::vector<float> getWindow();

Q_SIGNALS:
	void FftWindowChanged(std::vector<float> window, int channel_id);

private:
	int m_id;
	QString m_name;
	float m_line_width;
	QColor m_color;
	uint m_averaging;
	uint m_average_current_index;
	libm2k::analog::M2K_RANGE m_gain_mode;
	bool m_average_history;
	FftDisplayPlot::AverageType m_avg_type;
	SpectrumAnalyzer::FftWinType m_fft_win;
	FftDisplayPlot *m_plot;
	ChannelWidget *m_widget;
	std::vector<float> d_window;

	float calcCoherentPowerGain(const std::vector<float>& win) const;
	void scaletFftWindow(std::vector<float>& win, float gain);
};
} // namespace adiscope

#endif // SPECTRUM_ANALYZER_HPP
