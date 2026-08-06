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
#include "acqinstrument.h"
#include "acqplotmanager.h"

#include <core/acq_engine/AcquisitionEngine.h>
#include <core/acq_engine/Block.h>
#include <core/acq_engine/GenalyzerFFTProcessor.h>
#include <gui/cursorcontroller.h>
#include <gui/instrumenttemplate.h>
#include <gui/style.h>

#include <QPushButton>
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

	// Cursors over row 0. The controller and its settings page are created here rather
	// than lazily on the first click because the page has to be in the stack before the
	// button can show it.
	QString cursorPageId;
	if(CursorController *cursors = m_plots->cursors(&cursorPageId)) {
		QPushButton *cursorBtn = new QPushButton(tr("Cursors"), it);
		cursorBtn->setCheckable(true);
		// The same styling the shell gives its own Debug button, which is the other
		// bottom-rail toggle — both are view state rather than engine controls, which is
		// why they sit here and not in the top rail with Run.
		Style::setStyle(cursorBtn, style::properties::button::blueGrayButton);
		Style::setStyle(cursorBtn, style::properties::label::menuMedium);
		it->addToBottomRail(cursorBtn, TTA_RIGHT);
		connect(cursorBtn, &QPushButton::toggled, cursors, &CursorController::setVisible);
		// Showing the page on toggle-on only: unchecking hides the cursors, and yanking
		// the reader out of a page they may have navigated to on purpose would be worse
		// than leaving it up.
		connect(cursorBtn, &QPushButton::toggled, it, [it, cursorPageId](bool on) {
			if(on) {
				it->showMenuPage(cursorPageId);
			}
		});
	}

	// Every plot channel is created by the reader from here, and none at composition
	// time: the pipeline publishes several keys per run and which of them is worth
	// looking at — and as what — is not something this function can know. The plot
	// opens empty.
	//
	// Two things the picker cannot infer are supplied by the manager instead, so a
	// hand-made channel is configured exactly like a composed one would have been:
	// the measurement timeline, and the X stream an FFT curve is drawn against.
	if(m_fftProc) {
		// The timeline every horizontal measurement is divided by. From the FFT
		// processor because that is where the rate is configured — the source does not
		// publish one. Without it period and frequency come out in samples, which is
		// not wrong so much as unreadable.
		m_plots->setSampleRate(m_fftProc->sampleRate());
		// The FFT's bin frequencies. The processor writes them to a second key, which
		// is what lets a magnitude curve be drawn against Hz rather than the shared
		// sample-index ramp without the channel knowing anything about FFTs.
		m_plots->setXKeyFor(m_fftProc->outputKey(), m_fftProc->freqKey());
	}

	// Last, so the list it builds is populated from a store that already has whatever
	// setupBlocks() registered. Useful even when setupBlocks() bailed for want of a
	// context: the page exists, it is simply empty until a run writes something.
	m_plots->createKeyPickerPage();
}

void AcqInstrumentController::stop()
{
	if(m_ui) {
		m_ui->stop();
	}
}

AcqInstrument *AcqInstrumentController::ui() const { return m_ui; }

#include "moc_acqinstrumentcontroller.cpp"
