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
#include <gnuradio/blocks/complex_to_mag.h>

/* Qt includes */
#include <QPair>
#include <QPushButton>
#include <QVector>
#include <QWidget>
#include <QButtonGroup>

/* Local includes */
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
#include "osc_adc.h"

namespace Ui {
	class Channel;
	class Oscilloscope;
	class OscGeneralSettings;
	class ChannelSettings;
}

namespace adiscope {
	class Oscilloscope : public QWidget
	{
	    Q_OBJECT

	public:
		explicit Oscilloscope(struct iio_context *ctx,
				Filter *filt, QPushButton *runButton,
				float gain_ch1, float gain_ch2,
				QWidget *parent = 0);
		~Oscilloscope();

	signals:
		void triggerALevelChanged(double);
		void triggerBLevelChanged(double);
		void triggerDelayChanged(double);
		void selectedChannelChanged(int);

	private slots:
		void on_actionClose_triggered();
		void onCursorsToggled(bool on);

		void onFFT_view_toggled(bool visible);
		void onHistogram_view_toggled(bool visible);
		void onXY_view_toggled(bool visible);

		void onTriggerSettingsDelayChanged(double);
		void onTimeTriggerDelayChanged(double);

		void updateTriggerSpinbox(double);
		void updatePlotHorizDelay(double);

		void onVertScaleValueChanged(double value);
		void onHorizScaleValueChanged(double value);
		void onVertOffsetValueChanged(double value);
		void onHorizOffsetValueChanged(double value);

		void onChannelOffsetChanged(double value);

		void on_comboBox_currentIndexChanged(const QString &arg1);

		void channel_box_toggled(bool);
		void channel_name_checked(bool);

		void rightMenuFinished(bool opened);
		void toggleRightMenu();

		void runStopToggled(bool);

		void del_math_channel();

	private:
		OscADC adc;
		unsigned int nb_channels, nb_math_channels;
		QList<double> sampling_rates;
		double active_sample_rate;
		Ui::Oscilloscope *ui;
		Ui::OscGeneralSettings *gsettings_ui;
		Ui::ChannelSettings *ch_ui;
		adiscope::TriggerSettings trigger_settings;
		CapturePlot plot;
		FftDisplayPlot fft_plot;
		ConstellationDisplayPlot xy_plot;
		HistogramDisplayPlot hist_plot;

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
		iio_manager::port_id *xy_ids;

		ScaleSpinButton *timeBase;
		PositionSpinButton *timePosition;
		ScaleSpinButton *voltsPerDiv;
		PositionSpinButton *voltsPosition;

		bool fft_is_visible, hist_is_visible, xy_is_visible;

		int fft_size;

		double triggerDelay;
		int selectedChannel;

		NumberSeries voltsPerDivList;
		NumberSeries secPerDivList;

		bool menuOpened;
		unsigned int current_channel;
		unsigned int math_chn_counter;

		QButtonGroup *settings_group;
		QButtonGroup *channels_group; // selected state of each channel

		QPushButton *active_settings_btn;
		QPushButton *menuRunButton;

		static const unsigned long maxBufferSize;

		void comboBoxUpdateToValue(QComboBox *box, double value, std::vector<double>list);

		void trigger_delay_write_hardware(double value);

		void settings_panel_update(int id);
		void settings_panel_size_adjust();
		void toggleRightMenu(QPushButton *btn);
		void create_math_panel();
		void add_math_channel(const std::string& function);
		unsigned int find_curve_number();
		QWidget *channelWidgetAtId(int id);
		void update_chn_settings_panel(int id, QWidget *chn_widget = NULL);

		void updateRunButton(bool ch_enabled);

		double pickSampleRateFor(double timeSpanSecs,
					double desiredBufferSize);
	};
}

#endif /* M2K_OSCILLOSCOPE_H */
