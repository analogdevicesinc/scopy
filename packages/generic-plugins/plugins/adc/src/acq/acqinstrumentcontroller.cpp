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
#include "acqplot.h"
#include "acqplotkind.h"
#include "acqplotmanager.h"
#include "adxl355source.h"

#include <iio.h>

#include <core/acq_engine/AcquisitionEngine.h>
#include <core/acq_engine/Block.h>
#include <core/acq_engine/DataStore.h>
#include <core/acq_engine/GenalyzerFFTProcessor.h>
#include <core/acq_engine/SnapshotSource.h>
#include <core/acq_engine/SnapshotSourceWidget.h>
#include <core/acq_engine/SourceBlock.h>
#include <gui/instrumenttemplate.h>
#include <gui/style.h>
#include <gui/widgets/genalyzerpanel.h>

#include <QMap>
#include <QPointer>
#include <QSignalBlocker>
#include <QVBoxLayout>

#include <memory>
#include <vector>

using namespace scopy;
using namespace scopy::adc;

namespace {

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
	// After setupBlocks: the channels added here point at keys the blocks declare.
	setupPlots();
	// After both: it sits in a slot around the center widget setupPlots() installs,
	// and it listens to the block setupBlocks() created.
	setupAnalysisPanel();

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
	scopy::acq::AcquisitionEngine *engine = m_ui->engine();
	InstrumentTemplate            *it = m_ui->shell();

	// The group both paths add to. Created up front because the snapshot source belongs
	// in it whether or not a context was opened.
	MenuSectionCollapseWidget *sources = it->addChannelGroup("Sources");
	setupSnapshotBlock(sources);

	if(!ctx) {
		// Every hardware source here opens a real IIO device; there is no simulated
		// stand-in, so the hardware half of the pipeline is skipped rather than
		// pretended. The snapshot source above still works — it needs no device.
		return;
	}

	// Which sources appear is decided by what the context actually holds, not by
	// which tool opened it: both calls no-op when their device is absent, so a Pluto
	// context comes up with the Pluto source and an ADXL context with the ADXL one,
	// through one code path.
	setupPlutoBlocks(sources, ctx);
	setupAdxlBlocks(sources, ctx);

	if(m_fftProc) {
		MenuSectionCollapseWidget *procs = it->addChannelGroup("Processors");
		addBlockRow(procs, m_fftProc, QStringLiteral("FFT"), QStringLiteral("GENALYZER FFT"),
			    QStringLiteral("fft"));
	}
}

bool AcqInstrumentController::setupPlutoBlocks(MenuSectionCollapseWidget *sourcesGroup, iio_context *ctx)
{
	if(!iio_context_find_device(ctx, "cf-ad9361-lpc")) {
		return false;
	}

	scopy::acq::AcquisitionEngine *engine = m_ui->engine();

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
							  static_cast<int>(engine->bufferSize()), m_kPlutoSampleRate,
							  GnWindowHann, engine);
	// Lets the block claim chunk history when averaging is turned on: navg frames
	// are navg past chunks.
	m_fftProc->setAveragingStore(m_ui->store());
	engine->addProcessor(m_fftProc);

	addBlockRow(sourcesGroup, m_plutoSrc, QStringLiteral("pluto"), QStringLiteral("PLUTO"),
		    QStringLiteral("pluto"));
	return true;
}

bool AcqInstrumentController::setupAdxlBlocks(MenuSectionCollapseWidget *sourcesGroup, iio_context *ctx)
{
	if(!iio_context_find_device(ctx, "adxl355")) {
		return false;
	}

	scopy::acq::AcquisitionEngine *engine = m_ui->engine();

	// No FFT alongside it, unlike the Pluto path. The accel triple is three real
	// streams rather than one complex pair, so a spectrum here would be three
	// separate real transforms — three more blocks, which is a decision for whoever
	// wants them and not a default this instrument should make.
	m_adxlSrc = new Adxl355Source(ctx, QStringLiteral("adxl355"), QStringLiteral("adxl355"), engine);
	// The block registers its own four channels with accel on and temp off, so
	// nothing is enabled here — doing it again would just restate its default.
	engine->addSource(m_adxlSrc);

	addBlockRow(sourcesGroup, m_adxlSrc, QStringLiteral("adxl355"), QStringLiteral("ADXL355"),
		    QStringLiteral("adxl355"));
	return true;
}

CollapsableMenuControlButton *AcqInstrumentController::addBlockRow(MenuSectionCollapseWidget *group,
								   scopy::acq::Block *block, const QString &label,
								   const QString &pageTitle, const QString &menuId)
{
	if(!group || !block) {
		return nullptr;
	}
	InstrumentTemplate *it = m_ui->shell();

	// Every block gets the same entry, source or processor: one expandable row plus its
	// settings page. Uniform on purpose — the rail is a view of the pipeline, and a
	// pipeline block is a pipeline block whichever group it sits in, so a reader should
	// not have to learn two row shapes to read one graph.
	//
	// No colour: a coloured swatch on the rail means "this is the curve you see in that
	// colour", and only plot channels have one.
	CollapsableMenuControlButton *row = it->addExpandableChannelRow(group, label, QColor(), menuId);
	it->addMenuPage(menuId, blockPage(it, block, pageTitle));

	// The subtree is what the row is expandable *for*: a source's channels hang under it
	// as a tree rather than as a flat list that says nothing about which device they
	// belong to. A block that declares no channels — every processor here, and the
	// snapshot source until a slot is captured — just has an empty one; the row itself
	// stays the same shape either way.
	if(auto *src = qobject_cast<scopy::acq::SourceBlock *>(block)) {
		addSourceChannelRows(row, src);
	}

	return row;
}

void AcqInstrumentController::setupSnapshotBlock(MenuSectionCollapseWidget *sourcesGroup)
{
	scopy::acq::AcquisitionEngine *engine = m_ui->engine();
	scopy::acq::DataStore         *store = m_ui->store();

	m_snapSrc = new scopy::acq::SnapshotSource("snapshot", engine);
	// The store it captures *from*, which is the same one the engine writes to. A block
	// cannot reach it on its own.
	m_snapSrc->setSourceStore(store);
	// One empty slot, so the panel opens on something to configure rather than on a bare
	// "add" button. It publishes nothing until captured.
	m_snapSrc->addSlot();
	engine->addSource(m_snapSrc);

	// Host-built widget: its key pickers need the DataStore and the engine, which the
	// constructor of a block has no way to supply. Same pattern the trigger uses.
	auto *body = new QWidget;
	auto *lay = new QVBoxLayout(body);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(4);
	// The base virtual explicitly, or SnapshotSource's override builds a second panel
	// inside the one we are assembling.
	lay->addWidget(m_snapSrc->SourceBlock::createSettingsWidget(body));
	m_snapWidget = new scopy::acq::SnapshotSourceWidget(m_snapSrc, store, engine, body);
	lay->addWidget(m_snapWidget);
	// Before any settingsWidget() call, which blockPage() below is.
	m_snapSrc->setSettingsWidget(body);

	// Same entry as every other block, through the one path that builds them.
	addBlockRow(sourcesGroup, m_snapSrc, QStringLiteral("Snapshot"), QStringLiteral("SNAPSHOT"),
		    QStringLiteral("snapshot"));

	// Queued: keysChanged comes off the engine's worker thread.
	connect(store, &scopy::acq::DataStore::keysChanged, m_snapWidget,
		[this](const QList<scopy::acq::DataKey> &) {
			if(m_snapWidget) {
				m_snapWidget->refreshKeys();
			}
		},
		Qt::QueuedConnection);
}

void AcqInstrumentController::addSourceChannelRows(CollapsableMenuControlButton *parentRow,
						  scopy::acq::SourceBlock *src)
{
	if(!parentRow || !src) {
		return;
	}
	InstrumentTemplate *it = m_ui->shell();

	// id -> its switch, so the source flipping a channel itself can find the row again.
	// Shared rather than a member: it belongs to this one source's subtree, and both
	// lambdas below outlive this call.
	auto switches = std::make_shared<QMap<QString, QPointer<SmallOnOffSwitch>>>();

	// Rebuilt wholesale rather than diffed: a source has a handful of channels, and the
	// list is only rebuilt when it actually changes.
	auto rebuild = [this, it, parentRow, src, switches]() {
		const QList<MenuControlButton *> old = parentRow->findChildren<MenuControlButton *>();
		for(MenuControlButton *row : old) {
			// The header is a child too, and removing it would take the whole row's
			// selection and page with it.
			if(row == parentRow->getControlBtn()) {
				continue;
			}
			it->removeChannelRow(parentRow, row, QString());
		}
		switches->clear();

		const QList<QString> ids = src->channelIds();
		for(const QString &id : ids) {
			// A switch, not a checkbox: a channel is enabled or not on the device,
			// which is the same on/off a plot row's switch expresses. No page — the
			// source's own settings cover the whole device — and no colour, because a
			// raw channel is not a curve until someone adds one from the key picker.
			// Indented one step: the header's own text is already offset by the
			// collapse arrow, so a child at margin 0 reads as the source's sibling
			// rather than as something under it.
			MenuControlButton *row = it->addChannelSwitchRow(parentRow, id, QColor(), QString(), 1);
			SmallOnOffSwitch *sw = InstrumentTemplate::rowSwitch(row);
			if(!sw) {
				continue;
			}
			QSignalBlocker b(sw);
			sw->setChecked(src->isChannelEnabled(id));
			switches->insert(id, sw);
			connect(sw, &QAbstractButton::toggled, src, [src, id](bool en) { src->enableChannel(id, en); });
		}
	};

	rebuild();
	// Queued, both of them: a source can add channels or flip one from the worker
	// thread (onStart reading the device, disableAllChannels on a failed start), and
	// these touch widgets.
	connect(src, &scopy::acq::SourceBlock::channelsChanged, this, rebuild, Qt::QueuedConnection);
	// Without this the switch would keep claiming a channel is on after the source
	// turned it off by itself.
	connect(
		src, &scopy::acq::SourceBlock::channelEnabledChanged, this,
		[switches](const QString &id, bool en) {
			SmallOnOffSwitch *sw = switches->value(id).data();
			if(!sw) {
				return;
			}
			// Blocked: this reflects what the source already did, so echoing it
			// back through enableChannel() would be a round trip for nothing.
			QSignalBlocker b(sw);
			sw->setChecked(en);
		},
		Qt::QueuedConnection);
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
	// Nothing wired to the buffer size: a channel claims its window in samples and the
	// store converts, so the view never learns the chunk length. See AcqInstrument's
	// signal list for why that signal no longer exists.

	// How much history is drawn, independent of how much arrives per cycle: a window
	// wider than one buffer is stitched from several chunks, which is the store's
	// business. Live while running — it only changes claims and the X range.
	gui::MenuSpinbox *plotSpin = new gui::MenuSpinbox("Plot window", m_plots->plotSize(), "samples", 16, 1 << 20,
							  true, false, false, it);
	plotSpin->setIncrementMode(gui::MenuSpinbox::IS_POW2);
	it->addEngineControl(plotSpin);
	connect(plotSpin, &gui::MenuSpinbox::valueChanged, m_plots,
		[this](double v) { m_plots->setPlotSize(static_cast<int>(v)); });

	// The engine's sample-index ramp has to be at least as long as the widest plot, or a
	// plot wider than it reads a short X window and draws a truncated curve. The manager
	// states the requirement and this connection applies it — the manager holds no engine
	// pointer on purpose.
	//
	// Only a length change reaches the engine's std::iota; the write itself is one chunk,
	// so this costs nothing per cycle.
	if(scopy::acq::AcquisitionEngine *engine = m_ui->engine()) {
		connect(m_plots, &AcqPlotManager::maxWindowSizeChanged, engine,
			[engine](int n) { engine->setIndexRampLength(static_cast<std::size_t>(n)); });
		// Once now: the manager's plots and channels are created below, and a channel
		// reads X on its very first pull.
		engine->setIndexRampLength(static_cast<std::size_t>(m_plots->plotSize()));
	}

	// No cursors wiring here any more. Cursors are per plot, on the plot's own VIEW
	// section, because PlotCursors binds its handles to one canvas in its constructor —
	// so one instrument-wide button and controller could only ever drive the first plot.

	if(m_fftProc) {
		// The timeline for channels whose producer declared no rate — Pluto's raw
		// channels, since PlutoIIOSource does not read the device rate back. From the
		// FFT processor because that is where the rate is configured. A stream that
		// carries its own rate ignores this: the FFT magnitudes do, and so does every
		// ADXL channel, which is why setupAdxlBlocks needs nothing here. Before the
		// channels below, though either order works.
		m_plots->setFallbackSampleRate(m_fftProc->sampleRate());
	}

	// --- A starting view, and only a starting view -------------------------------
	//
	// Everything below is one worked example of the manager's API, not policy. The
	// manager scans nothing and infers nothing: with these lines removed the instrument
	// opens on an empty dock area and the reader builds whatever they want from the "Plots"
	// rail group. They are here so the tool opens on something useful, and so the two
	// cases the tree was designed around are exercised on every run.
	//
	// The reader can delete either plot, delete either channel, retarget any axis, or add
	// more of both — nothing here is protected and nothing recreates itself.
	//
	// The pipeline still owns the *details* of each channel: label, unit, colour, sample
	// rate and recommended X source all come from the producer's StreamInfo. What these
	// lines decide is only that the channel exists at all.
	setupExampleView();
}

void AcqInstrumentController::setupExampleView()
{
	if(!m_plots) {
		return;
	}

	// --- 1. Time domain: the registered source's raw channels on one Basic plot --
	//
	// First, which is all order decides now that the plots are docks: this one sits above
	// the waterfall, and the two share the height evenly until the reader drags the
	// separator.
	//
	// All against the sample index (the default empty xKey), which is what makes them
	// share one X scale and stay aligned with each other. They are the case per-channel X
	// costs nothing for: several channels, one X source, one visible X axis.
	if(m_plutoSrc) {
		if(AcqPlot *timePlot = m_plots->addPlot(tr("Time"), AcqPlotKind::Basic)) {
			m_plots->addChannel(timePlot, scopy::acq::ReprKind::Curve,
					    scopy::acq::DataKey::raw("pluto", "voltage0"));
			m_plots->addChannel(timePlot, scopy::acq::ReprKind::Curve,
					    scopy::acq::DataKey::raw("pluto", "voltage1"));
		}
	}

	if(m_adxlSrc) {
		// The accel triple only. temp starts disabled on the block, and a channel on
		// a stream nothing writes draws an empty curve — so the row exists in the rail
		// for the reader to enable, and the plot channel is theirs to add once it does.
		// Unit and colour come from the block's own StreamInfo, not from here.
		if(AcqPlot *accelPlot = m_plots->addPlot(tr("Acceleration"), AcqPlotKind::Basic)) {
			for(const QString &chId : Adxl355Source::kAccelChannels) {
				m_plots->addChannel(accelPlot, scopy::acq::ReprKind::Curve,
						    scopy::acq::DataKey::raw(QStringLiteral("adxl355"), chId));
			}
		}
	}

	if(!m_fftProc) {
		// No FFT block in the pipeline, so there is no magnitude stream to draw and no
		// frequency stream to draw it against. The time plot above still stands.
		return;
	}

	// --- 2. Spectrogram: FFT magnitude over time, indexed by frequency -----------
	//
	// A Waterfall plot rather than a Basic one, because the vertical axis here is chunk
	// history and not a value range — which is a different plot widget, not a different
	// curve style. One channel is all a spectrogram raster can show; nothing enforces
	// that, so a second one added by hand interleaves its rows with this one's.
	//
	// X is freqKey(), a real stream in Hz, not the sample index. This is the case the
	// per-channel X axis exists for, and it is why the channel registers a depth claim on
	// its X key as well as its Y key: an X stream left at the store's default capacity
	// would clip the magnitudes to a single chunk and the spectrum would draw short.
	// The Hz unit is read off the frequency stream's own descriptor, so it is not restated
	// here.
	//
	// freqKey() is deliberately not a channel of its own — it *is* this channel's X axis,
	// and a frequency ramp plotted against itself says nothing.
	if(AcqPlot *wfPlot = m_plots->addPlot(tr("Spectrogram"), AcqPlotKind::Waterfall)) {
		m_plots->addChannel(wfPlot, scopy::acq::ReprKind::Waterfall, m_fftProc->outputKey(),
				    m_fftProc->freqKey());
	}
}

void AcqInstrumentController::setupAnalysisPanel()
{
	if(!m_fftProc) {
		// No processor means no context: the panel would have nothing to ever show.
		return;
	}

	InstrumentTemplate *it = m_ui->shell();

	// PS_RIGHT rather than the right menu: this is a read-out that annotates the
	// spectrum, so it belongs beside the plot and stays visible while a channel's
	// menu page is open. The slot collapses to zero width when the panel is hidden.
	m_genalyzerPanel = new GenalyzerPanel(it);
	it->addToSlot(PS_RIGHT, m_genalyzerPanel);
	m_genalyzerPanel->setVisible(m_fftProc->config().enabled);

	// The one row the panel gets: the FFT output key, coloured to match nothing in
	// particular — the spectrum's own colour comes from the plot manager's palette.
	const QString channelName = m_fftProc->outputKey().toString();

	// Queued: the engine runs the processor on its own thread. The snapshot is a
	// registered metatype, so it deep-copies across the connection.
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

	// Disabling analysis leaves the last numbers frozen on screen, which reads as a
	// live measurement that stopped updating — so the table goes away with it. It is
	// cleared rather than just hidden: the next enable should not flash stale values
	// before the first snapshot arrives.
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
