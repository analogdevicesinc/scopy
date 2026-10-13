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

	// Engine first: the top rail and the debug tabs take it.
	setupEngine();
	setupTopRail();
	setupDebugTabs();
}

AcqInstrument::~AcqInstrument()
{
	// Before the children go: the debug tabs are connected to the engine.
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

	connect(m_engine, &scopy::acq::AcquisitionEngine::started, this, &AcqInstrument::onStarted,
		Qt::QueuedConnection);
	connect(m_engine, &scopy::acq::AcquisitionEngine::stopped, this, &AcqInstrument::onStopped,
		Qt::QueuedConnection);
	// A fault stop is a stop for the UI; the reason is in the log tab.
	connect(m_engine, &scopy::acq::AcquisitionEngine::forceStopped, this, &AcqInstrument::onStopped,
		Qt::QueuedConnection);
	connect(m_engine, &scopy::acq::AcquisitionEngine::cycleComplete, this, &AcqInstrument::onCycleComplete,
		Qt::QueuedConnection);
}

void AcqInstrument::setupTopRail()
{
	m_modeCombo = new MenuCombo("Mode", m_it);
	// Item order matches AcquisitionEngine::Mode, so the index is the mode.
	m_modeCombo->combo()->addItems({"Continuous", "Triggered"});
	m_modeCombo->combo()->setCurrentIndex(static_cast<int>(m_engine->mode()));
	m_it->addEngineControl(m_modeCombo);

	m_bufferSpin = new gui::MenuSpinbox("Buffer size", kDefaultBufferSize, "samples", 16, 1 << 20, true, false,
					    false, m_it);
	m_bufferSpin->setIncrementMode(gui::MenuSpinbox::IS_POW2);
	m_it->addEngineControl(m_bufferSpin);

	connect(m_modeCombo->combo(), &QComboBox::currentIndexChanged, this,
		[this](int i) { m_engine->setMode(static_cast<scopy::acq::AcquisitionEngine::Mode>(i)); });
	// Depth claims are in samples, so the store follows the new chunk length by itself.
	connect(m_bufferSpin, &gui::MenuSpinbox::valueChanged, this,
		[this](double v) { m_engine->setBufferSize(static_cast<std::size_t>(v)); });

	// Left enabled while running: the worker re-reads the fps ceiling every cycle.
	m_it->setTargetFps(static_cast<int>(m_engine->maxFPS()));
	connect(m_it, &InstrumentTemplate::targetFpsChanged, this,
		[this](int fps) { m_engine->setMaxFPS(static_cast<unsigned int>(fps)); });

	connect(m_it->runBtn(), &QAbstractButton::toggled, this, [this](bool checked) { checked ? run() : stop(); });
}

void AcqInstrument::setupDebugTabs()
{
	m_pipeline = new scopy::acq::PipelineInspector(m_engine, m_store, m_it);
	m_it->addDebugTab(m_pipeline, "Pipeline");

	m_storeViewer = new DataStoreViewer(m_store, m_engine, m_it);
	m_it->addDebugTab(m_storeViewer, "DataStore");

	m_logView = new LogView("No messages.", m_it);
	m_it->addDebugTab(m_logView, "Logs");

	connect(m_engine, &scopy::acq::AcquisitionEngine::error, m_logView, &LogView::append, Qt::QueuedConnection);
	// The combo gates the producer, not just the view: unemitted Info costs nothing.
	connect(m_logView, &LogView::minSeverityChanged, this, [this](int sev) {
		m_engine->setMinReportSeverity(static_cast<scopy::acq::AcquisitionError::Severity>(sev));
	});
	m_logView->setMinSeverity(static_cast<int>(m_engine->minReportSeverity()));

	connect(m_engine, &scopy::acq::AcquisitionEngine::blocksChanged, m_pipeline,
		&scopy::acq::PipelineInspector::rebuild, Qt::QueuedConnection);
}

void AcqInstrument::start(unsigned int cycles)
{
	if(m_engine->isRunning()) {
		return;
	}
	// Claims stay, chunks go: otherwise a window would splice the last run onto this one.
	m_store->clear();
	if(cycles == 0) {
		m_engine->run();
	} else {
		m_engine->single(cycles);
	}
}

void AcqInstrument::run() { start(0); }

void AcqInstrument::single() { start(1); }

void AcqInstrument::stop() { m_engine->stop(); }

void AcqInstrument::setRunning(bool running)
{
	// Blocked so reflecting the state does not re-enter run()/stop().
	{
		QSignalBlocker b(m_it->runBtn());
		m_it->runBtn()->setChecked(running);
	}
	{
		QSignalBlocker b(m_it->singleBtn());
		m_it->singleBtn()->setChecked(false);
	}
	m_it->singleBtn()->setEnabled(!running);
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
	// The last cycle may have landed after the last refresh, and nothing refreshes while
	// stopped.
	m_storeViewer->refresh();
	m_pipeline->refreshStatus();
	Q_EMIT stopped();
}

void AcqInstrument::onCycleComplete()
{
	// cycleComplete is rate-limited by the engine, so it is also the frame clock.
	m_it->setFps(QDateTime::currentMSecsSinceEpoch());

	// Only while the popup is open: both walk the store or the block list.
	if(m_it->debugBtn()->isChecked()) {
		m_storeViewer->refresh();
		m_pipeline->refreshStatus();
	}

	Q_EMIT cycleComplete();
}

#include "moc_acqinstrument.cpp"
