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
#include <core/acq_engine/SourceBlock.h>
#include <gui/cursorcontroller.h>
#include <gui/instrumenttemplate.h>
#include <gui/style.h>
#include <gui/widgets/cursorsettings.h>
#include <gui/widgets/hoverwidget.h>

#include <QMap>
#include <QPointer>
#include <QPushButton>
#include <QSignalBlocker>
#include <QVBoxLayout>

#include <memory>

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
	// draw. No colours: these are pipeline blocks, and a coloured swatch on the rail
	// means "this is the curve you see in that colour" — only plot channels have one.
	MenuSectionCollapseWidget *sources = it->addChannelGroup("Sources");
	// Expandable, so the source's channels hang under it as a tree rather than as a
	// flat list that says nothing about which device they belong to.
	CollapsableMenuControlButton *plutoRow =
		it->addExpandableChannelRow(sources, "pluto", QColor(), QStringLiteral("pluto"));
	it->addMenuPage("pluto", blockPage(it, m_plutoSrc, "PLUTO"));
	addSourceChannelRows(plutoRow, m_plutoSrc);

	MenuSectionCollapseWidget *procs = it->addChannelGroup("Processors");
	it->addChannelRow(procs, "FFT", QColor(), "fft");
	it->addMenuPage("fft", blockPage(it, m_fftProc, "GENALYZER FFT"));
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

	// Cursors over row 0, with their settings in a hover panel off the button rather
	// than a right-menu page: the settings are read while dragging the handles, so they
	// have to sit next to the plot instead of stealing the menu the channel being
	// measured is configured in.
	CursorSettings *cursorSettings = nullptr;
	if(CursorController *cursors = m_plots->cursors(&cursorSettings)) {
		QPushButton *cursorBtn = new QPushButton(tr("Cursors"), it);
		cursorBtn->setCheckable(true);
		// The same styling the shell gives its own Debug button, which is the other
		// bottom-rail toggle — both are view state rather than engine controls, which is
		// why they sit here and not in the top rail with Run.
		Style::setStyle(cursorBtn, style::properties::button::blueGrayButton);
		Style::setStyle(cursorBtn, style::properties::label::menuMedium);
		it->addToBottomRail(cursorBtn, TTA_RIGHT);
		connect(cursorBtn, &QPushButton::toggled, cursors, &CursorController::setVisible);

		// The same anchoring the ADC time and FFT instruments use for their cursor
		// panels (adctimeinstrumentcontroller.cpp:55-58): the button's top-right corner
		// to the panel's top-left, offset 10px up. Because HP_TOPLEFT places the content
		// by its bottom-right, that grows the panel up and to the left of the button —
		// which is what keeps it on screen from a bottom-rail anchor. Parented to the
		// shell so it floats over the plot instead of being clipped to the rail.
		HoverWidget *hover = new HoverWidget(cursorSettings, cursorBtn, it);
		hover->setAnchorPos(HoverPosition::HP_TOPRIGHT);
		hover->setContentPos(HoverPosition::HP_TOPLEFT);
		hover->setAnchorOffset(QPoint(0, -10));
		connect(cursorBtn, &QPushButton::toggled, hover, [hover](bool on) {
			hover->setVisible(on);
			// Later-created siblings (the measure and stats panels) stack above it
			// otherwise, and a settings panel behind the plot is unusable.
			hover->raise();
		});
	}

	// No plot channels are created here. The mechanism that decides which of the
	// pipeline's keys become channels is being reworked; until it exists the plot opens
	// empty, and AcqPlotManager::addChannel is the API whatever replaces it will call.
	if(m_fftProc) {
		// The timeline every horizontal measurement is divided by. From the FFT
		// processor because that is where the rate is configured — the source does not
		// publish one. Without it period and frequency come out in samples, which is
		// not wrong so much as unreadable.
		m_plots->setSampleRate(m_fftProc->sampleRate());
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
