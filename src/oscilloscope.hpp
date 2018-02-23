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

#ifndef M2K_OSCILLOSCOPE_H
#define M2K_OSCILLOSCOPE_H

#include <iio.h>

/* GNU Radio includes */
#include <gnuradio/blocks/short_to_float.h>
#include <gnuradio/iio/device_source.h>
#include <gnuradio/blocks/complex_to_mag_squared.h>
#include <gnuradio/blocks/keep_one_in_n.h>
#include <gnuradio/blocks/vector_sink_f.h>

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
#include "spinbox_a.hpp"
#include "trigger_settings.hpp"
#include "plot_utils.hpp"
#include "osc_adc.h"
#include "tool.hpp"
#include "osc_export_settings.h"
#include "math.hpp"
#include "scroll_filter.hpp"

/*Generated UI */
#include "ui_math_panel.h"

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
		friend class ToolLauncher_API;

		Q_OBJECT

	public:
		explicit Oscilloscope(struct iio_context *ctx, Filter *filt,
				std::shared_ptr<GenericAdc> adc,
				QPushButton *runButton, QJSEngine *engine,
				ToolLauncher *parent = 0);
		~Oscilloscope();

		void settingsLoaded();

	Q_SIGNALS:
		void triggerALevelChanged(double);
		void triggerBLevelChanged(double);
		void triggerPositionChanged(double);
		void selectedChannelChanged(int);
		void activateExportButton();
		void isRunning(bool);

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
		void onChannelOffsetChanged(double value);

		void onChannelWidgetEnabled(bool);
		void onChannelWidgetSelected(bool);
		void onChannelWidgetMenuToggled(bool);
		void onChannelWidgetDeleteClicked();

		void rightMenuFinished(bool opened);

		void toggle_blockchain_flow(bool);
		void runStopToggled(bool);
		void autosetNextStep();
		void autosetFinalStep();
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

		void onCursorReadoutsChanged(struct cursorReadoutsText);

		void onIioDataRefillTimeout();
		void onPlotNewData();

		void on_btnSettings_clicked(bool checked);
		void channelLineWidthChanged(int id);

		void update_chn_settings_panel(int id);

		void updateGainMode();
		void setGainMode(uint chnIdx, M2kAdc::GainMode gain_mode);
		void setChannelHwOffset(uint chnIdx, double offset);

		void on_xyPlotLineType_toggled(bool checked);
		void setup_xy_channels();

		void openEditMathPanel(bool on);

		void updateTriggerLevelValue(std::vector<float> value);
		void configureAcCouplingTrigger(bool);

		void readPreferences();

	public Q_SLOTS:
		void requestAutoset();
		void enableLabels(bool);

	private:
		std::shared_ptr<GenericAdc> adc;
		std::shared_ptr<M2kAdc> m2k_adc;
		unsigned int nb_channels, nb_math_channels;
		double active_sample_rate;
		double noZoomXAxisWidth;
		unsigned long active_sample_count;
		long long active_trig_sample_count;
		double active_time_pos;
		double last_set_time_pos;
		unsigned long last_set_sample_count;
		int zoom_level;

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
		const int autosetValidTone = 250;

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

		iio_manager::port_id *ids;
		iio_manager::port_id *fft_ids;
		iio_manager::port_id *hist_ids;
		iio_manager::port_id *autoset_id;

		ScaleSpinButton *timeBase;
		PositionSpinButton *timePosition;
		ScaleSpinButton *voltsPerDiv;
		PositionSpinButton *voltsPosition;

		bool fft_is_visible, hist_is_visible, xy_is_visible, autosetRequested;
		bool statistics_enabled;
		QList<bool> high_gain_modes;
		std::vector<double> channel_offset;

		std::vector<bool> chnAcCoupled;
		bool triggerAcCoupled;
		std::vector<gr::basic_block_sptr> filterBlocks;
		std::vector<gr::basic_block_sptr> subBlocks;
		QPair<boost::shared_ptr<signal_sample>, int> triggerLevelSink;
		boost::shared_ptr<gr::blocks::keep_one_in_n> keep_one;
		boost::shared_ptr<gr::blocks::vector_sink_f> autosetFFTSink;
		boost::shared_ptr<gr::blocks::vector_sink_f> autosetDataSink;

		bool trigger_is_forced;
		bool new_data_is_triggered;
		CapturePlot::TriggerState trigger_state;
		StateUpdater *triggerUpdater;

		int fft_size;
		int autoset_fft_size;

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
		void create_math_panel();
		void add_math_channel(const std::string& function);
		unsigned int find_curve_number();
		ChannelWidget *channelWidgetAtId(int id);
		void update_measure_for_channel(int ch_idx);
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

		std::vector<QPair<gr::basic_block_sptr, int> > xy_channels;
		int index_x, index_y;
		bool locked;
		boost::shared_ptr<gr::blocks::float_to_complex> ftc;

		void cancelZoom();

		void configureAcCoupling(int, bool);
		void activateAcCoupling(int);
		void deactivateAcCoupling(int);
		void activateAcCouplingTrigger(int);
		void deactivateAcCouplingTrigger();
		void clearMathChannels();
	};

	class Oscilloscope_API : public ApiObject
	{
		Q_OBJECT

		Q_PROPERTY(QVariantList channels READ getChannels);

		Q_PROPERTY(bool running READ running WRITE run STORED false);

		Q_PROPERTY(bool cursors READ hasCursors WRITE setCursors);
        Q_PROPERTY(bool measure READ hasMeasure WRITE setMeasure);
		Q_PROPERTY(bool measure_all
				READ measureAll WRITE setMeasureAll);
		Q_PROPERTY(bool counter READ hasCounter WRITE setCounter);
		Q_PROPERTY(bool statistics
				READ hasStatistics WRITE setStatistics);

		Q_PROPERTY(bool horizontal_cursors READ horizontalCursors
				WRITE setHorizontalCursors)
		Q_PROPERTY(bool vertical_cursors READ verticalCursors
				WRITE setVerticalCursors)

		Q_PROPERTY(double cursor_v1 READ cursorV1 WRITE setCursorV1);
		Q_PROPERTY(double cursor_v2 READ cursorV2 WRITE setCursorV2);
		Q_PROPERTY(double cursor_h1 READ cursorH1 WRITE setCursorH1);
		Q_PROPERTY(double cursor_h2 READ cursorH2 WRITE setCursorH2);
		Q_PROPERTY(int cursors_position READ getCursorsPosition
			  WRITE setCursorsPosition)
		Q_PROPERTY(int cursors_transparency READ getCursorsTransparency
			  WRITE setCursorsTransparency)

		Q_PROPERTY(bool auto_trigger
				READ autoTrigger WRITE setAutoTrigger)
		Q_PROPERTY(bool internal_trigger
				READ internalTrigger WRITE setInternalTrigger)
		Q_PROPERTY(bool external_trigger
				READ externalTrigger WRITE setExternalTrigger)
		Q_PROPERTY(int trigger_source
				READ triggerSource WRITE setTriggerSource)
		Q_PROPERTY(double trigger_level
				READ getTriggerLevel WRITE setTriggerLevel)
		Q_PROPERTY(double trigger_hysteresis READ getTriggerHysteresis
				WRITE setTriggerHysteresis)
		Q_PROPERTY(bool internal_condition READ internalCondition
				WRITE setInternalCondition)
		Q_PROPERTY(bool external_condition READ externalCondition
				WRITE setExternalCondition)

		Q_PROPERTY(QList<QString> math_channels
				READ getMathChannels WRITE setMathChannels
				SCRIPTABLE false /* too complex for now */);

		Q_PROPERTY(double time_position
				READ getTimePos WRITE setTimePos);
		Q_PROPERTY(double time_base READ getTimeBase WRITE setTimeBase);

		Q_PROPERTY(QList<int> measure_en
				READ measureEn WRITE setMeasureEn);

		Q_PROPERTY(QList<int> statistic_en
				READ statisticEn WRITE setStatisticEn)

		Q_PROPERTY(int current_channel READ getCurrentChannel
				WRITE setCurrentChannel)

		Q_PROPERTY(bool fft_en READ getFftEn WRITE setFftEn)
		Q_PROPERTY(bool xy_en READ getXyEn WRITE setXyEn)
		Q_PROPERTY(bool export_all READ getExportAll
			   WRITE setExportAll)
		Q_PROPERTY(bool autoset_en READ autosetEnabled WRITE enableAutoset)

	public:
		explicit Oscilloscope_API(Oscilloscope *osc) :
			ApiObject(), osc(osc) {}
		~Oscilloscope_API() {}

		QVariantList getChannels();

		bool running() const;
		void run(bool en);

		bool hasCursors() const;
		void setCursors(bool en);

		bool autosetEnabled() const;
		void enableAutoset(bool en);

		bool hasMeasure() const;
		void setMeasure(bool en);

		bool measureAll() const;
		void setMeasureAll(bool en);

		bool hasCounter() const;
		void setCounter(bool en);

		bool hasStatistics() const;
		void setStatistics(bool en);

		bool horizontalCursors() const;
		void setHorizontalCursors(bool en);

		bool verticalCursors() const;
		void setVerticalCursors(bool en);

		double cursorV1() const;
		double cursorV2() const;
		double cursorH1() const;
		double cursorH2() const;
		void setCursorV1(double val);
		void setCursorV2(double val);
		void setCursorH1(double val);
		void setCursorH2(double val);

		bool autoTrigger() const;
		void setAutoTrigger(bool en);

		bool internalTrigger() const;
		void setInternalTrigger(bool en);

		bool externalTrigger() const;
		void setExternalTrigger(bool en);

		int triggerSource() const;
		void setTriggerSource(int idx);

		double getTriggerLevel() const;
		void setTriggerLevel(double level);

		double getTriggerHysteresis() const;
		void setTriggerHysteresis(double hyst);

		int internalCondition() const;
		void setInternalCondition(int cond);

		int externalCondition() const;
		void setExternalCondition(int cond);

		int internExtern() const;
		void setInternExtern(int option);

		QList<QString> getMathChannels() const;
		void setMathChannels(const QList<QString>& list);

		double getTimePos() const;
		void setTimePos(double pos);

		double getTimeBase() const;
		void setTimeBase(double base);

		QList<int> measureEn() const;
		void setMeasureEn(const QList<int>& list);

		QList<int> statisticEn() const;
		void setStatisticEn(const QList<int>& list);

		int getCurrentChannel() const;
		void setCurrentChannel(int chn_id);

		bool getFftEn() const;
		void setFftEn(bool en);

		bool getXyEn() const;
		void setXyEn(bool en);

		bool getExportAll() const;
		void setExportAll(bool en);

		int getCursorsPosition() const;
		void setCursorsPosition(int val);

		int getCursorsTransparency() const;
		void setCursorsTransparency(int val);

	private:
		Oscilloscope *osc;
	};

	class Channel_API : public ApiObject
	{
		Q_OBJECT

		Q_PROPERTY(bool enabled READ channelEn WRITE setChannelEn)

		Q_PROPERTY(double volts_per_div
				READ getVoltsPerDiv WRITE setVoltsPerDiv)

		Q_PROPERTY(double v_offset READ getVOffset WRITE setVOffset)

		Q_PROPERTY(double line_thickness
				READ getLineThickness WRITE setLineThickness)

		Q_PROPERTY(double probe_attenuation READ getProbeAttenuation
				WRITE setProbeAttenuation)
		Q_PROPERTY(bool ac_coupling READ getAcCoupling
				WRITE setAcCoupling)

		Q_PROPERTY(double period READ measured_period)
		Q_PROPERTY(double frequency READ measured_frequency)
		Q_PROPERTY(double min READ measured_min)
		Q_PROPERTY(double max READ measured_max)
		Q_PROPERTY(double peak_to_peak READ measured_peak_to_peak)
		Q_PROPERTY(double mean READ measured_mean)
		Q_PROPERTY(double cycle_mean READ measured_cycle_mean)
		Q_PROPERTY(double rms READ measured_rms)
		Q_PROPERTY(double cycle_rms READ measured_cycle_rms)
		Q_PROPERTY(double ac_rms READ measured_ac_rms)
		Q_PROPERTY(double area READ measured_area)
		Q_PROPERTY(double cycle_area READ measured_cycle_area)
		Q_PROPERTY(double low READ measured_low)
		Q_PROPERTY(double high READ measured_high)
		Q_PROPERTY(double amplitude READ measured_amplitude)
		Q_PROPERTY(double middle READ measured_middle)
		Q_PROPERTY(double pos_overshoot READ measured_pos_overshoot)
		Q_PROPERTY(double neg_overshoot READ measured_neg_overshoot)
		Q_PROPERTY(double rise READ measured_rise)
		Q_PROPERTY(double fall READ measured_fall)
		Q_PROPERTY(double pos_width READ measured_pos_width)
		Q_PROPERTY(double neg_width READ measured_neg_width)
		Q_PROPERTY(double pos_duty READ measured_pos_duty)
		Q_PROPERTY(double neg_duty READ measured_neg_duty)


	public:
		explicit Channel_API(Oscilloscope *osc) :
			ApiObject(), osc(osc) {}
		~Channel_API() {}

		bool channelEn() const;
		void setChannelEn(bool en);

		double getVoltsPerDiv() const;
		void setVoltsPerDiv(double val);

		double getVOffset() const;
		void setVOffset(double val);

		double getLineThickness() const;
		void setLineThickness(double val);

		double getProbeAttenuation() const;
		void setProbeAttenuation(double val);

		bool getAcCoupling() const;
		void setAcCoupling(bool val);

		double measured_period() const;
		double measured_frequency() const;
		double measured_min() const;
		double measured_max() const;
		double measured_peak_to_peak() const;
		double measured_mean() const;
		double measured_cycle_mean() const;
		double measured_rms() const;
		double measured_cycle_rms() const;
		double measured_ac_rms() const;
		double measured_area() const;
		double measured_cycle_area() const;
		double measured_low() const;
		double measured_high() const;
		double measured_amplitude() const;
		double measured_middle() const;
		double measured_pos_overshoot() const;
		double measured_neg_overshoot() const;
		double measured_rise() const;
		double measured_fall() const;
		double measured_pos_width() const;
		double measured_neg_width() const;
		double measured_pos_duty() const;
		double measured_neg_duty() const;

	private:
		Oscilloscope *osc;
	};
}

#endif /* M2K_OSCILLOSCOPE_H */
