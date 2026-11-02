/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#include "m2kspectrumcontroller.h"

#include "m2kanaloginsource.h"
#include "m2kcalibration.h"
#include "m2kcontext.h"

#include <core/acq_engine/acquisitionengine.h>
#include <core/acq_engine/datastore.h>
#include <core/acq_engine/genalyzerfftprocessor.h>
#include <core/acq_engine/snapshotsource.h>
#include <core/acq_engine/spectrumprocessor.h>
#include <core/acq_ui/acqaxis.h>
#include <core/acq_ui/acqcurvechannel.h>
#include <core/acq_ui/acqinstrument.h>
#include <core/acq_ui/acqmarkercontroller.h>
#include <core/acq_ui/acqplot.h>
#include <core/acq_ui/acqwaterfallchannel.h>
#include <core/acq_ui/filesourceblock.h>

#include <component/ping.h>

#include <gui/cursorcontroller.h>
#include <gui/filemanager.h>
#include <gui/docking/dockablearea.h>
#include <gui/docking/dockwrapper.h>
#include <gui/instrumenttemplate.h>
#include <gui/plot_utils.hpp>
#include <gui/plotaxis.h>
#include <gui/plotmarkercontroller.h>
#include <gui/plotnavigator.hpp>
#include <gui/plotwidget.h>
#include <gui/printplotmanager.h>
#include <gui/semiexclusivebuttongroup.h>
#include <gui/style.h>
#include <gui/waterfallplotwidget.h>
#include <gui/widgets/cursorsettings.h>
#include <gui/widgets/hoverwidget.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menucombo.h>
#include <gui/widgets/menucontrolbutton.h>
#include <gui/widgets/menuplotchannelcurvestylecontrol.h>
#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menuspinbox.h>
#include <gui/widgets/toolbuttons.h>
#include <pluginbase/statusbarmanager.h>

#include <QAbstractButton>
#include <QDesktopServices>
#include <QHBoxLayout>
#include <QLabel>
#include <QLoggingCategory>
#include <QLineF>
#include <QMouseEvent>
#include <QPushButton>
#include <QTextStream>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>

#include <qwt_plot.h>
#include <qwt_scale_map.h>

#include <algorithm>
#include <cmath>

Q_LOGGING_CATEGORY(CAT_M2K_SPECTRUM, "M2kSpectrumController")

using namespace scopy;
using namespace scopy::adalm2000;
using scopy::acq::DataKey;
using scopy::acq::SpectrumProcessor;

namespace {
const QString kGeneralGroup = QStringLiteral("General");
const QString kChannelsGroup = QStringLiteral("Channels");
const QString kReferencesGroup = QStringLiteral("References");

constexpr int kFrameIntervalMs = 16;
constexpr int kMaxFps = 60;
constexpr double kDbRange = 200.0;

const QList<QPair<QString, PlotMarkerController::MarkerTypes>> kMarkerTypes = {
	{QStringLiteral("None"), PlotMarkerController::MC_NONE},
	{QStringLiteral("Peak"), PlotMarkerController::MC_PEAK},
	{QStringLiteral("Fixed"), PlotMarkerController::MC_FIXED},
	{QStringLiteral("Single tone"), PlotMarkerController::MC_SINGLETONE},
};

const QList<QPair<QString, int>> kWindows = {
	{QStringLiteral("Hann"), GnWindowHann},
	{QStringLiteral("Blackman-Harris"), GnWindowBlackmanHarris},
	{QStringLiteral("Rectangular"), GnWindowNoWindow},
};

QString channelName(int ch) { return QStringLiteral("CH%1").arg(ch + 1); }
QString menuIdFor(int ch) { return QStringLiteral("ch%1").arg(ch); }

QPushButton *button(const QString &text, QWidget *parent)
{
	auto *b = new QPushButton(text, parent);
	Style::setStyle(b, style::properties::button::basicButton);
	return b;
}

gui::MenuSpinbox *hzSpin(const QString &name, double v, double min, QWidget *parent)
{
	auto *s =
		new gui::MenuSpinbox(name, v, QStringLiteral("Hz"), min, spec::MAX_STOP_HZ, true, false, true, parent);
	s->setIncrementMode(gui::MenuSpinbox::IS_125);
	return s;
}
} // namespace

M2kSpectrumController::M2kSpectrumController(ToolMenuEntry *tme, M2kContext *ctx, const QString &uri, QObject *parent)
	: AcqInstrumentController(tme, parent)
	, m_ctx(ctx)
	, m_resourceKey(QStringLiteral("adalm2000-adc:") + uri)
	, m_fftSize(spec::fftSizes().at(spec::DEFAULT_FFT_INDEX))
{
	m_ui->setRunGuard([this]() { return claim(); });
	m_ui->setBufferControlVisible(false);
	m_ui->engine()->setMaxFPS(kMaxFps);
	m_ui->shell()->setTargetFps(kMaxFps);
}

M2kSpectrumController::~M2kSpectrumController()
{
	AcqInstrumentController::stop();
	release();
	for(scopy::adc::AcqCurveChannel *c : m_curve) {
		delete c;
	}
	delete m_waterfall;
}

DataKey M2kSpectrumController::spectrumKey(int ch)
{
	return DataKey::withStage(QStringLiteral("spectrum"), M2kAnalogInSource::channelId(ch), QStringLiteral("mag"));
}

DataKey M2kSpectrumController::freqKey(int ch)
{
	return DataKey::withStage(QStringLiteral("fft"), M2kAnalogInSource::channelId(ch), QStringLiteral("freq"));
}

void M2kSpectrumController::setupBlocks(scopy::component::Context *ctx)
{
	Q_UNUSED(ctx)
	scopy::acq::AcquisitionEngine *engine = m_ui->engine();
	InstrumentTemplate *it = m_ui->shell();

	it->addChannelGroup(kGeneralGroup);
	it->addChannelGroup(kChannelsGroup);

	m_source = new M2kAnalogInSource(m_ctx, engine);
	engine->addSource(m_source);
	m_file = new scopy::adc::FileSourceBlock(QStringLiteral("import"), engine);
	engine->addSource(m_file);
	m_ui->setBufferSize(m_fftSize);
	m_source->setOversampling(spec::oversamplingFor(m_span.stop));
}

void M2kSpectrumController::setupProcessors()
{
	scopy::acq::AcquisitionEngine *engine = m_ui->engine();
	for(int ch = 0; ch < CHANNELS; ++ch) {
		const DataKey dbKey = DataKey::withStage(QStringLiteral("fft"), M2kAnalogInSource::channelId(ch),
							 QStringLiteral("db"));
		m_fft[ch] = new scopy::acq::GenalyzerFFTProcessor(m_source->rawKey(ch), dbKey, freqKey(ch), m_fftSize,
								  m_source->sampleRate(), GnWindowHann, engine);
		m_fft[ch]->setAveragingStore(m_ui->store());
		engine->addProcessor(m_fft[ch]);

		m_spectrum[ch] = new SpectrumProcessor(QStringLiteral("spectrum%1").arg(ch), dbKey, spectrumKey(ch),
						       m_fft[ch], engine);
		engine->addProcessor(m_spectrum[ch]);

		connect(
			m_source, &M2kAnalogInSource::fullScaleChanged, m_spectrum[ch],
			[proc = m_spectrum[ch], ch](int c, double v) {
				if(c == ch) {
					proc->setFullScaleVolts(v);
				}
			},
			Qt::DirectConnection);
	}
}

void M2kSpectrumController::setupPlots()
{
	setupCenter();
	setupChannelRows();
	setupTopRail();

	InstrumentTemplate *it = m_ui->shell();
	MenuControlButton *general = it->addChannelRow(it->addChannelGroup(kGeneralGroup), tr("General settings"),
						       QColor(), QStringLiteral("general"));
	general->checkBox()->setVisible(false);
	it->addMenuPage(QStringLiteral("general"), buildGeneralPage());

	addHoverButton(tr("Markers"), buildMarkersPage());
	addHoverButton(tr("Cursors"), buildCursorsPage());
	MenuSectionCollapseWidget *refs = it->addChannelGroup(kReferencesGroup);
	MenuControlButton *import = it->addChannelRow(refs, tr("Import file"), QColor(), QStringLiteral("import"));
	import->checkBox()->setVisible(false);
	auto *importPage = new QWidget();
	auto *importLay = new QVBoxLayout(importPage);
	importLay->setContentsMargins(0, 0, 0, 0);
	importLay->addWidget(m_file->settingsWidget());
	importLay->addStretch();
	it->addMenuPage(QStringLiteral("import"), importPage);
	connect(m_file, &scopy::adc::FileSourceBlock::slotsChanged, this,
		&M2kSpectrumController::syncImportedReferences);
	connect(m_file, &scopy::adc::FileSourceBlock::published, this,
		&M2kSpectrumController::syncImportedReferences);

	connect(m_ui, &scopy::adc::AcqInstrument::started, this, &M2kSpectrumController::onStarted);
	connect(m_ui, &scopy::adc::AcqInstrument::cycleComplete, this, &M2kSpectrumController::onCycleComplete);
	connect(m_ui, &scopy::adc::AcqInstrument::stopped, this, &M2kSpectrumController::onStopped);
	connect(m_source, &M2kAnalogInSource::sampleRateChanged, this, [this]() {
		m_xPending = true;
		refreshSweepReadouts();
	});

	setupCalibrationGate();
	setSpan(m_span);
	onUnitsChanged(0);
	general->click();
}

void M2kSpectrumController::addHoverButton(const QString &label, QWidget *content)
{
	InstrumentTemplate *it = m_ui->shell();
	auto *btn = new QPushButton(label, it);
	btn->setCheckable(true);
	Style::setStyle(btn, style::properties::button::blueGrayButton);
	Style::setStyle(btn, style::properties::label::menuMedium);
	it->tool()->addWidgetToBottomContainerHelper(btn, TTA_RIGHT);
	if(!m_hoverGroup) {
		m_hoverGroup = new SemiExclusiveButtonGroup(this);
	}
	m_hoverGroup->addButton(btn);

	content->setMinimumWidth(Style::getDimension(json::global::unit_6) * 2 +
				 Style::getDimension(json::global::unit_4));
	Style::setBackgroundColor(content, json::theme::background_primary);
	auto *hover = new HoverWidget(content, btn, it);
	hover->setStyleSheet(QStringLiteral("scopy--HoverWidget { background-color: transparent; }"));
	hover->setAnchorPos(HP_TOPRIGHT);
	hover->setContentPos(HP_TOPLEFT);
	hover->setVisible(false);
	connect(btn, &QAbstractButton::toggled, hover, [hover](bool on) {
		hover->setVisible(on);
		hover->raise();
	});
}

void M2kSpectrumController::setupCenter()
{
	InstrumentTemplate *it = m_ui->shell();
	auto *center = new QWidget(m_ui);
	auto *lay = new QVBoxLayout(center);
	lay->setContentsMargins(0, 0, 0, 0);

	m_specPlot = new scopy::adc::AcqPlot(tr("Spectrum"), scopy::adc::AcqPlotKind::Basic, 0, center);
	m_wfPlot = new scopy::adc::AcqPlot(tr("Waterfall"), scopy::adc::AcqPlotKind::Waterfall, 1, center);
	DockableAreaInterface *area = createDockableArea(center);
	Style::setBackgroundColor(area->asWidget(), json::theme::background_subtle, true);
	DockWrapperInterface *specDock = createDockWrapper(m_specPlot->name());
	specDock->setTitle(m_specPlot->name());
	specDock->setInnerWidget(m_specPlot->plot());
	area->addDockWrapper(specDock, DockableAreaInterface::Direction_BOTTOM);
	m_wfDock = createDockWrapper(m_wfPlot->name());
	m_wfDock->setTitle(m_wfPlot->name());
	m_wfDock->setInnerWidget(m_wfPlot->plot());
	area->addDockWrapper(m_wfDock, DockableAreaInterface::Direction_BOTTOM);
	m_wfDock->setActivated(false);
	lay->addWidget(area->asWidget());
	it->setCenterWidget(center);
	m_specPlot->plot()->plot()->canvas()->installEventFilter(this);

	m_markerPanel = new MarkerPanel(it);
	m_markerPanel->setVisible(false);
	it->addToSlot(PS_BOTTOM, m_markerPanel);

	for(int ch = 0; ch < CHANNELS; ++ch) {
		scopy::adc::AcqChannel::Args args;
		args.store = m_ui->store();
		args.engine = m_ui->engine();
		args.key = spectrumKey(ch);
		args.xKey = freqKey(ch);
		args.info = m_ui->engine()->streamInfo(args.key).value_or(scopy::acq::StreamInfo{});
		args.info.kind = scopy::acq::ReprKind::Curve;
		args.info.colorIndex = ch;
		args.uid = m_uid++;
		m_curve[ch] = new scopy::adc::AcqCurveChannel(args);
		m_curve[ch]->setName(channelName(ch));
		attachChannel(m_curve[ch], m_specPlot);
		m_curve[ch]->xAxis()->setSourceFixed(true);
		m_curve[ch]->yAxis()->setSourceFixed(true);
		m_curve[ch]->xAxis()->setUnit(QStringLiteral("Hz"));

		auto *mc = new scopy::adc::AcqMarkerController(m_curve[ch], this);
		mc->bind();
		mc->setEnabled(true);
		m_markers[ch] = mc;
		const QString name = channelName(ch);
		connect(mc, &PlotMarkerController::markerInfoUpdated, this, [this, mc, name, ch]() {
			if(ch == m_selCh && m_selFreqSpin && m_markerType == PlotMarkerController::MC_FIXED) {
				refreshSelectedMarker();
			}
			if(m_markerPanel) {
				m_markerPanel->updateChannel(
					name,
					m_deltaTable ? scopy::adc::AcqMarkerController::deltaInfo(mc->markerInfo())
						     : mc->markerInfo());
			}
		});
		connect(mc, &PlotMarkerController::markerEnabled, this, [this, ch, name](bool on) {
			if(!m_markerPanel) {
				return;
			}
			m_markerPanel->deleteChannel(name);
			if(on && m_chEnabled[ch]) {
				m_markerPanel->newChannel(name, QPen(m_curve[ch]->color()));
			}
			m_markerPanel->setVisible(m_markerPanel->markerCount() != 0);
		});
	}
	rebuildWaterfall();

	PlotNavigator::syncXNavigators(m_specPlot->plot()->navigator(), m_wfPlot->plot()->navigator());
	CursorController::syncXCursorControllers(m_specPlot->cursors(), m_wfPlot->cursors());
	m_specPlot->plot()->selectChannel(m_curve[0]->plotChannel());

	m_frameTimer = new QTimer(this);
	m_frameTimer->setInterval(kFrameIntervalMs);
	connect(m_frameTimer, &QTimer::timeout, this, [this]() {
		if(!m_dirty) {
			return;
		}
		m_dirty = false;
		m_specPlot->replot();
		if(m_waterfallOn) {
			m_wfPlot->replot();
		}
	});
}

QList<scopy::adc::AcqCurveChannel *> M2kSpectrumController::curves() const
{
	QList<scopy::adc::AcqCurveChannel *> out{m_curve[0], m_curve[1]};
	for(const Reference &r : m_refs) {
		out.append(r.curve);
	}
	return out;
}

QList<scopy::adc::AcqChannel *> M2kSpectrumController::allChannels() const
{
	QList<scopy::adc::AcqChannel *> out{m_waterfall};
	for(scopy::adc::AcqCurveChannel *c : curves()) {
		out.append(c);
	}
	return out;
}

void M2kSpectrumController::attachChannel(scopy::adc::AcqChannel *ch, scopy::adc::AcqPlot *plot)
{
	plot->addChannelRef(ch);
	ch->attach(plot);
	ch->reclaimDepth(bins());
	ch->setKeyPresent(true);
}

void M2kSpectrumController::rebuildWaterfall()
{
	int rows = 200;
	if(auto *old = qobject_cast<scopy::adc::AcqWaterfallChannel *>(m_waterfall)) {
		rows = old->rows();
		m_wfPlot->removeChannelRef(m_waterfall);
		delete m_waterfall;
		m_waterfall = nullptr;
	}
	const int ch = m_waterfallCh;
	scopy::adc::AcqChannel::Args args;
	args.store = m_ui->store();
	args.engine = m_ui->engine();
	args.key = spectrumKey(ch);
	args.xKey = freqKey(ch);
	args.info = m_ui->engine()->streamInfo(args.key).value_or(scopy::acq::StreamInfo{});
	args.info.kind = scopy::acq::ReprKind::Waterfall;
	args.info.colorIndex = ch;
	args.uid = m_uid++;
	auto *wf = new scopy::adc::AcqWaterfallChannel(args);
	wf->setRows(rows);
	m_waterfall = wf;
	attachChannel(m_waterfall, m_wfPlot);
	if(m_waterfall->xAxis()) {
		m_waterfall->xAxis()->setUnit(QStringLiteral("Hz"));
	}
	if(m_waterfall->yAxis()) {
		m_waterfall->yAxis()->setUnit(QStringLiteral("s"));
	}
	setWaterfallEnabled(m_waterfallOn);
	applyYRange();
}

void M2kSpectrumController::setWaterfallEnabled(bool on)
{
	m_waterfallOn = on;
	m_waterfall->setEnabled(on);
	m_wfDock->setActivated(on);
	if(on) {
		m_waterfall->reclaimDepth(bins());
	} else {
		m_ui->store()->releaseClaimant(m_waterfall->claimant());
	}
	m_xPending = true;
}

QWidget *M2kSpectrumController::page(MenuSectionCollapseWidget *section)
{
	auto *w = new QWidget();
	auto *lay = new QVBoxLayout(w);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->addWidget(section);
	lay->addStretch();
	return w;
}

void M2kSpectrumController::setupChannelRows()
{
	InstrumentTemplate *it = m_ui->shell();
	MenuSectionCollapseWidget *group = it->addChannelGroup(kChannelsGroup);
	for(int ch = 0; ch < CHANNELS; ++ch) {
		MenuControlButton *row = it->addChannelRow(group, channelName(ch), m_curve[ch]->color(), menuIdFor(ch));
		it->addMenuPage(menuIdFor(ch), buildChannelPage(ch));
		row->checkBox()->setChecked(true);
		connect(row->checkBox(), &QAbstractButton::toggled, this,
			[this, ch](bool on) { applyChannelEnabled(ch, on); });
		m_chRow[ch] = row;
	}
}

void M2kSpectrumController::setupTopRail()
{
	InstrumentTemplate *it = m_ui->shell();

	m_progressLabel = new QLabel(it);
	Style::setStyle(m_progressLabel, style::properties::label::subtle);
	m_progressLabel->setVisible(false);
	it->addEngineControl(m_progressLabel);
	m_progressTimer = new QTimer(this);
	m_progressTimer->setInterval(100);
	connect(m_progressTimer, &QTimer::timeout, this, &M2kSpectrumController::updateProgress);

	connect(it->infoBtn(), &QAbstractButton::clicked, this, []() {
		QDesktopServices::openUrl(QUrl(
			QStringLiteral("https://analogdevicesinc.github.io/scopy/plugins/m2k/spectrum_analyzer.html")));
	});
	auto *printManager = new PrintPlotManager(this);
	connect(it->printBtn(), &QAbstractButton::clicked, this, [this, printManager]() {
		QList<PlotWidget *> plots{m_specPlot->plot()};
		if(m_waterfallOn) {
			plots.append(m_wfPlot->plot());
		}
		printManager->printPlots(plots, QStringLiteral("Spectrum Analyzer"));
	});
	connect(it->exportBtn(), &QAbstractButton::clicked, this, &M2kSpectrumController::exportCsv);
}

QWidget *M2kSpectrumController::buildChannelPage(int ch)
{
	InstrumentTemplate *it = m_ui->shell();
	MenuSectionCollapseWidget *section = it->createMenuSection(channelName(ch), SO_CH);

	auto *typeCombo = new MenuCombo(tr("Averaging"), section);
	typeCombo->combo()->addItems(SpectrumProcessor::averagingNames());
	auto *history = new gui::MenuSpinbox(tr("History"), m_spectrum[ch]->history(), tr("frames"), 1, 10000, true,
					     false, false, section);
	history->setIncrementMode(gui::MenuSpinbox::IS_FIXED);
	history->setVisible(false);

	auto *windowCombo = new MenuCombo(tr("Window"), section);
	for(const auto &w : kWindows) {
		windowCombo->combo()->addItem(w.first, w.second);
	}

	auto *rangeCombo = new MenuCombo(tr("Range"), section);
	rangeCombo->combo()->addItems({QStringLiteral("±25 V"), QStringLiteral("±2.5 V")});

	auto *style = new gui::MenuPlotChannelCurveStyleControl(section);
	style->addChannels(m_curve[ch]->plotChannel());

	QPushButton *snap = button(tr("Snapshot as reference"), section);

	section->contentLayout()->addWidget(typeCombo);
	section->contentLayout()->addWidget(history);
	section->contentLayout()->addWidget(windowCombo);
	section->contentLayout()->addWidget(rangeCombo);
	section->contentLayout()->addWidget(style);
	section->contentLayout()->addWidget(snap);
	m_chUi[ch] = {typeCombo, history, windowCombo, rangeCombo};

	connect(typeCombo->combo(), &QComboBox::currentIndexChanged, this, [this, ch, history](int i) {
		const auto mode = static_cast<SpectrumProcessor::Averaging>(i);
		m_spectrum[ch]->setAveraging(mode);
		history->setVisible(SpectrumProcessor::usesHistory(mode));
	});
	connect(history, &gui::MenuSpinbox::valueChanged, this,
		[this, ch](double v) { m_spectrum[ch]->setHistory(static_cast<int>(v)); });
	connect(windowCombo->combo(), &QComboBox::currentIndexChanged, this, [this, ch, windowCombo](int i) {
		m_fft[ch]->setWindow(static_cast<GnWindow>(windowCombo->combo()->itemData(i).toInt()));
	});
	connect(rangeCombo->combo(), &QComboBox::currentIndexChanged, this,
		[this, ch](int i) { m_source->setLowRange(ch, i == 1); });
	connect(snap, &QPushButton::clicked, this, [this, ch]() { snapshot(ch); });
	return page(section);
}

QWidget *M2kSpectrumController::buildGeneralPage()
{
	InstrumentTemplate *it = m_ui->shell();
	auto *w = new QWidget();
	auto *lay = new QVBoxLayout(w);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(10);

	MenuSectionCollapseWidget *freq = it->createMenuSection(tr("FREQUENCY"), SO_DEV, w);
	m_startSpin = hzSpin(tr("Start"), m_span.start, 0, freq);
	m_stopSpin = hzSpin(tr("Stop"), m_span.stop, 0, freq);
	m_centerSpin = hzSpin(tr("Center"), m_span.center(), 0, freq);
	m_spanSpin = hzSpin(tr("Span"), m_span.width(), 0, freq);
	m_rbwCombo = new MenuCombo(tr("Resolution BW"), freq);
	m_rateLabel = new QLabel(freq);
	Style::setStyle(m_rateLabel, style::properties::label::subtle);
	m_logSwitch = new MenuOnOffSwitch(tr("Logarithmic"), freq, false);
	for(QWidget *c : std::initializer_list<QWidget *>{m_startSpin, m_stopSpin, m_centerSpin, m_spanSpin, m_rbwCombo,
							  m_rateLabel, m_logSwitch}) {
		freq->contentLayout()->addWidget(c);
	}
	lay->addWidget(freq);
	connect(m_logSwitch->onOffswitch(), &QAbstractButton::toggled, this, &M2kSpectrumController::setLogFrequency);

	auto minSpan = [this]() { return spec::minSpan(m_source->sampleRate(), m_fftSize); };
	connect(m_startSpin, &gui::MenuSpinbox::valueChanged, this,
		[this, minSpan](double v) { setSpan(spec::withStart(m_span, v, minSpan())); });
	connect(m_stopSpin, &gui::MenuSpinbox::valueChanged, this,
		[this, minSpan](double v) { setSpan(spec::withStop(m_span, v, minSpan())); });
	connect(m_centerSpin, &gui::MenuSpinbox::valueChanged, this,
		[this, minSpan](double v) { setSpan(spec::withCenter(m_span, v, minSpan())); });
	connect(m_spanSpin, &gui::MenuSpinbox::valueChanged, this,
		[this, minSpan](double v) { setSpan(spec::withWidth(m_span, v, minSpan())); });
	connect(m_rbwCombo->combo(), &QComboBox::currentIndexChanged, this, [this](int i) {
		if(i >= 0) {
			setFftSize(spec::fftSizes().at(i));
		}
	});

	MenuSectionCollapseWidget *amp = it->createMenuSection(tr("AMPLITUDE"), SO_VIEW, w);
	m_unitsCombo = new MenuCombo(tr("Units"), amp);
	m_unitsCombo->combo()->addItems(SpectrumProcessor::unitNames());
	m_topSpin = new gui::MenuSpinbox(tr("Top"), 0, QString(), -300, 300, true, false, true, amp);
	m_bottomSpin = new gui::MenuSpinbox(tr("Bottom"), -kDbRange, QString(), -300, 300, true, false, true, amp);
	m_topSpin->setIncrementMode(gui::MenuSpinbox::IS_FIXED);
	m_bottomSpin->setIncrementMode(gui::MenuSpinbox::IS_FIXED);
	amp->contentLayout()->addWidget(m_unitsCombo);
	amp->contentLayout()->addWidget(m_topSpin);
	amp->contentLayout()->addWidget(m_bottomSpin);
	lay->addWidget(amp);
	connect(m_unitsCombo->combo(), &QComboBox::currentIndexChanged, this, &M2kSpectrumController::onUnitsChanged);
	connect(m_topSpin, &gui::MenuSpinbox::valueChanged, this, [this]() { applyYRange(); });
	connect(m_bottomSpin, &gui::MenuSpinbox::valueChanged, this, [this]() { applyYRange(); });

	MenuSectionCollapseWidget *wfSection = it->createMenuSection(tr("WATERFALL"), SO_VIEW, w);
	m_wfSwitch = new MenuOnOffSwitch(tr("Waterfall"), wfSection, false);
	m_wfChannelCombo = new MenuCombo(tr("Channel"), wfSection);
	for(int ch = 0; ch < CHANNELS; ++ch) {
		m_wfChannelCombo->combo()->addItem(channelName(ch));
	}
	auto *wfRows =
		new gui::MenuSpinbox(tr("History rows"), 200, QString(), 10, 2000, true, false, false, wfSection);
	wfRows->setIncrementMode(gui::MenuSpinbox::IS_FIXED);
	wfSection->contentLayout()->addWidget(m_wfSwitch);
	wfSection->contentLayout()->addWidget(m_wfChannelCombo);
	wfSection->contentLayout()->addWidget(wfRows);
	lay->addWidget(wfSection);
	lay->addStretch();

	connect(m_wfSwitch->onOffswitch(), &QAbstractButton::toggled, this,
		&M2kSpectrumController::setWaterfallEnabled);
	connect(m_wfChannelCombo->combo(), &QComboBox::currentIndexChanged, this, [this](int i) {
		m_waterfallCh = i;
		rebuildWaterfall();
	});
	connect(wfRows, &gui::MenuSpinbox::valueChanged, this, [this](double rows) {
		if(auto *wf = qobject_cast<scopy::adc::AcqWaterfallChannel *>(m_waterfall)) {
			wf->setRows(static_cast<int>(rows));
		}
	});
	return w;
}

QWidget *M2kSpectrumController::buildMarkersPage()
{
	MenuSectionCollapseWidget *section = m_ui->shell()->createMenuSection(tr("MARKERS"), SO_VIEW);
	m_markerTypeCombo = new MenuCombo(tr("Marker type"), section);
	for(const auto &t : kMarkerTypes) {
		m_markerTypeCombo->combo()->addItem(t.first, static_cast<int>(t.second));
	}
	m_markerCountSpin =
		new gui::MenuSpinbox(tr("Marker count"), 5, tr("markers"), 1, 10, true, false, false, section);
	m_markerCountSpin->setIncrementMode(gui::MenuSpinbox::IS_FIXED);
	auto *delta = new MenuOnOffSwitch(tr("Delta to first marker"), section, false);
	section->contentLayout()->addWidget(m_markerTypeCombo);
	section->contentLayout()->addWidget(m_markerCountSpin);
	section->contentLayout()->addWidget(delta);

	m_selSection = new QWidget(section);
	auto *selLay = new QVBoxLayout(m_selSection);
	selLay->setContentsMargins(0, 0, 0, 0);
	m_selChCombo = new MenuCombo(tr("Channel"), m_selSection);
	for(int ch = 0; ch < CHANNELS; ++ch) {
		m_selChCombo->combo()->addItem(channelName(ch));
	}
	m_selMarkerCombo = new MenuCombo(tr("Marker"), m_selSection);
	m_selFreqSpin = hzSpin(tr("Frequency"), 0, 0, m_selSection);
	auto *steps = new QWidget(m_selSection);
	auto *stepsLay = new QHBoxLayout(steps);
	stepsLay->setContentsMargins(0, 0, 0, 0);
	const QList<QPair<QString, scopy::adc::AcqMarkerController::PeakStep>> stepButtons = {
		{QStringLiteral("◀"), scopy::adc::AcqMarkerController::PeakStep::Left},
		{QStringLiteral("▶"), scopy::adc::AcqMarkerController::PeakStep::Right},
		{QStringLiteral("▲"), scopy::adc::AcqMarkerController::PeakStep::Higher},
		{QStringLiteral("▼"), scopy::adc::AcqMarkerController::PeakStep::Lower},
		{tr("Max"), scopy::adc::AcqMarkerController::PeakStep::Highest},
	};
	for(const auto &[label, step] : stepButtons) {
		QPushButton *b = button(label, steps);
		stepsLay->addWidget(b);
		connect(b, &QPushButton::clicked, this,
			[this, step = step]() { m_markers[m_selCh]->stepToPeak(m_selMarker, step); });
	}
	selLay->addWidget(m_selChCombo);
	selLay->addWidget(m_selMarkerCombo);
	selLay->addWidget(m_selFreqSpin);
	selLay->addWidget(steps);
	m_selSection->setVisible(false);
	section->contentLayout()->addWidget(m_selSection);

	connect(m_selChCombo->combo(), &QComboBox::currentIndexChanged, this,
		[this](int i) { selectMarker(i, m_selMarker); });
	connect(m_selMarkerCombo->combo(), &QComboBox::currentIndexChanged, this,
		[this](int i) { selectMarker(m_selCh, i); });
	connect(m_selFreqSpin, &gui::MenuSpinbox::valueChanged, this,
		[this](double v) { m_markers[m_selCh]->moveFixedMarker(m_selMarker, v); });

	connect(m_markerTypeCombo->combo(), &QComboBox::currentIndexChanged, this,
		[this](int i) { setMarkerType(m_markerTypeCombo->combo()->itemData(i).toInt()); });
	connect(m_markerCountSpin, &gui::MenuSpinbox::valueChanged, this,
		[this](double v) { setMarkerCount(static_cast<int>(v)); });
	connect(delta->onOffswitch(), &QAbstractButton::toggled, this, [this](bool on) {
		m_deltaTable = on;
		for(scopy::adc::AcqMarkerController *mc : m_markers) {
			mc->computeMarkers();
		}
	});
	return page(section);
}

QWidget *M2kSpectrumController::buildCursorsPage()
{
	MenuSectionCollapseWidget *section = m_ui->shell()->createMenuSection(tr("CURSORS"), SO_VIEW);
	CursorSettings *settings = m_specPlot->cursorSettings(section);
	m_wfPlot->cursors()->connectSignals(settings);
	settings->getXEn()->setChecked(false);
	settings->getYEn()->setChecked(false);
	m_specPlot->setShowCursors(true);
	m_wfPlot->setShowCursors(true);
	section->contentLayout()->addWidget(settings);
	return page(section);
}

void M2kSpectrumController::setupCalibrationGate()
{
	auto *calibration = m_ctx ? m_ctx->findChild<M2kCalibration *>() : nullptr;
	if(!calibration) {
		return;
	}
	auto gate = [this](bool calibrating) {
		m_calibrating = calibrating;
		if(calibrating) {
			AcqInstrumentController::stop();
		}
		updateRunEnabled();
	};
	connect(calibration, &M2kCalibration::started, this, [gate]() { gate(true); });
	connect(calibration, &M2kCalibration::finished, this, [gate](bool) { gate(false); });
	gate(calibration->isRunning());
}

void M2kSpectrumController::onStarted()
{
	for(scopy::adc::AcqChannel *c : allChannels()) {
		c->refreshSampleRate();
		c->reset();
	}
	m_xPending = true;
	m_frameTimer->start();
	m_cycleClock.start();
	m_progressTimer->start();
}

void M2kSpectrumController::updateProgress()
{
	const double period = m_fftSize / m_source->sampleRate();
	m_progressLabel->setVisible(period >= 0.5);
	const int pct = std::min(99, static_cast<int>(100.0 * m_cycleClock.elapsed() / (period * 1000.0)));
	m_progressLabel->setText(tr("Acquiring: %1 %").arg(pct));
}

void M2kSpectrumController::onCycleComplete()
{
	for(int ch = 0; ch < CHANNELS; ++ch) {
		if(m_chEnabled[ch]) {
			m_curve[ch]->pull(bins());
		}
	}
	m_cycleClock.restart();
	for(const Reference &r : std::as_const(m_refs)) {
		r.curve->pull(r.size);
	}
	m_waterfall->pull(bins());
	if(m_xPending) {
		m_xPending = false;
		applyXRange();
	}
	for(int ch = 0; ch < CHANNELS; ++ch) {
		m_markers[ch]->computeMarkers();
	}
	m_dirty = true;
}

void M2kSpectrumController::onStopped()
{
	m_frameTimer->stop();
	m_progressTimer->stop();
	m_progressLabel->setVisible(false);
	for(scopy::adc::AcqChannel *c : allChannels()) {
		c->onStopped();
	}
	m_specPlot->replot();
	m_wfPlot->replot();
	if(!m_ui->engine()->isRunning()) {
		release();
	}
}

void M2kSpectrumController::restartIfRunning(const std::function<void()> &change)
{
	const bool running = m_ui->engine()->isRunning();
	if(running) {
		m_ui->stop();
	}
	change();
	if(running) {
		m_ui->run();
	}
}

void M2kSpectrumController::setChannelEnabled(int ch, bool on) { m_chRow[ch]->checkBox()->setChecked(on); }

void M2kSpectrumController::applyChannelEnabled(int ch, bool on)
{
	m_chEnabled[ch] = on;
	m_source->enableChannel(M2kAnalogInSource::channelId(ch), on);
	m_fft[ch]->setEnabled(on);
	m_spectrum[ch]->setEnabled(on);
	m_curve[ch]->setEnabled(on);
	m_markers[ch]->setEnabled(on);
	updateRunEnabled();
	m_specPlot->replot();
}

void M2kSpectrumController::updateRunEnabled()
{
	bool any = false;
	for(int ch = 0; ch < CHANNELS; ++ch) {
		any |= m_chEnabled[ch];
	}
	const bool can = any && !m_calibrating;
	m_ui->shell()->runBtn()->setEnabled(can);
	m_ui->shell()->singleBtn()->setEnabled(can);
	if(m_tme) {
		m_tme->setRunEnabled(can);
	}
	if(!any) {
		AcqInstrumentController::stop();
	}
}

void M2kSpectrumController::setSpan(const spec::Span &s)
{
	m_span = s;
	m_startSpin->setValueSilent(s.start);
	m_stopSpin->setValueSilent(s.stop);
	m_centerSpin->setValueSilent(s.center());
	m_spanSpin->setValueSilent(s.width());

	m_source->setOversampling(spec::oversamplingFor(s.stop));
	applyXRange();
	m_xPending = true;
	refreshSweepReadouts();
	m_specPlot->replot();
}

void M2kSpectrumController::applyXRange()
{
	const double firstBin = spec::rbw(m_source->sampleRate(), m_fftSize);
	const double start = m_logX ? std::max(m_span.start, firstBin) : m_span.start;
	for(scopy::adc::AcqCurveChannel *c : curves()) {
		c->xAxis()->plotAxis()->setLogScale(m_logX);
		c->xAxis()->setInterval(start, m_span.stop);
	}
	m_wfPlot->plot()->xAxis()->setLogScale(m_logX);
	m_wfPlot->plot()->xAxis()->setInterval(start, m_span.stop);
}

void M2kSpectrumController::setLogFrequency(bool on)
{
	m_logX = on;
	applyXRange();
	m_specPlot->replot();
	m_wfPlot->replot();
}

void M2kSpectrumController::setFftSize(int n)
{
	if(n == m_fftSize) {
		return;
	}
	restartIfRunning([this, n]() {
		m_fftSize = n;
		m_ui->setBufferSize(n);
		for(scopy::adc::AcqCurveChannel *c : m_curve) {
			c->reclaimDepth(bins());
		}
		setWaterfallEnabled(m_waterfallOn);
		m_xPending = true;
		refreshSweepReadouts();
	});
}

void M2kSpectrumController::refreshSweepReadouts()
{
	if(!m_rbwCombo || !m_rateLabel) {
		return;
	}
	const double sr = m_source->sampleRate();
	MetricPrefixFormatter fmt;
	{
		QSignalBlocker b(m_rbwCombo->combo());
		m_rbwCombo->combo()->clear();
		for(int n : spec::fftSizes()) {
			m_rbwCombo->combo()->addItem(fmt.format(spec::rbw(sr, n), QStringLiteral("Hz"), 2));
		}
		m_rbwCombo->combo()->setCurrentIndex(spec::fftSizes().indexOf(m_fftSize));
	}
	m_rateLabel->setText(tr("Sample rate %1, %2 bins").arg(fmt.format(sr, QStringLiteral("SPS"), 3)).arg(bins()));
}

QString M2kSpectrumController::unitSymbol() const
{
	return SpectrumProcessor::unitSymbol(
		static_cast<SpectrumProcessor::Unit>(m_unitsCombo->combo()->currentIndex()));
}

void M2kSpectrumController::onUnitsChanged(int idx)
{
	const auto unit = static_cast<SpectrumProcessor::Unit>(idx);
	for(SpectrumProcessor *s : m_spectrum) {
		s->setUnit(unit);
	}
	double top = 0.0, bottom = -kDbRange;
	switch(unit) {
	case SpectrumProcessor::Unit::DBFS:
		break;
	case SpectrumProcessor::Unit::DBV:
	case SpectrumProcessor::Unit::DBU: {
		const double vrms = m_spectrum[0]->fullScaleVolts() / M_SQRT2;
		const double ref = unit == SpectrumProcessor::Unit::DBV ? 1.0 : 0.7745966692;
		top = std::ceil(20.0 * std::log10(vrms / ref) / 10.0) * 10.0;
		bottom = top - kDbRange;
		break;
	}
	case SpectrumProcessor::Unit::VPeak:
	case SpectrumProcessor::Unit::VRms:
		top = 10.0;
		bottom = 0.0;
		break;
	case SpectrumProcessor::Unit::VRmsPerSqrtHz:
		top = 1.0;
		bottom = 1e-9;
		break;
	}
	m_logY = unit == SpectrumProcessor::Unit::VRmsPerSqrtHz;
	const QString sym = unitSymbol();
	m_topSpin->setUnit(sym);
	m_bottomSpin->setUnit(sym);
	m_topSpin->setValueSilent(top);
	m_bottomSpin->setValueSilent(bottom);
	for(scopy::adc::AcqCurveChannel *c : curves()) {
		c->yAxis()->setUnit(sym);
	}
	applyYRange();
}

void M2kSpectrumController::applyYRange()
{
	if(!m_topSpin || !m_bottomSpin || !m_curve[0]) {
		return;
	}
	const double top = m_topSpin->value();
	const double bottom = m_bottomSpin->value();
	if(!(top > bottom) || (m_logY && bottom <= 0.0)) {
		return;
	}
	for(scopy::adc::AcqCurveChannel *c : curves()) {
		c->yAxis()->setAutoscale(false);
		c->yAxis()->plotAxis()->setLogScale(m_logY);
		c->yAxis()->setInterval(bottom, top);
	}
	if(auto *wf = qobject_cast<WaterfallPlotWidget *>(m_wfPlot->plot()); wf && unitSymbol().startsWith('d')) {
		wf->setIntensityRange(bottom, top);
	}
	m_specPlot->replot();
}

void M2kSpectrumController::setMarkerType(int type)
{
	m_markerType = type;
	for(scopy::adc::AcqMarkerController *mc : m_markers) {
		mc->setMarkerType(static_cast<PlotMarkerController::MarkerTypes>(type));
	}
	m_selSection->setVisible(type == PlotMarkerController::MC_FIXED);
	selectMarker(m_selCh, m_selMarker);
}

void M2kSpectrumController::selectMarker(int ch, int idx)
{
	const int count = static_cast<int>(m_markerCountSpin->value());
	m_selCh = std::clamp(ch, 0, CHANNELS - 1);
	m_selMarker = std::clamp(idx, 0, count - 1);
	QSignalBlocker chBlock(m_selChCombo->combo());
	QSignalBlocker markerBlock(m_selMarkerCombo->combo());
	m_selChCombo->combo()->setCurrentIndex(m_selCh);
	if(m_selMarkerCombo->combo()->count() != count) {
		m_selMarkerCombo->combo()->clear();
		for(int i = 0; i < count; ++i) {
			m_selMarkerCombo->combo()->addItem(QStringLiteral("F%1").arg(i));
		}
	}
	m_selMarkerCombo->combo()->setCurrentIndex(m_selMarker);
	refreshSelectedMarker();
}

void M2kSpectrumController::refreshSelectedMarker()
{
	const QList<PlotMarkerController::MarkerInfo> &info = m_markers[m_selCh]->markerInfo();
	if(m_selMarker < info.size()) {
		m_selFreqSpin->setValueSilent(info[m_selMarker].peak.x);
	}
}

bool M2kSpectrumController::eventFilter(QObject *watched, QEvent *event)
{
	if(event->type() != QEvent::MouseButtonPress || m_markerType != PlotMarkerController::MC_FIXED) {
		return AcqInstrumentController::eventFilter(watched, event);
	}
	const QPointF click = static_cast<QMouseEvent *>(event)->position();
	QwtPlot *plot = m_specPlot->plot()->plot();
	double best = 10.0;
	for(int ch = 0; ch < CHANNELS; ++ch) {
		if(!m_chEnabled[ch]) {
			continue;
		}
		const QwtScaleMap xMap = plot->canvasMap(m_curve[ch]->xAxis()->plotAxis()->axisId());
		const QwtScaleMap yMap = plot->canvasMap(m_curve[ch]->yAxis()->plotAxis()->axisId());
		const QList<PlotMarkerController::MarkerInfo> &info = m_markers[ch]->markerInfo();
		for(int i = 0; i < info.size(); ++i) {
			const QPointF p(xMap.transform(info[i].peak.x), yMap.transform(info[i].peak.y));
			const double d = QLineF(p, click).length();
			if(d < best) {
				best = d;
				selectMarker(ch, i);
			}
		}
	}
	return AcqInstrumentController::eventFilter(watched, event);
}

void M2kSpectrumController::setMarkerCount(int n)
{
	for(scopy::adc::AcqMarkerController *mc : m_markers) {
		mc->setNrOfMarkers(n);
		mc->computeMarkers();
	}
	selectMarker(m_selCh, m_selMarker);
}

void M2kSpectrumController::addReference(const QString &name, const DataKey &key, const DataKey &xKey, int size,
					 const std::function<void()> &drop)
{
	InstrumentTemplate *it = m_ui->shell();
	const QString id = QStringLiteral("ref%1").arg(m_refCount);

	scopy::adc::AcqChannel::Args args;
	args.store = m_ui->store();
	args.engine = m_ui->engine();
	args.key = key;
	args.xKey = xKey;
	args.info.kind = scopy::acq::ReprKind::Curve;
	args.info.colorIndex = CHANNELS + m_refCount++;
	args.uid = m_uid++;
	auto *curve = new scopy::adc::AcqCurveChannel(args);
	curve->setName(name);
	attachChannel(curve, m_specPlot);
	if(xKey.isEmpty()) {
		curve->xAxis()->setSource(scopy::adc::AcqAxis::Source::time());
	}
	curve->xAxis()->setSourceFixed(true);
	curve->yAxis()->setSourceFixed(true);
	curve->xAxis()->setUnit(QStringLiteral("Hz"));
	curve->yAxis()->setUnit(unitSymbol());
	curve->pull(size);
	m_refs.append({nullptr, id, curve, key, xKey, size, drop});
	applyXRange();

	MenuSectionCollapseWidget *section = it->createMenuSection(name, SO_VIEW);
	auto *style = new gui::MenuPlotChannelCurveStyleControl(section);
	style->addChannels(curve->plotChannel());
	QPushButton *exportRef = button(tr("Export reference"), section);
	QPushButton *remove = button(tr("Remove reference"), section);
	section->contentLayout()->addWidget(style);
	section->contentLayout()->addWidget(exportRef);
	exportRef->setVisible(!xKey.isEmpty());
	section->contentLayout()->addWidget(remove);
	MenuControlButton *row = it->addChannelRow(it->addChannelGroup(kReferencesGroup), name, curve->color(), id);
	m_refs.last().row = row;
	it->addMenuPage(id, page(section));
	row->checkBox()->setChecked(true);
	connect(row->checkBox(), &QAbstractButton::toggled, this, [this, curve](bool on) {
		curve->setEnabled(on);
		m_specPlot->replot();
	});
	connect(exportRef, &QPushButton::clicked, this,
		[this, name, key, xKey]() { saveCsv({{name, key}}, xKey); });
	connect(remove, &QPushButton::clicked, this, [this, id]() { removeReference(id); });
	applyYRange();
}

void M2kSpectrumController::removeReference(const QString &menuId)
{
	for(int i = 0; i < m_refs.size(); ++i) {
		if(m_refs[i].menuId != menuId) {
			continue;
		}
		Reference ref = m_refs.takeAt(i);
		m_specPlot->removeChannelRef(ref.curve);
		delete ref.curve;
		ref.drop();
		QTimer::singleShot(0, this, [this, ref]() {
			m_ui->shell()->removeChannelRow(m_ui->shell()->addChannelGroup(kReferencesGroup), ref.row,
							ref.menuId, true);
		});
		m_specPlot->replot();
		return;
	}
}

void M2kSpectrumController::snapshot(int ch)
{
	scopy::acq::DataStore *store = m_ui->store();
	if(!store->latestFloat(spectrumKey(ch)) || !store->latestFloat(freqKey(ch))) {
		StatusBarManager::pushMessage(
			tr("Spectrum Analyzer: run first, %1 has no spectrum yet").arg(channelName(ch)));
		return;
	}
	if(!m_snap) {
		m_snap = new scopy::acq::SnapshotSource(QStringLiteral("ref"), m_ui->engine());
	}
	if(m_snap->allSlots().isEmpty()) {
		m_ui->engine()->addSource(m_snap);
	}
	const QString title = QStringLiteral("ref%1").arg(m_refCount);
	const QStringList titles{title, title + QStringLiteral("-hz")};
	const DataKey sources[] = {spectrumKey(ch), freqKey(ch)};
	for(int k = 0; k < 2; ++k) {
		const int slot = m_snap->addSlot();
		m_snap->setSlotTitle(slot, titles[k]);
		m_snap->setSlotSource(slot, sources[k]);
		m_snap->setSlotCyclic(slot, false);
		m_snap->capture(slot);
		store->copy(scopy::acq::SnapshotSource::keyFor(m_snap->name(), titles[k]), m_snap->slotAt(slot)->data);
	}
	auto drop = [this, titles]() {
		for(const QString &t : titles) {
			const QList<scopy::acq::SnapshotSource::Slot> all = m_snap->allSlots();
			for(int i = 0; i < all.size(); ++i) {
				if(all[i].title == t) {
					m_snap->removeSlot(i);
					break;
				}
			}
		}
		if(m_snap->allSlots().isEmpty()) {
			m_ui->engine()->removeSource(m_snap);
		}
	};
	addReference(tr("%1 REF %2").arg(channelName(ch)).arg(m_refCount + 1),
		     scopy::acq::SnapshotSource::keyFor(m_snap->name(), titles[0]),
		     scopy::acq::SnapshotSource::keyFor(m_snap->name(), titles[1]), bins(), drop);
}

void M2kSpectrumController::syncImportedReferences()
{
	QList<QPair<DataKey, int>> wanted;
	const QList<QString> enabled = m_file->enabledChannels();
	for(const scopy::adc::FileSourceBlock::SlotInfo &slot : m_file->slotInfos()) {
		for(const QString &c : enabled) {
			if(c.startsWith(slot.title + '-')) {
				wanted.append({DataKey::raw(m_file->id(), c), slot.sampleCount});
			}
		}
	}
	auto isWanted = [&wanted](const DataKey &k) {
		return std::any_of(wanted.begin(), wanted.end(), [&k](const auto &w) { return w.first == k; });
	};
	for(const Reference &r : QList<Reference>(m_refs)) {
		if(r.key.sourceId() == m_file->id() && !isWanted(r.key)) {
			removeReference(r.menuId);
		}
	}
	for(const auto &[key, size] : std::as_const(wanted)) {
		auto ref =
			std::find_if(m_refs.begin(), m_refs.end(), [&key](const Reference &r) { return r.key == key; });
		if(ref == m_refs.end()) {
			addReference(key.channelId(), key, DataKey(), size,
				     [this, key]() { m_file->enableChannel(key.channelId(), false); });
		} else {
			ref->size = size;
			ref->curve->pull(size);
		}
	}
	m_specPlot->replot();
}

void M2kSpectrumController::exportCsv()
{
	QList<QPair<QString, DataKey>> columns;
	for(int ch = 0; ch < CHANNELS; ++ch) {
		if(m_chEnabled[ch]) {
			columns.append({channelName(ch), spectrumKey(ch)});
		}
	}
	saveCsv(columns, freqKey(m_chEnabled[0] ? 0 : 1));
}

void M2kSpectrumController::saveCsv(const QList<QPair<QString, DataKey>> &columns, const DataKey &xKey)
{
	scopy::acq::DataStore *store = m_ui->store();
	const QVector<float> hz = store->latestFloat(xKey).value_or(QVector<float>{});
	QList<QVector<float>> mags;
	for(const auto &c : columns) {
		if(const std::optional<QVector<float>> m = store->latestFloat(c.second)) {
			mags.append(*m);
		}
	}
	if(hz.isEmpty() || mags.isEmpty() || mags.size() != columns.size()) {
		StatusBarManager::pushMessage(tr("Spectrum Analyzer: nothing to export yet"));
		return;
	}

	QString csv;
	QTextStream out(&csv);
	out << "Frequency (Hz)";
	for(const auto &c : columns) {
		out << "," << c.first << " (" << unitSymbol() << ")";
	}
	out << "\n";
	for(int i = 0; i < hz.size(); ++i) {
		out << QString::number(hz[i], 'g', 10);
		for(const QVector<float> &m : std::as_const(mags)) {
			out << "," << (i < m.size() ? QString::number(m[i], 'g', 8) : QString());
		}
		out << "\n";
	}
	out.flush();
	FileManagerHelper::saveDataToFile(m_ui, csv, tr("Export spectrum"));
}

bool M2kSpectrumController::claim()
{
	if(m_calibrating) {
		StatusBarManager::pushMessage(tr("Spectrum Analyzer: wait for calibration to finish"));
		return false;
	}
	if(!m_claimed) {
		ResourceManager::open(m_resourceKey, this, true);
		m_claimed = true;
	}
	if(auto *ping = m_ctx ? m_ctx->findChild<component::Ping *>(QString(), Qt::FindDirectChildrenOnly) : nullptr) {
		if(ping->isMonitoring()) {
			ping->stopMonitoring();
			m_pingPaused = true;
		}
	}
	return true;
}

void M2kSpectrumController::release()
{
	if(m_claimed) {
		m_claimed = false;
		ResourceManager::close(m_resourceKey);
	}
	if(m_pingPaused && m_ctx) {
		m_pingPaused = false;
		if(auto *ping = m_ctx->findChild<component::Ping *>(QString(), Qt::FindDirectChildrenOnly)) {
			ping->startMonitoring();
		}
	}
}

void M2kSpectrumController::stop()
{
	m_claimed = false;
	AcqInstrumentController::stop();
}

#include "moc_m2kspectrumcontroller.cpp"
