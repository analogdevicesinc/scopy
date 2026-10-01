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

#include "acqinstrument.h"
#include "acqaxis.h"
#include "acqplot.h"
#include "acqplotmanager.h"
#include "filesourceblock.h"
#include "sourceregistry.h"
// Under src/sim/ but not sim-specific: the manager's plot and overlay are optional and the
// panel has no plot dependency at all.
#include "../sim/DecoderManager.h"
#include "../sim/DecoderPanel.h"
#include "../sim/PipelineInspector.h"

#include <core/acq_engine/AcquisitionEngine.h>
#include <core/acq_engine/Block.h>
#include <core/acq_engine/DataStore.h>
#include <core/acq_engine/GenalyzerFFTProcessor.h>
#include <core/acq_engine/SourceBlock.h>
#include <core/acq_engine/TriggerBinder.h>
#include <core/acq_engine/TriggerMarker.h>
#include <core/acq_engine/TriggerProcessor.h>
#include <core/acq_engine/TriggerProcessorWidget.h>
#include <core/decoder/SigrokCliBackendFactory.h>
#include <core/decoder/SigrokCliCatalog.h>
#include <gui/axishandle.h>
#include <gui/instrumenttemplate.h>
#include <gui/plotaxis.h>
#include <gui/plotaxishandle.h>
#include <gui/plotwidget.h>
#include <gui/style.h>
#include <gui/style_attributes.h>
#include <gui/widgets/genalyzerpanel.h>

#include <qwt_axis.h>
#include <qwt_plot.h>

#include <algorithm>

#include <QMap>
#include <QPointer>
#include <QVBoxLayout>

#include <memory>
#include <vector>

using namespace scopy;
using namespace scopy::adc;

namespace {

// A rail row's right-menu page: any widget inside a section with an owner pill.
QWidget *widgetPage(InstrumentTemplate *it, QWidget *body, const QString &title)
{
	QWidget *page = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(page);
	lay->setContentsMargins(0, 0, 0, 0);

	MenuSectionCollapseWidget *section = it->createMenuSection(title, SO_CH, page);
	section->add(body);
	lay->addWidget(section);
	lay->addStretch();
	return page;
}

QWidget *blockPage(InstrumentTemplate *it, scopy::acq::Block *block, const QString &title)
{
	return widgetPage(it, block->settingsWidget(), title);
}

// The engine's registered sources are the only record of what setupBlocks() built, so the
// code below asks it rather than holding a pointer per source type.
scopy::acq::SourceBlock *sourceById(scopy::acq::AcquisitionEngine *engine, const QString &id)
{
	for(scopy::acq::SourceBlock *src : engine->sources()) {
		if(src->id() == id) {
			return src;
		}
	}
	return nullptr;
}

} // namespace

AcqInstrumentController::AcqInstrumentController(ToolMenuEntry *tme, QObject *parent)
	: QObject(parent)
	, m_kPlutoSampleRate(2.4e6)
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
	setupProcessors();
	setupPlots();
	// Both after setupPlots(): the fire window and the decode window are the plot window.
	setupTrigger();
	setupDecoders();
	// Last: it sits in a slot around the center widget setupPlots() installs.
	setupAnalysisPanel();

	// The tool menu's own run button and the instrument's stay in step. No loop: setRunning()
	// only changes state, so the echo back is a no-op.
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
	scopy::acq::AcquisitionEngine *engine = m_ui->engine();
	MenuSectionCollapseWidget     *sources = m_ui->shell()->addChannelGroup("Sources");

	for(const AcqSourceFactory &make : AcqSourceRegistry::instance().all()) {
		// Constructed to be asked: a source holds the context and decides for itself whether
		// its device is in it. Cheap — an unavailable source's constructor is a no-op.
		std::unique_ptr<scopy::acq::SourceBlock> src(make(ctx, engine));
		if(!src || !src->isAvailable()) {
			continue;
		}
		addBlockRow(sources, src.get(), src->id(), src->id().toUpper(), src->id());
		engine->addSource(src.release());
	}
}

void AcqInstrumentController::setupProcessors()
{
	scopy::acq::AcquisitionEngine *engine = m_ui->engine();

	// Before the early return, so the section lands between Sources and Plots on the rail
	// even with no Pluto: the trigger and the decoders add their rows to it later.
	MenuSectionCollapseWidget *procs = m_ui->shell()->addChannelGroup("Processors");

	if(!sourceById(engine, QStringLiteral("pluto"))) {
		return;
	}

	// Two watched keys, so this is the complex path: I on voltage0, Q on voltage1, nfft
	// tied to the engine's buffer size.
	m_fftProc = new scopy::acq::GenalyzerFFTProcessor(scopy::acq::DataKey::raw("pluto", "voltage0"),
							  scopy::acq::DataKey::raw("pluto", "voltage1"),
							  scopy::acq::DataKey::withStage("pluto", "iq", "fft"),
							  scopy::acq::DataKey::withStage("pluto", "iq", "freq"),
							  static_cast<int>(engine->bufferSize()), m_kPlutoSampleRate,
							  GnWindowHann, engine);
	// Lets the block claim chunk history when averaging is turned on: navg frames are navg
	// past chunks.
	m_fftProc->setAveragingStore(m_ui->store());
	engine->addProcessor(m_fftProc);

	addBlockRow(procs, m_fftProc, QStringLiteral("FFT"), QStringLiteral("GENALYZER FFT"),
		    QStringLiteral("fft"));
}

MenuControlButton *AcqInstrumentController::addBlockRow(MenuSectionCollapseWidget *group, scopy::acq::Block *block,
							const QString &label, const QString &pageTitle,
							const QString &menuId)
{
	if(!group || !block) {
		return nullptr;
	}
	InstrumentTemplate *it = m_ui->shell();

	// No colour: a coloured swatch on the rail means "this is the curve you see in that
	// colour", and only plot channels have one.
	MenuControlButton *row = it->addChannelRow(group, label, QColor(), menuId);
	it->addMenuPage(menuId, blockPage(it, block, pageTitle));

	return row;
}

void AcqInstrumentController::setupPlots()
{
	InstrumentTemplate *it = m_ui->shell();

	m_plots = new AcqPlotManager(m_ui->store(), m_ui->engine(), it, m_ui);
	it->setCenterWidget(m_plots);

	// Direct, not queued: AcqInstrument already mirrors the engine's worker-thread
	// signals onto the GUI thread, and these are its GUI-thread re-emissions.
	//
	// Kept: setupTrigger() swaps this for the trigger's fires.
	m_cycleConn = connect(m_ui, &AcqInstrument::cycleComplete, m_plots, &AcqPlotManager::onCycleComplete);
	connect(m_ui, &AcqInstrument::started, m_plots, &AcqPlotManager::onStarted);
	connect(m_ui, &AcqInstrument::stopped, m_plots, &AcqPlotManager::onStopped);

	// The file source publishes outside a cycle, and the frame timer only runs while
	// acquiring — this is what makes a file draw with the engine stopped.
	if(auto *fileSrc = qobject_cast<FileSourceBlock *>(sourceById(m_ui->engine(), QStringLiteral("file")))) {
		connect(fileSrc, &FileSourceBlock::published, m_plots, [this]() {
			m_plots->onCycleComplete();
			m_plots->replot();
		});
	}

	// How much history is drawn, independent of how much arrives per cycle: a window wider
	// than one buffer is stitched from several chunks, which is the store's business.
	gui::MenuSpinbox *plotSpin = new gui::MenuSpinbox("Plot window", m_plots->plotSize(), "samples", 16, 1 << 20,
							  true, false, false, it);
	plotSpin->setIncrementMode(gui::MenuSpinbox::IS_POW2);
	it->addEngineControl(plotSpin);
	connect(plotSpin, &gui::MenuSpinbox::valueChanged, m_plots, [this](double v) {
		const int n = static_cast<int>(v);
		m_plots->setPlotSize(n);
		// The marker's axis needs nothing — the curve re-requests its range on the next
		// read — but its window does: one call carries the new width to the marker, the
		// processor and the target spinbox, so the three cannot disagree.
		updateTriggerMarkerWindow();
		if(m_decoderMgr) {
			m_decoderMgr->setDecoderWindowSize(n);
		}
	});

	// The engine's sample-index ramp has to be at least as long as the widest plot, or a plot
	// wider than it reads a short X window and draws a truncated curve. The manager states
	// the requirement and this connection applies it.
	if(scopy::acq::AcquisitionEngine *engine = m_ui->engine()) {
		connect(m_plots, &AcqPlotManager::maxWindowSizeChanged, engine,
			[engine](int n) { engine->setIndexRampLength(static_cast<std::size_t>(n)); });
		// Once now: a channel reads X on its very first pull.
		engine->setIndexRampLength(static_cast<std::size_t>(m_plots->plotSize()));
	}

	if(m_fftProc) {
		// The timeline for channels whose producer declared no rate — Pluto's raw ones,
		// since PlutoIIOSource does not read the device rate back. A stream carrying its
		// own rate ignores this.
		m_plots->setFallbackSampleRate(m_fftProc->sampleRate());
	}
}

void AcqInstrumentController::setupTrigger()
{
	scopy::acq::AcquisitionEngine *engine = m_ui->engine();
	InstrumentTemplate            *it = m_ui->shell();

	m_trigProc = new scopy::acq::TriggerProcessor(QStringLiteral("trigger"), engine);
	m_trigProc->setEnabled(false);
	m_trigProc->setWindowSize(m_plots->plotSize());
	m_trigProc->setTriggerPosition(0.5);
	engine->addProcessor(m_trigProc);
	// Its snapshot spans the whole store, so it has to run after this cycle's derived data —
	// the FFT, the decoder annotations — is written.
	engine->setRunLast(m_trigProc);

	m_trigMarker = new scopy::acq::TriggerMarker(m_trigProc, this);

	m_trigBinder = new scopy::acq::TriggerBinder(m_trigProc, engine, this);
	// The fire's own window set is forwarded rather than dropped: re-reading the store here
	// would draw the newest samples, a different window from the one the fire index names.
	connect(m_trigBinder, &scopy::acq::TriggerBinder::replotFired, m_plots,
		[this](quint32, const QMap<QString, scopy::acq::SampleVariant> &snap) {
			m_plots->onTriggerFired(snap);
		});

	// Built here and handed to the block, so the Pipeline tab shows this one widget rather
	// than a second, independent copy of the same condition list.
	QWidget     *body = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(body);
	lay->setContentsMargins(0, 0, 0, 0);
	// The base widget explicitly: TriggerProcessor's own createSettingsWidget() would build
	// a second TriggerProcessorWidget right above the one added below.
	lay->addWidget(m_trigProc->ProcessorBlock::createSettingsWidget(body));

	// Which X axis the marker rides.
	MenuSectionCollapseWidget *axisSection = it->createMenuSection(tr("TRIGGER AXIS"), SO_VIEW, body);
	m_trigAxisCombo = new MenuCombo(tr("Axis source"), axisSection);
	// Every key, unfiltered: a picker that hid a key until some channel happened to draw it
	// would change its own contents behind the reader. A pick that resolves to no axis is
	// handled instead, by hiding the handle until it does resolve.
	m_plots->populateKeyCombo(m_trigAxisCombo, /*withSampleIndex=*/true);
	m_trigAxisKey = AcqPlotManager::keyFromCombo(m_trigAxisCombo.data());
	connect(m_trigAxisCombo->combo(), &QComboBox::currentIndexChanged, this, [this](int) {
		m_trigAxisKey = AcqPlotManager::keyFromCombo(m_trigAxisCombo.data());
		retargetTriggerHandle();
	});
	axisSection->add(m_trigAxisCombo);
	lay->addWidget(axisSection);

	m_trigWidget = new scopy::acq::TriggerProcessorWidget(m_trigProc, body);
	m_trigWidget->setMaxTargetSample(std::max(0, m_plots->plotSize() - 1));
	lay->addWidget(m_trigWidget);
	m_trigProc->setSettingsWidget(body);

	addBlockRow(it->addChannelGroup("Processors"), m_trigProc, tr("Trigger"), QStringLiteral("TRIGGER"),
		    QStringLiteral("trigger"));

	// All four signals, not just plotAdded: the axis the marker rides is created by a
	// *channel*, so a plot appearing is neither necessary nor sufficient.
	if(!m_plots.isNull()) {
		retargetTriggerHandle();
		connect(m_plots, &AcqPlotManager::plotAdded, this, [this](quint32) { retargetTriggerHandle(); });
		// The dying plot is excluded because it is still in m_plots->plots() here, so the
		// resolve would otherwise find its axis and put the handle straight back onto it.
		connect(m_plots, &AcqPlotManager::plotRemoved, this, [this](quint32 uuid) {
			if(m_plots.isNull()) {
				return;
			}
			retargetTriggerHandle(m_plots->plot(uuid));
		});
		// No exclusion for channelRemoved: the plot survives and may have another channel
		// on the same X.
		connect(m_plots, &AcqPlotManager::channelAdded, this, [this](AcqChannel *) { retargetTriggerHandle(); });
		connect(m_plots, &AcqPlotManager::channelRemoved, this,
			[this](AcqChannel *) { retargetTriggerHandle(); });
	}

	// Follows the block, not a button of our own: the reader flips the trigger from its own
	// settings widget. The marker's visibility follows the same signal on its own.
	connect(m_trigProc, &scopy::acq::ProcessorBlock::enabledChanged, this, [this](bool en) {
		if(!m_trigBinder) {
			return;
		}
		if(en) {
			disconnect(m_cycleConn);
			m_cycleConn = {};
			m_trigBinder->bindReplotOnFire();
		} else {
			m_trigBinder->unbindReplotOnFire();
			if(!m_cycleConn) {
				m_cycleConn = connect(m_ui, &AcqInstrument::cycleComplete, m_plots,
						      &AcqPlotManager::onCycleComplete);
			}
		}
		// With a trigger, "one acquisition" is one fire, not one cycle.
		m_ui->setSingleWaitsForStop(en);
	});

	// Armed per press, not once: the binder's one-shot disconnects itself on the fire it
	// stops on.
	connect(m_ui, &AcqInstrument::singleRequested, this, [this]() {
		if(m_trigBinder && m_trigProc && m_trigProc->isEnabled()) {
			m_trigBinder->armSingleShot();
		}
	});
	// On any stop, so an aborted Single does not leave an arm behind to stop the *next* run
	// on its first fire.
	connect(m_ui, &AcqInstrument::stopped, this, [this]() {
		if(m_trigBinder) {
			m_trigBinder->disarmSingleShot();
		}
	});

	// The widget's status label ("idle" / "waiting" / "triggered") and its key combos.
	connect(m_ui, &AcqInstrument::started, m_trigWidget,
		[this]() { m_trigWidget->setAcquisitionRunning(true); });
	connect(m_ui, &AcqInstrument::stopped, m_trigWidget,
		[this]() { m_trigWidget->setAcquisitionRunning(false); });
	connect(m_plots, &AcqPlotManager::keysAvailable, m_trigWidget,
		[this](const QList<scopy::acq::DataKey> &keys) {
			QStringList names;
			names.reserve(keys.size());
			for(const scopy::acq::DataKey &k : keys) {
				names << k.key;
			}
			m_trigWidget->setAvailableKeys(names);
		});
	// The axis picker off the same list. No retarget: populateKeyCombo keeps the current
	// selection, and a new key is not a new axis — channelAdded says one was drawn.
	connect(m_plots, &AcqPlotManager::keysAvailable, this, [this](const QList<scopy::acq::DataKey> &) {
		if(!m_trigAxisCombo.isNull() && !m_plots.isNull()) {
			m_plots->populateKeyCombo(m_trigAxisCombo.data(), /*withSampleIndex=*/true);
		}
	});
}

PlotAxis *AcqInstrumentController::findXAxisFor(AcqPlot *plot, const scopy::acq::DataKey &key) const
{
	if(!plot || key.key.isEmpty()) {
		return nullptr;
	}

	// The channels, not plot->axisForSource(): that one creates on miss, and only a channel
	// can say an axis is really being drawn against.
	PlotAxis *fallback = nullptr;
	const QList<AcqChannel *> chans = plot->channels();
	for(AcqChannel *ch : chans) {
		if(!ch || !ch->xAxis()) {
			continue;
		}
		AcqAxis *ax = ch->xAxis();
		// isHorizontal() even on the X side: a channel's "X" axis is whichever PlotAxis it
		// was given, and nothing stops a caller handing it a vertical one.
		if(ax->source().key != key || !ax->isHorizontal() || !ax->plotAxis()) {
			continue;
		}
		// Sample index and time share the ramp key, so a key match cannot tell them apart.
		// Prefer the index axis: its interval really is 0..plotSize-1, which makes the
		// position-to-sample map exact rather than merely monotonic.
		if(ax->isSampleIndex()) {
			return ax->plotAxis();
		}
		if(!fallback) {
			fallback = ax->plotAxis();
		}
	}
	return fallback;
}

void AcqInstrumentController::retargetTriggerHandle(AcqPlot *excluding)
{
	if(!m_trigProc || m_plots.isNull()) {
		return;
	}

	// First plot that has it: one trigger, and no basis for preferring a later plot.
	AcqPlot  *target = nullptr;
	PlotAxis *axis = nullptr;
	const QList<AcqPlot *> plots = m_plots->plots();
	for(AcqPlot *p : plots) {
		if(p == excluding) {
			continue;
		}
		if(PlotAxis *ax = findXAxisFor(p, m_trigAxisKey)) {
			target = p;
			axis = ax;
			break;
		}
	}

	if(!axis) {
		// Detached rather than parked on some other axis. The combo keeps the reader's
		// pick, so this resolves itself once a channel draws X against that key.
		const bool had = m_trigMarker && m_trigMarker->isAttached();
		if(m_trigMarker) {
			m_trigMarker->detach();
		}
		m_trigMarkerPlot = nullptr;
		m_trigMarkerAxis = nullptr;
		if(had) {
			// Only on the transition: this runs on every plot and channel change.
			qWarning() << "acq: trigger axis" << m_trigAxisKey.toString()
				   << "is not drawn as an X axis on any plot — marker hidden";
		}
		return;
	}

	PlotWidget *w = target->plot();
	if(!w) {
		return;
	}

	m_trigMarkerPlot = target;
	m_trigMarkerAxis = axis;
	// The window before the attach, so the marker's first sync places the bar against the
	// right one.
	updateTriggerMarkerWindow();
	m_trigMarker->attach(w, axis);
}

void AcqInstrumentController::updateTriggerMarkerWindow()
{
	if(m_trigMarker.isNull()) {
		return;
	}

	// The *marker plot's* plotSize: plots can be different widths. The manager's default is
	// only the answer before the first resolve.
	const int n = m_trigMarkerPlot.isNull() ? (m_plots.isNull() ? 1 : m_plots->plotSize())
						: m_trigMarkerPlot->plotSize();
	const int last = std::max(0, n - 1);

	// The axis units that window spans: slot numbers, or slots over the channel's rate for a
	// time axis. Any other X is left as slot numbers — monotonic, which is all the marker
	// needs, and the most a proportional map can claim without scanning the stream.
	double x1 = static_cast<double>(last);
	if(!m_trigMarkerAxis.isNull() && !m_trigMarkerPlot.isNull()) {
		const QList<AcqChannel *> chans = m_trigMarkerPlot->channels();
		for(AcqChannel *ch : chans) {
			AcqAxis *ax = ch ? ch->xAxis() : nullptr;
			if(!ax || ax->plotAxis() != m_trigMarkerAxis.data()) {
				continue;
			}
			if(ax->isTime()) {
				const double rate = ch->sampleRate() > 0.0 ? ch->sampleRate() : 1.0;
				x1 = static_cast<double>(last) / rate;
			}
			break;
		}
	}

	m_trigMarker->setWindow(n, 0.0, x1);

	// The processor's fire window from the same `n`: the fire index is in these units and the
	// marker maps it back through them, so the two must be one number.
	if(m_trigProc) {
		m_trigProc->setWindowSize(n);
	}
	if(m_trigWidget) {
		m_trigWidget->setMaxTargetSample(std::max(0, last));
	}
}

void AcqInstrumentController::setupDecoders()
{
	scopy::acq::AcquisitionEngine *engine = m_ui->engine();
	InstrumentTemplate            *it = m_ui->shell();
	scopy::decoder::DecoderLogger *log = m_ui->decoderLogger();

	// Neither is enumerated here: listing decoders runs sigrok-cli as a subprocess, so it
	// waits until the reader opens a picker.
	auto catalog = std::make_unique<scopy::decoder::SigrokCliCatalog>();
	catalog->setLogger(log);
	auto factory = std::make_unique<scopy::decoder::SigrokCliBackendFactory>(catalog.get());
	factory->setLogger(log);

	m_decoderMgr = new DecoderManager(engine, m_ui->store(), factory.get(), this);
	m_decoderMgr->setLogger(log);
	m_decoderMgr->setDecoderWindowSize(m_plots->plotSize());
	// No setPlot/setOverlay: annotations are drawn by adding a ReprKind::Annotations channel
	// on the output key, through the same ADD CHANNEL flow every other stream uses.

	// HostScrolls: the right menu already wraps its stack in a scroll area, and a nested one
	// would cap the panel's reported height and cut off everything past it.
	m_decoderPanel = new DecoderPanel(m_decoderMgr, m_ui->store(), catalog.get(), m_ui, DecoderPanel::HostScrolls);
	m_decoderPanel->setLogger(log);

	// What addBlockRow() does, inlined: the decoders are a stack of blocks the manager adds
	// on demand, so there is no one Block whose settings widget this is.
	MenuSectionCollapseWidget *procs = it->addChannelGroup("Processors");
	it->addChannelRow(procs, tr("Decoders"), QColor(), QStringLiteral("decoders"));
	it->addMenuPage(QStringLiteral("decoders"), widgetPage(it, m_decoderPanel, tr("DECODERS")));

	// Members last: the panel and the manager borrow both, and the factory borrows the
	// catalog, so all three must outlive them.
	m_decoderCatalog = std::move(catalog);
	m_decoderFactory = std::move(factory);

	// A new editor comes up with empty channel combos unless it is given the key set: the
	// store may not have changed since the last refresh.
	connect(m_decoderMgr, &DecoderManager::decoderAdded, m_decoderPanel,
		[this](const QString &) { m_decoderPanel->refreshKeys(m_ui->store()->keys()); });
	connect(m_plots, &AcqPlotManager::keysAvailable, m_decoderPanel,
		[this](const QList<scopy::acq::DataKey> &keys) { m_decoderPanel->refreshKeys(keys); });

	if(PipelineInspector *pipeline = m_ui->pipeline()) {
		pipeline->setDecoderManager(m_decoderMgr);
	}

	// A new decoder draws itself on the first plot there is. The plot manager knows nothing
	// about decoders; this is the whole link between the two.
	connect(m_decoderMgr, &DecoderManager::decoderAdded, m_plots, [this](const QString &uid) {
		DecoderInstance *d = m_decoderMgr->find(uid);
		if(!d || d->outKeys.isEmpty()) {
			return;
		}
		// A fresh decoder is one stage, so one output key: the root's.
		drawDecoderStage(d->outKeys.first());
	});

	// A stacked stage is a new output key on a decoder that already exists, so decoderAdded
	// has already fired and says nothing about it. Without this it publishes undrawn.
	connect(m_decoderMgr, &DecoderManager::stageAdded, m_plots, [this](const QString &uid, int stageIndex) {
		DecoderInstance *d = m_decoderMgr->find(uid);
		if(!d || stageIndex < 0 || stageIndex >= d->outKeys.size()) {
			return;
		}
		drawDecoderStage(d->outKeys.at(stageIndex));
	});

	// The mirror: a dropped key stops being written, so the channel drawing it would sit
	// there greyed out forever.
	connect(m_decoderMgr, &DecoderManager::stagesRemoved, m_plots,
		[this](const QString &, const QList<scopy::acq::DataKey> &keys) {
			// Over a copy of the channel list: removeChannel mutates it.
			const QList<AcqChannel *> chans = m_plots->channels();
			for(AcqChannel *ch : chans) {
				if(ch && keys.contains(ch->key())) {
					m_plots->removeChannel(ch);
				}
			}
		});
}

void AcqInstrumentController::drawDecoderStage(const scopy::acq::DataKey &outKey)
{
	if(!m_plots) {
		return;
	}
	// No plot yet: nothing to draw on, and not an error — a decoder can exist before any
	// plot does, and the reader can still add the channel from ADD CHANNEL.
	const QList<AcqPlot *> plots = m_plots->plots();
	if(plots.isEmpty()) {
		return;
	}
	// Two channels may share a Y key by design, so the manager will not refuse a duplicate.
	// Reachable: a popped stage can be re-pushed, which reuses the key.
	for(AcqChannel *ch : m_plots->channels()) {
		if(ch && ch->key() == outKey) {
			return;
		}
	}
	// X left empty: annotation offsets are relative to the window the decode ran on, so they
	// index the plot's own sample ramp.
	m_plots->addChannel(plots.first(), scopy::acq::ReprKind::Annotations, outKey);
}

void AcqInstrumentController::setupAnalysisPanel()
{
	if(!m_fftProc) {
		return;
	}

	InstrumentTemplate *it = m_ui->shell();

	// PS_RIGHT rather than the right menu: this read-out annotates the spectrum, so it
	// belongs beside the plot and stays visible while a channel's menu page is open.
	m_genalyzerPanel = new GenalyzerPanel(it);
	it->addToSlot(PS_RIGHT, m_genalyzerPanel);
	m_genalyzerPanel->setVisible(m_fftProc->config().enabled);

	const QString channelName = m_fftProc->outputKey().toString();

	// Queued: the engine runs the processor on its own thread. The snapshot is a registered
	// metatype, so it deep-copies across the connection.
	connect(
		m_fftProc, &scopy::acq::GenalyzerFFTProcessor::analysisReady, this,
		[this, channelName](const scopy::acq::GenalyzerResultsSnapshot &snap) {
			if(!m_genalyzerPanel) {
				return;
			}
			// GenalyzerPanel::updateResults takes genalyzer's raw char**/double*
			// arrays, so the snapshot is unpacked into views that outlive the call.
			const int               n = snap.keys.size();
			std::vector<QByteArray> keyBytes;
			keyBytes.reserve(n);
			std::vector<char *> keyPtrs;
			keyPtrs.reserve(n);
			for(const QString &k : snap.keys) {
				keyBytes.emplace_back(k.toUtf8());
				keyPtrs.push_back(keyBytes.back().data());
			}
			std::vector<double> values(snap.values.begin(), snap.values.end());
			m_genalyzerPanel->updateResults(channelName, QColor(0x4a, 0xb8, 0xff), static_cast<size_t>(n),
							keyPtrs.empty() ? nullptr : keyPtrs.data(),
							values.empty() ? nullptr : values.data());
		},
		Qt::QueuedConnection);

	// Cleared rather than just hidden: the next enable should not flash stale values before
	// the first snapshot arrives.
	connect(
		m_fftProc, &scopy::acq::GenalyzerFFTProcessor::analysisEnabledChanged, this,
		[this](bool en) {
			if(!m_genalyzerPanel) {
				return;
			}
			if(!en) {
				m_genalyzerPanel->clear();
			}
			m_genalyzerPanel->setVisible(en);
		},
		Qt::QueuedConnection);
}

void AcqInstrumentController::stop()
{
	if(m_ui) {
		m_ui->stop();
	}
}

AcqInstrument *AcqInstrumentController::ui() const { return m_ui; }

#include "moc_acqinstrumentcontroller.cpp"
