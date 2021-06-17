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

#ifndef M2K_OSCILLOSCOPE_H
#define M2K_OSCILLOSCOPE_H

#include <iio.h>

/* GNU Radio includes */
#include <gnuradio/blocks/short_to_float.h>
#include <iio/device_source.h>
#include <gnuradio/blocks/complex_to_mag_squared.h>
#include <gnuradio/analog/rail_ff.h>
#include <gnuradio/blocks/keep_one_in_n.h>
#include <gnuradio/blocks/vector_sink.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/blocks/add_blk.h>
#include <gnuradio/blocks/null_sink.h>


/* Qt includes */
#include <QPair>
#include <QPushButton>
#include <QStringList>
#include <QVector>
#include <QWidget>
#include <QButtonGroup>
#include <QMap>
#include <QQueue>

/* Local includes */
#include "apiObject.hpp"
#include "oscilloscope_plot.hpp"
#include "iio_manager.hpp"
#include "filter.hpp"
#include "fft_block.hpp"
#include "scope_sink_f.h"
#include "xy_sink_c.h"
#include "histogram_sink_f.h"
#include "ConstellationDisplayPlot.h"
#include "FftDisplayPlot.h"
#include "HistogramDisplayPlot.h"
#include "gui/spinbox_a.hpp"
#include "trigger_settings.hpp"
#include "plot_utils.hpp"
#include "tool.hpp"
#include "gui/osc_import_settings.h"
#include "math.hpp"
#include "scroll_filter.hpp"
#include "cancel_dc_offset_block.h"
#include "frequency_compensation_filter.h"
#include "oscilloscope_api.hpp"
#include "logicanalyzer/logic_analyzer.h"

/* libm2k includes */
#include <libm2k/analog/m2kanalogin.hpp>
#include <libm2k/digital/m2kdigital.hpp>
#include <libm2k/m2k.hpp>

/* gr-m2k includes */
#include <m2k/digital_in_source.h>
#include "logic_analyzer_sink.h"
#include "mixed_signal_sink.h"

/*Generated UI */
#include "ui_math_panel.h"

#define TIMEBASE_THRESHOLD 0.1

class QJSEngine;
class SymmetricBufferMode;

namespace Ui {
	class Oscilloscope;
	class OscGeneralSettings;
	class ChannelSettings;
	class MeasurementsPanel;
	class StatisticsPanel;
	class CursorReadouts;
	class CursorsSettings;
}

namespace adiscope {
	class CustomPushButton;
	class Oscilloscope_API;
	class Channel_API;
	class MeasurementData;
	class MeasurementGui;
	class MeasureSettings;
	class StateUpdater;
	class AnalogBufferPreviewer;
	class ChannelWidget;
	class signal_sample;

	class Oscilloscope : public Tool
	{
		friend class Oscilloscope_API;
		friend class Channel_API;
		friend class Channel_Digital_Filter_API;
		friend class ToolLauncher_API;

		Q_OBJECT

	public:
		explicit Oscilloscope(struct iio_context *ctx, Filter *filt,
				ToolMenuItem *toolMenuItem, QJSEngine *engine,
				ToolLauncher *parent = 0);
		~Oscilloscope();

		void settingsLoaded();

		bool getTrigger_input() const;
		void setTrigger_input(bool value);

		void add_ref_waveform(QString name, QVector<double> xData, QVector<double> yData, unsigned int sampleRate);
		void remove_ref_waveform(QString name);
		void setNativeDialogs(bool nativeDialogs) override;

		void setLogicAnalyzer(logic::LogicAnalyzer *la);

	Q_SIGNALS:
		void triggerALevelChanged(double);
		void triggerBLevelChanged(double);
		void triggerPositionChanged(double);
		void selectedChannelChanged(int);
		void activateExportButton();
		void startRunning(bool);
		void importFileLoaded(bool);
		void showTool();

	private Q_SLOTS:
		void btnExport_clicked();

		void on_actionClose_triggered();
		void on_boxCursors_toggled(bool on);
		void on_boxMeasure_toggled(bool on);

		void onFFT_view_toggled(bool visible);
		void onHistogram_view_toggled(bool visible);
		void onXY_view_toggled(bool visible);

		void on_btnAddMath_toggled(bool);
		void on_btnCursors_toggled(bool);
		void on_btnMeasure_toggled(bool);
		void on_btnTrigger_toggled(bool);
		void on_btnGeneralSettings_toggled(bool);

		void onTriggerSourceChanged(int);
		void onTimeTriggerDelayChanged(double);
		void onTriggerLevelChanged(double);
		void onTriggerModeChanged(int);

		void onVertScaleValueChanged(double value);
		void onHorizScaleValueChanged(double value);
		void onVertOffsetValueChanged(double value);
		void onTimePositionChanged(double value);


		void onChannelCouplingChanged(bool en);
		void onChannelOffsetChanged(unsigned int chnIdx, double value);
		void on_xyLineThickness_currentIndexChanged(int idx);

		void onChannelWidgetEnabled(bool);
		void onChannelWidgetSelected(bool);
		void onChannelWidgetMenuToggled(bool);
		void onChannelWidgetDeleteClicked();

		void rightMenuFinished(bool opened);

		void toggle_blockchain_flow(bool);
		void runStopToggled(bool);
		void periodicFlowRestart(bool force=false);
		void autosetNextStep();
		void autosetFinalStep();
		void autosetFindPeaks();
		bool autosetFindFrequency();
		void setupAutosetFreqSweep();
		void singleCaptureDone();

		void onMeasuremetsAvailable();

		void onMeasurementActivated(int id, int chnIdx);
		void onMeasurementDeactivated(int id, int chnIdx);
		void onMeasurementSelectionListChanged();

		void onStatisticActivated(int id, int chnIdx);
		void onStatisticDeactivated(int id, int chnIdx);
		void onStatisticSelectionListChanged();
		void onStatisticsEnabled(bool on);
		void onStatisticsReset();
		void onGatingEnabled(bool on);
		void onLeftGateChanged(double);
		void onRightGateChanged(double);

		void onCursorReadoutsChanged(struct cursorReadoutsText);

		void onIioDataRefillTimeout();
		void onPlotNewData();

		void on_btnSettings_clicked(bool checked);
		void channelLineWidthChanged(int id);

		void update_chn_settings_panel(int id);

		void updateGainMode();
		void setGainMode(uint chnIdx, libm2k::analog::M2K_RANGE gain_mode);
		void setChannelHwOffset(uint chnIdx, double offset);

		void on_xyPlotLineType_toggled(bool checked);
		void setup_xy_channels();

		void openEditMathPanel(bool on);

		void updateTriggerLevelValue(std::vector<float> value);
		void configureAcCouplingTrigger(bool);

		void readPreferences();

		void onCmbMemoryDepthChanged(QString);
		void setSinksDisplayOneBuffer(bool);
		void cleanBuffersAllSinks();
		void resetStreamingFlag(bool);
		void onFilledScreen(bool, unsigned int);

		void toggleCursorsMode(bool toggled);
		void toolDetached(bool);
		void setFilteringEnabled(bool set);

		void enableMixedSignalView(ChannelWidget *cw);
		void disableMixedSignalView();
		void setDigitalPlotCurvesParams();

	public Q_SLOTS:
		void requestAutoset();
		void enableLabels(bool);
		void import();		
		void run() override;
		void stop() override;

	private:
		libm2k::context::M2k* m_m2k_context;
		libm2k::analog::M2kAnalogIn* m_m2k_analogin;
		libm2k::digital::M2kDigital* m_m2k_digital;
		bool m_filtering_enabled;

		unsigned int nb_channels, nb_math_channels;
		unsigned int nb_ref_channels;
		double active_sample_rate;
		double noZoomXAxisWidth;
		unsigned long active_sample_count;
		unsigned long active_plot_sample_count;
		long long active_trig_sample_count;
		double active_time_pos;
		double memory_adjusted_time_pos;
		double last_set_time_pos;
		unsigned long last_set_sample_count;
		int zoom_level;
		bool plot_samples_sequentially, d_displayOneBuffer, d_shouldResetStreaming;
		double horiz_offset;
		bool reset_horiz_offset;
		double time_trigger_offset;
		int min_detached_width;
		bool miniHistogram;

		int autosetMaxIndexAmpl;
		int autosetFFTIndex;
		double autosetFrequency;
		double autosetMaxAmpl;
		double autosetMinAmpl;
		int autosetSampleRateCnt;
		int autosetChannel;
		bool autosetEnabled;
		const int autosetSkippedTimeSamples = 4096;
		const int autosetFFTSize = 8192;
		const int autosetNrOfSkippedTones=50;
		const int autosetValidTone = 51;

		Ui::Oscilloscope *ui;
		Ui::OscGeneralSettings *gsettings_ui;
		Ui::ChannelSettings *ch_ui;
		adiscope::TriggerSettings trigger_settings;
		adiscope::MeasureSettings *measure_settings;
		CapturePlot plot;
		FftDisplayPlot fft_plot;
		ConstellationDisplayPlot xy_plot;
		HistogramDisplayPlot hist_plot;
		Ui::MeasurementsPanel *measure_panel_ui;
		QWidget *measurePanel;
		Ui::CursorReadouts *cursor_readouts_ui;
		QWidget *cursorReadouts;
		Ui::StatisticsPanel *statistics_panel_ui;
		Ui::CursorsSettings *cr_ui;
		QWidget *statisticsPanel;
		AnalogBufferPreviewer *buffer_previewer;
		ExportSettings *exportSettings;
		CustomPlotPositionButton *cursorsPositionButton;

		MouseWheelWidgetGuard *wheelEventGuard;

		QPair<Ui::MathPanel, Math*> *math_pair;
		bool addChannel;
		QTabWidget *tabWidget;
		QWidget *ref;

		QVector<QVector<double>> import_data;
		QString import_error;
		ImportSettings *importSettings;
		bool lastFunctionValid;

		QMap<int, bool> exportConfig;

		std::shared_ptr<SymmetricBufferMode> symmBufferMode;

		adiscope::scope_sink_f::sptr qt_time_block;
		adiscope::scope_sink_f::sptr qt_fft_block;
		adiscope::xy_sink_c::sptr qt_xy_block;
		adiscope::histogram_sink_f::sptr qt_hist_block;
		boost::shared_ptr<iio_manager> iio;
		gr::basic_block_sptr adc_samp_conv_block;

		QMap<QString, QPair<gr::basic_block_sptr,
		gr::basic_block_sptr>> math_sinks;
		QMap<QString, boost::shared_ptr<gr::analog::rail_ff>> math_rails;
		std::vector<boost::shared_ptr<gr::blocks::multiply_const_ff>> math_probe_atten;

		iio_manager::port_id *ids;
		iio_manager::port_id *hist_ids;
		iio_manager::port_id *autoset_id;

		ScaleSpinButton *timeBase;
		PositionSpinButton *timePosition;
		ScaleSpinButton *voltsPerDiv;
		PositionSpinButton *voltsPosition;

		ScaleSpinButton *refChannelTimeBase;

		bool fft_is_visible, hist_is_visible, xy_is_visible, autosetRequested;
		bool statistics_enabled;
		QList<bool> high_gain_modes;
		std::vector<double> channel_offset;

		bool hCursorsEnabled;
		bool vCursorsEnabled;
		bool gatingEnabled;

		std::vector<bool> chnAcCoupled;
		bool triggerAcCoupled;
		QPair<boost::shared_ptr<signal_sample>, int> triggerLevelSink;
		boost::shared_ptr<gr::blocks::keep_one_in_n> keep_one;
		boost::shared_ptr<gr::blocks::vector_sink_f> autosetFFTSink;

		bool trigger_is_forced;
		bool new_data_is_triggered;
		bool trigger_input;
		CapturePlot::TriggerState trigger_state;
		StateUpdater *triggerUpdater;

		int fft_size;
		int autoset_fft_size;
		int fft_plot_size;

		NumberSeries voltsPerDivList;
		NumberSeries secPerDivList;

		MetricPrefixFormatter vertMeasureFormat;
		TimePrefixFormatter horizMeasureFormat;

		int current_channel;
		int current_ch_widget;
		unsigned int math_chn_counter;

		QButtonGroup *channels_group; // selected state of each channel

		QPushButton *menuRunButton;

		QList<Channel_API *> channels_api;

		QList<std::shared_ptr<MeasurementData>> measurements_data;
		QList<std::shared_ptr<MeasurementGui>> measurements_gui;

		QList<QPair<std::shared_ptr<MeasurementData>,
			Statistic>> statistics_data;

		QList<CustomPushButton *> menuOrder;

		QQueue<QPair<CustomPushButton *, bool>> menuButtonActions;

		QVector<double> probe_attenuation;

		void writeAllSettingsToHardware();

		void comboBoxUpdateToValue(QComboBox *box, double value, std::vector<double>list);

		void settings_panel_update(int id);
		void settings_panel_size_adjust();
		void triggerRightMenuToggle(CustomPushButton *, bool checked);
		void toggleRightMenu(CustomPushButton *, bool checked);
		void create_add_channel_panel();
		void add_math_channel(const std::string& function);
		unsigned int find_curve_number();
		ChannelWidget *channelWidgetAtId(int id);
		void update_measure_for_channel(int ch_idx);
		QString getChannelRangeStringVDivHelper(int ch);
		void setAllSinksSampleCount(unsigned long sample_count);
		void autosetFFT();

		void updateRunButton(bool ch_enabled);

		void fillCursorReadouts(const struct cursorReadoutsText&);

		void measure_panel_init();
		void measure_settings_init();
		void measureLabelsRearrange();
		void measureUpdateValues();
		void measureCreateAndAppendGuiFrom(const MeasurementData&);

		void statistics_panel_init();
		void statisticsUpdateValues();
		void statisticsReset();
		void statisticsUpdateGui();
		void statisticsUpdateGuiTitleColor();
		void statisticsUpdateGuiPosIndex();

		void updateBufferPreviewer();
		void export_settings_init();
		void pause(bool paused);
		void cursor_panel_init();
		void setFFT_params(bool force=false);
		void setChannelWidgetIndex(int chnIdx);

		void init_channel_settings();
		void editMathChannelFunction(int id, const std::string &new_function);

		std::vector<boost::shared_ptr<cancel_dc_offset_block>> dc_cancel;
		std::vector<QPair<gr::basic_block_sptr, int> > xy_channels;
		std::vector<QPair<gr::basic_block_sptr, int> > fft_channels;
		int index_x, index_y;
		bool locked;
		boost::shared_ptr<gr::blocks::float_to_complex> ftc;
		std::vector<gr::basic_block_sptr> fft_blocks;
		std::vector<gr::basic_block_sptr> ctm_blocks;

		void cancelZoom();

		void configureAcCoupling(int, bool);
		void activateAcCoupling(int);
		void deactivateAcCoupling(int);
		void activateAcCouplingTrigger(int);
		void deactivateAcCouplingTrigger();
		void clearMathChannels();
		void add_ref_waveform(unsigned int chIdx);
		void init_selected_measurements(int, std::vector<int>);
		void init_buffer_scrolling();
		bool gainUpdateNeeded();
		int binSearchPointOnXaxis(double time);
		void scaleHistogramPlot(bool newData = false);
		void toggleMiniHistogramPlotVisible(bool enabled);
		void resetHistogramDataPoints();
		bool isIioManagerStarted() const;
		void updateXyPlotScales();
		void setSampleRate(double sample_rate);
		double getSampleRate();

		logic::LogicAnalyzer *m_logicAnalyzer;
		bool m_mixedSignalViewEnabled;
		gr::m2k::digital_in_source::sptr logic_source;
		gr::top_block_sptr logic_top_block;
		logic_analyzer_sink::sptr logic_sink;
		gr::blocks::short_to_float::sptr s2f;
		gr::blocks::add_ff::sptr add;
		gr::blocks::null_sink::sptr nullSink;
		std::vector<QWidget *> m_mixedSignalViewMenu;

		gr::m2k::mixed_signal_source::sptr mixed_source;
		mixed_signal_sink::sptr mixed_sink;
		QMetaObject::Connection showLogicAnalyzerTriggerConnection;
	};
}
#endif /* M2K_OSCILLOSCOPE_H */
