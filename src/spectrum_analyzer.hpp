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

#include "iio_manager.hpp"
#include "scope_sink_f.h"
#include "fft_block.hpp"
#include "FftDisplayPlot.h"
#include "osc_adc.h"

#include <QWidget>

extern "C" {
	struct iio_buffer;
	struct iio_channel;
	struct iio_context;
	struct iio_device;
}

namespace Ui {
	class SpectrumAnalyzer;
	class Channel;
}

class PositionSpinButton;

namespace adiscope {
	class SpectrumChannel;
	class Filter;
}

class QPushButton;
class QButtonGroup;

namespace adiscope {

class SpectrumAnalyzer: public QWidget
{
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
		QWidget *parent = 0);
	~SpectrumAnalyzer();

private Q_SLOTS:
	void on_btnToolSettings_toggled(bool checked);
	void on_btnSettings_pressed();
	void on_btnSweep_toggled(bool checked);
	void on_comboBox_type_currentIndexChanged(const QString&);
	void on_comboBox_window_currentIndexChanged(const QString&);
	void on_spinBox_averaging_valueChanged(int);
	void runStopToggled(bool);
	void onChannelSettingsToggled(bool);
	void onStartStopChanged();
	void onCenterSpanChanged();

private:
	void build_gnuradio_block_chain();
	void build_gnuradio_block_chain_no_ctx();
	void writeAllSettingsToHardware();

private:
	Ui::SpectrumAnalyzer *ui;

	PositionSpinButton *ui_startFreq;
	PositionSpinButton *ui_stopFreq;
	PositionSpinButton *ui_centerFreq;
	PositionSpinButton *ui_span;

	QPushButton *menuRunButton;
	QButtonGroup *settings_group;
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

	gr::top_block_sptr top_block;

	static std::vector<std::pair<QString,
		FftDisplayPlot::AverageType>> avg_types;
	static std::vector<std::pair<QString, FftWinType>> win_types;
};

class SpectrumChannel: public QObject
{
Q_OBJECT

public:
	boost::shared_ptr<fft_block> fft_block;
	QWidget *m_widget;
	Ui::Channel *m_ui;

	SpectrumChannel(int id, const QString& name, FftDisplayPlot *plot);

	int id() const { return m_id; }
	QString name() const { return m_name; }

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

Q_SIGNALS:
	void enabled(bool);
	void selected(bool);
	void settingsToggled(bool);

private Q_SLOTS:
	void onEnableBoxToggled(bool);
	void onNameButtonToggled(bool);
	void onSettingsBtnToggled(bool);

private:
	int m_id;
	QString m_name;
	float m_line_width;
	QColor m_color;
	uint m_averaging;
	FftDisplayPlot::AverageType m_avg_type;
	SpectrumAnalyzer::FftWinType m_fft_win;
	FftDisplayPlot *m_plot;

	static std::vector<float> build_win(SpectrumAnalyzer::FftWinType type,
		int ntaps);
};

} // namespace adiscope

#endif // SPECTRUM_ANALYZER_HPP
