#include "fftplotplugin.h"
#include "logging_categories.h"
#include "scopyExceptionHandler.h"
#include <newinstrument.hpp>

using namespace adiscope;
using namespace gui;

FftPlotPlugin::FftPlotPlugin(QWidget *parent, gui::ToolView* toolView, ChannelManager* chManager, bool dockable): BasePlugin(parent, dockable), chManager(chManager), toolView(toolView), fft_plot(nullptr)
{
	init();
}
FftPlotPlugin::~FftPlotPlugin()
{

}

void FftPlotPlugin::init()
{
	qDebug(CAT_FFTPLOT_PLUGIN()) << "init()";

	double start = 0, stop = 50000000;

	fft_plot = new FftDisplayPlot(0, parent);
	fft_plot->disableLegend();
	fft_plot->setXaxisMouseGesturesEnabled(false);

	fft_plot->setZoomerEnabled();
	fft_plot->setAxisVisible(QwtAxis::XBottom, false);
	fft_plot->setAxisVisible(QwtAxis::YLeft, false);
	fft_plot->setUsingLeftAxisScales(false);
	fft_plot->enableXaxisLabels();
	fft_plot->enableYaxisLabels();
	//	setYAxisUnit(ui->cmb_units->currentText());
	fft_plot->setBtmHorAxisUnit("Hz");

	fft_plot->setStartStop(start, stop);
	fft_plot->setAxisScale(QwtAxis::XBottom, start, stop);
	fft_plot->replot();
	fft_plot->bottomHandlesArea()->repaint();
	setSampleRate(2 * stop);

	auto instrument = dynamic_cast<NewInstrument*>(parent);
	if (instrument != nullptr) {
		instrument->addPlot(fft_plot);
	}

	if (dockable) {
		toolView->addDockableTabbedWidget(fft_plot->getPlotwithElements(), "FFT Plot");
	}
	else {
		toolView->addFixedCentralWidget(fft_plot->getPlotwithElements(), 0, 0, 1, 1);
	}

	fft_plot->setResolutionBW(24414.0625);

	connectSignals();
}

FftDisplayPlot *FftPlotPlugin::getPlot()
{
	return fft_plot;
}

void FftPlotPlugin::setSampleRate(double sr)
{
	sample_rate_divider = (int)(m_max_sample_rate / sr);
	double new_sr = m_max_sample_rate / sample_rate_divider;

	if (new_sr == sample_rate) {
		return;
	}

	sample_rate = new_sr;
	fft_plot->setSampleRate(sr, 1, "");
//	if (isIioManagerStarted()) {
//		stop_blockchain_flow();

//		if (m_m2k_analogin) {
//			try {
//				m_m2k_analogin->setOversamplingRatio(sample_rate_divider);
//			} catch (libm2k::m2k_exception &e) {
//				HANDLE_EXCEPTION(e);
//				qDebug(CAT_SPECTRUM_ANALYZER) << "Can't write oversampling ratio: " << e.what();
//			}
//		} else {
			try {
				for (unsigned int i = 0; i < chManager->getChannelsCount(); i++) {
					m_generic_analogin->setSampleRate(i, sr);
				}
			} catch (libm2k::m2k_exception &e) {
				HANDLE_EXCEPTION(e);
				qDebug(CAT_SPECTRUM_ANALYZER) << "Can't write sampling frequency: " << e.what();
			}
//		}

		fft_plot->presetSampleRate(new_sr);
		fft_plot->resetAverageHistory();
//		fft_sink->set_samp_rate(new_sr);

//		sample_timer->stop();
//		m_time_start = std::chrono::system_clock::now();

//		start_blockchain_flow();
//		sample_timer->start(TIMER_TIMEOUT_MS);
//	}
}

void FftPlotPlugin::connectSignals()
{
	connect(parent, SIGNAL(selectedChannelChanged(int)),
		fft_plot, SLOT(setSelectedChannel(int)));
}
