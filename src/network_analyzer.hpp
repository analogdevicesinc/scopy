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

#ifndef SCOPY_NETWORK_ANALYZER_HPP
#define SCOPY_NETWORK_ANALYZER_HPP

#include "spinbox_a.hpp"
#include "apiObject.hpp"
#include "iio_manager.hpp"
#include "signal_sample.hpp"
#include "tool.hpp"
#include "dbgraph.hpp"
#include "handles_area.hpp"
#include <QtConcurrentRun>
#include "customPushButton.hpp"
#include "scroll_filter.hpp"


extern "C" {
	struct iio_buffer;
	struct iio_channel;
	struct iio_context;
	struct iio_device;
}

namespace Ui {
	class NetworkAnalyzer;
}

class QPushButton;
class QJSEngine;

namespace adiscope {
	class NetworkAnalyzer_API;
	class Filter;
	class GenericAdc;

	class NetworkAnalyzer : public Tool
	{
		friend class NetworkAnalyzer_API;
		friend class ToolLauncher_API;

		Q_OBJECT

	public:
		explicit NetworkAnalyzer(struct iio_context *ctx, Filter *filt,
				std::shared_ptr<GenericAdc> &adc_dev,
				QPushButton *runButton, QJSEngine *engine,
				ToolLauncher *parent);
		~NetworkAnalyzer();

	private:
		Ui::NetworkAnalyzer *ui;
		struct iio_channel *dac1, *dac2, *amp1, *amp2;
		struct iio_device *adc;
		std::shared_ptr<GenericAdc> adc_dev;
		boost::shared_ptr<iio_manager> iio;
        dBgraph m_dBgraph;
        dBgraph m_phaseGraph;

        PlotLineHandleH *d_hCursorHandle1;
        PlotLineHandleH *d_hCursorHandle2;
        bool d_cursorsEnabled;

        ScaleSpinButton *samplesCount;
        ScaleSpinButton *minFreq;
        ScaleSpinButton *maxFreq;
        ScaleSpinButton *amplitude;
        PositionSpinButton *offset;
        PositionSpinButton *magMax;
        PositionSpinButton *magMin;
        PositionSpinButton *phaseMax;
        PositionSpinButton *phaseMin;

        HorizHandlesArea *d_bottomHandlesArea;

        QQueue<QPair<CustomPushButton *, bool>> menuButtonActions;

        MouseWheelWidgetGuard *wheelEventGuard;

		QFuture<void> thd;
		bool stop;

		void run();

		static size_t get_sin_samples_count(
				const struct iio_device *dev,
				unsigned long rate,
				double frequency);

		static struct iio_buffer * generateSinWave(
				const struct iio_device *dev,
				double frequency,
				double amplitude,
				double offset,
				unsigned long rate,
				size_t samples_count);

		static unsigned long get_best_sample_rate(
				const struct iio_device *dev,
				double frequency);
		void configHwForNetworkAnalyzing();

		void triggerRightMenuToggle(CustomPushButton *btn, bool checked);
		void toggleRightMenu(CustomPushButton *btn, bool checked);

	private Q_SLOTS:
		void startStop(bool start);
		void updateNumSamples();

        void toggleCursors(bool en);
        void onVbar1PixelPosChanged(int pos);
        void onVbar2PixelPosChanged(int pos);
        void readPreferences();
        void onGraphIndexChanged(int);
        void on_btnExport_clicked();
        void rightMenuFinished(bool opened);

        public Q_SLOTS:

        void showEvent(QShowEvent *event);

	Q_SIGNALS:
		void sweepDone();
		void showTool();
	};
} /* namespace adiscope */

#endif /* SCOPY_NETWORK_ANALYZER_HPP */
