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

#include "acqinstrument.h"

#include "datastoreviewer.h"
#include "logview.h"

#include <core/acq_engine/acquisitionengine.h>
#include <core/acq_engine/datastore.h>
#include <core/acq_engine/pipelineinspector.h>
#include <core/decoder/decoderlogger.h>
#include <gui/instrumenttemplate.h>
#include <gui/widgets/menucombo.h>
#include <gui/widgets/menuspinbox.h>
// InstrumentTemplate only forward-declares its buttons.
#include <gui/widgets/toolbuttons.h>

#include <QComboBox>
#include <QDateTime>
#include <QSignalBlocker>
#include <QVBoxLayout>

using namespace scopy;
using namespace scopy::adc;

namespace {
// Defaults, so an owner that never touches them still gets a usable engine.
constexpr std::size_t kDefaultBufferSize = 1024;
constexpr unsigned int kDefaultMaxFPS = 30;
} // namespace

AcqInstrument::AcqInstrument(QWidget *parent)
	: QWidget(parent)
{
	QVBoxLayout *lay = new QVBoxLayout(this);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(0);

	m_it = new InstrumentTemplate(this);
	lay->addWidget(m_it);

	// Engine first: the top rail's controls and every debug tab take it as a
	// constructor argument.
	setupEngine();
	setupTopRail();
	setupDebugTabs();
}

AcqInstrument::~AcqInstrument()
{
	// The engine joins its worker in its own destructor, but the debug tabs are
	// connected to it and are destroyed after this body runs. Stopping here means
	// no cycle can be in flight while children are going away.
	if(m_engine) {
		m_engine->stop();
	}
}

void AcqInstrument::setupEngine()
{
	m_store = new scopy::acq::DataStore(this);
	m_engine = new scopy::acq::AcquisitionEngine(m_store, this);
	m_engine->setBufferSize(kDefaultBufferSize);
	m_engine->setMaxFPS(kDefaultMaxFPS);
	m_engine->setMode(scopy::acq::AcquisitionEngine::Mode::Continuous);

	// The engine's Warning default for report severity stands: blocks on the per-cycle
	// path emit Info every frame, and the Logs tab's combo raises it on request instead
	// — see setupDebugTabs().

	// Built up front so a decoder backend added later has a sink to log into, and
	// so the tab isn't conditional on anything decoding. Not forwarded to the
	// engine: per-annotation decoder chatter would bury the acquisition log.
	m_decoderLogger = new scopy::decoder::DecoderLogger(this);
	m_decoderLogger->setEngine(m_engine);
	m_decoderLogger->setForwardToEngine(false);

	// Every engine signal crosses from the worker thread.
	connect(m_engine, &scopy::acq::AcquisitionEngine::started, this, &AcqInstrument::onStarted,
		Qt::QueuedConnection);
	connect(m_engine, &scopy::acq::AcquisitionEngine::stopped, this, &AcqInstrument::onStopped,
		Qt::QueuedConnection);
	// A fault stop is a stop as far as the UI is concerned; the reason for it is
	// already in the log tab.
	connect(m_engine, &scopy::acq::AcquisitionEngine::forceStopped, this, &AcqInstrument::onStopped,
		Qt::QueuedConnection);
	connect(m_engine, &scopy::acq::AcquisitionEngine::cycleComplete, this, &AcqInstrument::onCycleComplete,
		Qt::QueuedConnection);
}

void AcqInstrument::setupTopRail()
{
	m_modeCombo = new MenuCombo("Mode", m_it);
	// Item order matches AcquisitionEngine::Mode, so the index *is* the mode.
	m_modeCombo->combo()->addItems({"Continuous", "Triggered"});
	m_modeCombo->combo()->setCurrentIndex(static_cast<int>(m_engine->mode()));
	m_it->addEngineControl(m_modeCombo);

	m_bufferSpin = new gui::MenuSpinbox("Buffer size", kDefaultBufferSize, "samples", 16, 1 << 20, true, false,
					    false, m_it);
	m_bufferSpin->setIncrementMode(gui::MenuSpinbox::IS_POW2);
	m_it->addEngineControl(m_bufferSpin);

	connect(m_modeCombo->combo(), &QComboBox::currentIndexChanged, this,
		[this](int i) { m_engine->setMode(static_cast<scopy::acq::AcquisitionEngine::Mode>(i)); });
	connect(m_bufferSpin, &gui::MenuSpinbox::valueChanged, this, [this](double v) {
		// Tell the engine and nothing else: depth claims are in samples and the
		// DataStore follows the new chunk length on the next push.
		m_engine->setBufferSize(static_cast<std::size_t>(v));
	});

	// The shell's target-fps field drives the engine's own ceiling. Seeded from the
	// engine so the field shows what is actually in force, and left enabled while
	// running: setMaxFPS is atomic and the worker re-reads it every cycle, unlike
	// mode and buffer size.
	m_it->setTargetFps(static_cast<int>(m_engine->maxFPS()));
	connect(m_it, &InstrumentTemplate::targetFpsChanged, this,
		[this](int fps) { m_engine->setMaxFPS(static_cast<unsigned int>(fps)); });

	// Toggle rather than click: the button reflects run state, and onStarted /
	// onStopped are what actually set it, so the engine stays the authority.
	connect(m_it->runBtn(), &QAbstractButton::toggled, this, [this](bool checked) {
		if(checked) {
			run();
		} else {
			stop();
		}
	});
	connect(m_it->singleBtn(), &QAbstractButton::toggled, this, [this](bool checked) {
		if(checked) {
			single();
		}
	});
}

void AcqInstrument::setupDebugTabs()
{
	// All live with no blocks registered: the pipeline tree and the store viewer are
	// derived from the engine and the store, so they simply show nothing yet.
	m_pipeline = new scopy::acq::PipelineInspector(m_engine, m_store, m_it);
	m_it->addDebugTab(m_pipeline, "Pipeline");

	// The engine as well as the store: a stream's descriptor is owned by the block
	// that produces it, so the store alone cannot say what a key means.
	m_storeViewer = new DataStoreViewer(m_store, m_engine, m_it);
	m_it->addDebugTab(m_storeViewer, "DataStore");

	m_logView = new LogView("No messages.", m_it);
	m_it->addDebugTab(m_logView, "Logs");

	m_decoderLogView = new LogView("No decoder messages.", m_it);
	m_it->addDebugTab(m_decoderLogView, "Decoder logs");

	// Block reports arrive on the worker thread; AcquisitionError::Severity and
	// decoder::LogLevel share the Info/Warning/Critical ordering LogView takes.
	connect(m_engine, &scopy::acq::AcquisitionEngine::error, m_logView, &LogView::append, Qt::QueuedConnection);
	connect(m_decoderLogger, &scopy::decoder::DecoderLogger::messageLogged, m_decoderLogView, &LogView::append,
		Qt::QueuedConnection);

	// Each combo gates its own producer. Filtering in the widget alone still pays
	// to build, copy and queue every per-frame Info line, so the only way Info is
	// affordable in continuous mode is for it to stay unemitted until asked for.
	connect(m_logView, &LogView::minSeverityChanged, this, [this](int sev) {
		m_engine->setMinReportSeverity(static_cast<scopy::acq::AcquisitionError::Severity>(sev));
	});
	connect(m_decoderLogView, &LogView::minSeverityChanged, this,
		[this](int sev) { m_decoderLogger->setMinLevel(static_cast<scopy::decoder::LogLevel>(sev)); });

	// Start both combos where the producers actually are, so the filter never
	// claims to be showing Info while the source is dropping it.
	m_logView->setMinSeverity(static_cast<int>(m_engine->minReportSeverity()));
	m_decoderLogView->setMinSeverity(static_cast<int>(m_decoderLogger->minLevel()));

	// The tree is derived from the block lists, so it has to be told when they
	// change — nothing else would notice a block being registered.
	connect(m_engine, &scopy::acq::AcquisitionEngine::blocksChanged, m_pipeline,
		&scopy::acq::PipelineInspector::rebuild, Qt::QueuedConnection);
}

void AcqInstrument::run()
{
	if(m_engine->isRunning()) {
		return;
	}
	// A fresh run must not inherit the previous one's chunks: depth claims stay,
	// so a consumer's window would otherwise splice old data onto new.
	m_store->clear();
	m_engine->run();
}

void AcqInstrument::single()
{
	if(m_engine->isRunning()) {
		return;
	}
	m_store->clear();
	// Before the start, so a listener gets to arm its one-shot stop — and to set
	// m_singleWaitsForStop, read just below — before the first cycle can fire.
	Q_EMIT singleRequested();
	if(m_singleWaitsForStop) {
		m_engine->run();
	} else {
		m_engine->single();
	}
}

void AcqInstrument::stop() { m_engine->stop(); }

void AcqInstrument::setRunning(bool running)
{
	// Blocked so reflecting the engine's state doesn't re-enter run()/stop().
	{
		QSignalBlocker b(m_it->runBtn());
		m_it->runBtn()->setChecked(running);
	}
	{
		QSignalBlocker b(m_it->singleBtn());
		m_it->singleBtn()->setChecked(false);
	}
	m_it->singleBtn()->setEnabled(!running);

	// These write engine fields the worker reads between cycles. Locking them
	// while running is the cheap alternative to a restart-pending mechanism.
	m_modeCombo->setEnabled(!running);
	m_bufferSpin->setEnabled(!running);
}

void AcqInstrument::onStarted()
{
	setRunning(true);
	Q_EMIT started();
}

void AcqInstrument::onStopped()
{
	setRunning(false);
	// One last refresh: the run's final cycle may have landed after the last
	// rate-limited cycleComplete, and while stopped nothing else will update the
	// debug tabs.
	m_storeViewer->refresh();
	m_pipeline->refreshStatus();
	Q_EMIT stopped();
}

void AcqInstrument::onCycleComplete()
{
	// cycleComplete is already rate-limited to maxFPS by the engine, so this is
	// the frame clock as well as the data notification.
	m_it->setFps(QDateTime::currentMSecsSinceEpoch());

	// Only while the popup is open: both of these walk the store or the block
	// list, and doing that at cycle rate for a panel nobody is looking at is the
	// most expensive thing the GUI thread would be doing.
	if(m_it->debugBtn()->isChecked()) {
		m_storeViewer->refresh();
		m_pipeline->refreshStatus();
	}

	Q_EMIT cycleComplete();
}

#include "moc_acqinstrument.cpp"
