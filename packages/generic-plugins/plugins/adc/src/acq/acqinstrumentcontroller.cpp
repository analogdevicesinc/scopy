/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "acqinstrumentcontroller.h"

#include "PlutoIIOSource.h"
#include "acqchannel.h"
#include "acqcurverepr.h"
#include "acqinstrument.h"
#include "acqplotmanager.h"

#include <core/acq_engine/AcquisitionEngine.h>
#include <core/acq_engine/Block.h>
#include <core/acq_engine/GenalyzerFFTProcessor.h>
#include <gui/instrumenttemplate.h>
#include <gui/style.h>

#include <QVBoxLayout>

using namespace scopy;
using namespace scopy::adc;

namespace {

// Pluto's RX default. The source doesn't read the rate back, so the FFT has to be
// told, and a wrong value only mislabels the frequency axis.
constexpr double kPlutoSampleRate = 2.4e6;

// Wraps a block's own settings widget in a menu page with an owner pill, which is
// what every rail row's page is made of.
QWidget *blockPage(InstrumentTemplate *it, scopy::acq::Block *block, const QString &title)
{
	QWidget *page = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(page);
	lay->setContentsMargins(0, 0, 0, 0);

	MenuSectionCollapseWidget *section = it->createMenuSection(title, SO_CH, page);
	section->add(block->settingsWidget(section));
	lay->addWidget(section);
	lay->addStretch();
	return page;
}

} // namespace

AcqInstrumentController::AcqInstrumentController(ToolMenuEntry *tme, QObject *parent)
	: QObject(parent)
	, m_tme(tme)
{
}

AcqInstrumentController::~AcqInstrumentController() { stop(); }

void AcqInstrumentController::init(iio_context *ctx)
{
	if(m_ui) {
		return;
	}

	// Parentless: the ToolMenuEntry takes it, and the plugin deletes it in
	// deleteInstrument(). Same ownership as the other controllers here.
	m_ui = new AcqInstrument(nullptr);

	setupBlocks(ctx);
	// After setupBlocks: the channels added here point at keys the blocks declare.
	setupPlots();

	// The tool menu's own run button and the instrument's stay in step. Guarded
	// by ToolMenuEntry::setRunning() only changing state, so the echo back is a
	// no-op rather than a loop.
	connect(m_tme, &ToolMenuEntry::runToggled, m_ui, [this](bool on) {
		if(on) {
			m_ui->run();
		} else {
			m_ui->stop();
		}
	});
	connect(m_ui, &AcqInstrument::started, this, [this]() { m_tme->setRunning(true); });
	connect(m_ui, &AcqInstrument::stopped, this, [this]() { m_tme->setRunning(false); });
}

void AcqInstrumentController::setupBlocks(iio_context *ctx)
{
	if(!ctx) {
		// PlutoIIOSource opens a real IIO buffer device; there is no simulated
		// stand-in here, so the instrument stays empty rather than pretending.
		return;
	}

	scopy::acq::AcquisitionEngine *engine = m_ui->engine();
	InstrumentTemplate            *it = m_ui->shell();

	m_plutoSrc = new sim::PlutoIIOSource(ctx, "pluto", "cf-ad9361-lpc", engine);
	m_plutoSrc->enableChannel("voltage0", true);
	m_plutoSrc->enableChannel("voltage1", true);
	engine->addSource(m_plutoSrc);

	// Two watched keys, so this is the complex path: I on voltage0, Q on voltage1,
	// nfft tied to the engine's buffer size.
	m_fftProc = new scopy::acq::GenalyzerFFTProcessor(scopy::acq::DataKey::raw("pluto", "voltage0"),
							  scopy::acq::DataKey::raw("pluto", "voltage1"),
							  scopy::acq::DataKey::withStage("pluto", "iq", "fft"),
							  scopy::acq::DataKey::withStage("pluto", "iq", "freq"),
							  static_cast<int>(engine->bufferSize()), kPlutoSampleRate,
							  GnWindowHann, engine);
	engine->addProcessor(m_fftProc);

	// Rail rows, so both blocks are reachable and the pipeline tab has something to
	// draw. The FFT is indented: it stacks on top of the source it reads.
	MenuSectionCollapseWidget *sources = it->addChannelGroup("Sources");
	it->addChannelRow(sources, "pluto", Style::getChannelColor(0), "pluto");
	it->addMenuPage("pluto", blockPage(it, m_plutoSrc, "PLUTO"));

	MenuSectionCollapseWidget *procs = it->addChannelGroup("Processors");
	it->addChannelRow(procs, "FFT", Style::getChannelColor(1), "fft", 1);
	it->addMenuPage("fft", blockPage(it, m_fftProc, "GENALYZER FFT"));
}

void AcqInstrumentController::setupPlots()
{
	InstrumentTemplate *it = m_ui->shell();

	m_plots = new AcqPlotManager(m_ui->store(), m_ui->engine(), it, m_ui);
	it->setCenterWidget(m_plots);

	// Direct, not queued: AcqInstrument already mirrors the engine's worker-thread
	// signals onto the GUI thread, and these are its GUI-thread re-emissions.
	connect(m_ui, &AcqInstrument::cycleComplete, m_plots, &AcqPlotManager::onCycleComplete);
	connect(m_ui, &AcqInstrument::started, m_plots, &AcqPlotManager::onStarted);
	connect(m_ui, &AcqInstrument::stopped, m_plots, &AcqPlotManager::onStopped);
	// Depth is ceil(plotSize / bufferSize), so every channel's claim depends on a
	// number only the instrument's buffer control knows about.
	connect(m_ui, &AcqInstrument::bufferSizeChanged, m_plots, &AcqPlotManager::onBufferSizeChanged);

	// How much history is drawn, independent of how much arrives per cycle: depth is
	// ceil(plotSize / bufferSize), so a window wider than the buffer is stitched from
	// several chunks. Live while running — it only changes claims and the X range.
	gui::MenuSpinbox *plotSpin = new gui::MenuSpinbox("Plot window", m_plots->plotSize(), "samples", 16, 1 << 20,
							  true, false, false, it);
	plotSpin->setIncrementMode(gui::MenuSpinbox::IS_POW2);
	it->addEngineControl(plotSpin);
	connect(plotSpin, &gui::MenuSpinbox::valueChanged, m_plots,
		[this](double v) { m_plots->setPlotSize(static_cast<int>(v)); });

	if(!m_plutoSrc || !m_fftProc) {
		// setupBlocks() bailed for want of a context. The manager still exists — an
		// empty plot beats no center widget — but there are no keys to point at.
		return;
	}

	// Explicit kinds throughout: nothing has been written yet, so DataStore::typeOf()
	// is nullopt and ReprKind::Auto could only guess.
	m_plots->addChannel(scopy::acq::DataKey::raw("pluto", "voltage0"), "I", Style::getChannelColor(0),
			    AcqPlotManager::ReprKind::Curve);
	m_plots->addChannel(scopy::acq::DataKey::raw("pluto", "voltage1"), "Q", Style::getChannelColor(1),
			    AcqPlotManager::ReprKind::Curve);

	// The FFT is the one channel with a real X stream: the processor writes the bin
	// frequencies to a second key, so the curve is drawn against Hz rather than the
	// shared sample-index ramp.
	AcqChannel *fft = m_plots->addChannel(m_fftProc->outputKey(), "FFT", Style::getChannelColor(2),
					      AcqPlotManager::ReprKind::Curve);
	if(fft) {
		static_cast<CurveRepr *>(fft->repr())->setXKey(m_fftProc->freqKey());
	}
}

void AcqInstrumentController::stop()
{
	if(m_ui) {
		m_ui->stop();
	}
}

AcqInstrument *AcqInstrumentController::ui() const { return m_ui; }

#include "moc_acqinstrumentcontroller.cpp"
