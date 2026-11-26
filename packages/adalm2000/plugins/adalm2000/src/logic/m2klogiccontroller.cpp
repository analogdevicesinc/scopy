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

#include "m2klogiccontroller.h"

#include "horizontalsettings.h"
#include "m2kattributeio.h"
#include "m2kcontext.h"
#include "m2ksource.h"
#include "m2ktriggermath.h"
#include "uihelpers.h"

#include <core/acq_engine/acquisitionengine.h>
#include <core/acq_engine/datastore.h>
#include <core/acq_ui/acqannotationchannel.h>
#include <core/acq_ui/acqaxis.h>
#include <core/acq_ui/acqdigitalchannel.h>
#include <core/acq_ui/acqinstrument.h>
#include <core/acq_ui/acqplot.h>
#include <core/acq_engine/externaldecoderprocessor.h>
#include <core/decoder/decodermanager.h>
#include <core/decoder/decoderpanel.h>

#include <gui/axishandle.h>
#include <gui/instrumenttemplate.h>
#include <gui/plot_utils.hpp>
#include <gui/plotaxis.h>
#include <gui/plotaxishandle.h>
#include <gui/plotnavigator.hpp>
#include <gui/plotwidget.h>
#include <gui/printplotmanager.h>
#include <gui/style.h>
#include <gui/widgets/cursorsettings.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menucombo.h>
#include <gui/widgets/menucontrolbutton.h>
#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menuspinbox.h>
#include <gui/widgets/plotinfo.h>
#include <gui/widgets/toolbuttons.h>
#include <pluginbase/preferences.h>
#include <pluginbase/statusbarmanager.h>

#include <component/channel.h>
#include <component/device.h>

#include <QApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QFontMetrics>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QRegularExpression>
#include <QScrollBar>
#include <QTextStream>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>

#include <qwt_plot.h>
#include <qwt_scale_map.h>
#include <qwt_scale_widget.h>

#include <algorithm>
#include <cmath>

using namespace scopy;
using namespace scopy::adalm2000;
using namespace scopy::adalm2000::ui;
using scopy::acq::DataKey;

namespace {
const QString kGeneralGroup = QStringLiteral("General");
const QString kChannelsGroup = QStringLiteral("Channels");
const QString kDecodersGroup = QStringLiteral("Decoders");

constexpr int kFrameIntervalMs = 16;
constexpr int kMinSamples = 100;
constexpr int kMaxBuffer = 1 << 20;
constexpr int kMaxWindow = 1 << 22;
constexpr int kDelayMin = trigger::DELAY_MIN;
constexpr int kDelayMax = -trigger::DELAY_MIN - 1;
constexpr int kTimeBaseSamples = 1 << 16;
constexpr int kTrigDebounceMs = 300;
constexpr int kTrigWaitMs = 250;
constexpr double kMaxContinuousRate = 5e6;
constexpr double kMaxSampleRate = 100e6;
constexpr unsigned kStreamingKernelBuffers = 4;

constexpr int kTopPx = 8;
constexpr int kInfoSpacingPx = 6;
constexpr int kHandleClearPx = 22;
constexpr int kTrackPitchPx = 26;
constexpr int kDecodeRowPx = 24;
constexpr int kDecodeGapPx = 6;

const QList<QPair<QString, int>> kConditions = {
	{QStringLiteral("-"), trigger::DIGITAL_CONDITION_NONE},
	{QStringLiteral("Rising edge"), 0},
	{QStringLiteral("Falling edge"), 1},
	{QStringLiteral("Low"), 2},
	{QStringLiteral("High"), 3},
	{QStringLiteral("Any edge"), 4},
};

constexpr int kSourceTriggerIn = 0;
constexpr int kSourceAnalogIn = 1;
constexpr int kSourceNone = 2;

// One hertz over, as the old instrument did, so 100 MHz / n does not round to n + 1.
QString rateValue(double hz) { return QString::number(hz + 1, 'g', 12); }

MenuCombo *conditionCombo(const QString &label, QWidget *parent)
{
	auto *combo = new MenuCombo(label, parent);
	for(const auto &c : kConditions) {
		combo->combo()->addItem(c.first, c.second);
	}
	return combo;
}

} // namespace

M2kLogicController::M2kLogicController(ToolMenuEntry *tme, M2kContext *ctx, const QString &uri, QObject *parent)
	: M2kInstrumentController(tme, ctx, QStringLiteral("adalm2000-digital-in:") + uri, tr("Logic Analyzer"), parent)
{
	m_limits.rates = sweep::series125(1.0, kMaxSampleRate);
	m_limits.minSamples = kMinSamples;
	m_limits.maxBuffer = kMaxBuffer;
	m_limits.maxWindow = kMaxWindow;
	m_limits.timeBaseSamples = kTimeBaseSamples;
	m_limits.delayMin = kDelayMin;
	m_limits.delayMax = kDelayMax;
	m_limits.triggerInBuffer = true;
}

M2kLogicController::~M2kLogicController()
{
	AcqInstrumentController::stop();
	for(scopy::adc::AcqDigitalChannel *c : m_dio) {
		delete c;
	}
	qDeleteAll(m_decodes);
}

void M2kLogicController::setupBlocks(scopy::component::Context *ctx)
{
	Q_UNUSED(ctx)
	InstrumentTemplate *it = m_ui->shell();
	it->addChannelGroup(kGeneralGroup);
	it->addChannelGroup(kChannelsGroup);
	it->addChannelGroup(kDecodersGroup);

	m_source = new M2kSource(m_ctx, QStringLiteral("logic"), m_ui->engine());
	m_ui->engine()->addSource(m_source);
	m_ui->setBufferSize(m_sweep.bufferSize);

	component::Device *dev = digital();
	m_source->set(dev, "sample_rate_in", rateValue(m_sweep.sampleRate), M2kSource::Reopen);
	m_source->set(dev, "trigger_streaming_flag", QStringLiteral("0"), M2kSource::Reopen);
	m_source->set(dev, "trigger_delay", QStringLiteral("0"));
	m_source->set(dev, "trigger_mode", trigger::toName(trigger::DIGITAL_MODE_OPTIONS, 0));
	for(int pin = 0; pin < CHANNELS; ++pin) {
		m_source->enableChannel(M2kSource::channelId(M2kSource::Digital, pin), true);
		m_source->set(m_source->channel(M2kSource::Digital, pin), "trigger_condition",
			      trigger::toName(trigger::DIGITAL_CONDITION_OPTIONS, trigger::DIGITAL_CONDITION_NONE));
	}
	if(hasExternalTrigger()) {
		m_source->set(dev, "trigger_source", trigger::toName(trigger::DIGITAL_SOURCE_OPTIONS, kSourceNone));
		m_source->set(dev, "trigger_external_condition",
			      trigger::toName(trigger::DIGITAL_CONDITION_OPTIONS, trigger::DIGITAL_CONDITION_NONE));
	}
}

component::Device *M2kLogicController::digital() const { return m_source->device(M2kSource::Digital); }

bool M2kLogicController::hasExternalTrigger() const { return attr::find(digital(), "trigger_source") != nullptr; }

void M2kLogicController::setupPlots()
{
	setupCenter();
	setupChannels();
	setupTopRail();

	InstrumentTemplate *it = m_ui->shell();
	MenuSectionCollapseWidget *group = it->addChannelGroup(kGeneralGroup);
	MenuControlButton *general =
		it->addChannelRow(group, tr("General settings"), QColor(), QStringLiteral("general"));
	general->checkBox()->setVisible(false);
	it->addMenuPage(QStringLiteral("general"), buildGeneralPage());
	MenuControlButton *trig = it->addChannelRow(group, tr("Trigger"), QColor(), QStringLiteral("trigger"));
	m_trigEnable = trig->checkBox();
	m_trigEnable->setChecked(true);
	m_trigStatus = addRowStatus(trig);
	m_trigWaitTimer = new QTimer(this);
	m_trigWaitTimer->setSingleShot(true);
	connect(m_trigWaitTimer, &QTimer::timeout, this, [this]() { setTriggerStatus(tr("Waiting")); });
	setTriggerStatus(tr("Stop"));
	it->addMenuPage(QStringLiteral("trigger"), buildTriggerPage());
	addHoverButton(tr("Cursors"), buildCursorsPage());

	connect(m_ui, &scopy::adc::AcqInstrument::started, this, &M2kLogicController::onStarted);
	connect(m_ui, &scopy::adc::AcqInstrument::cycleComplete, this, &M2kLogicController::onCycleComplete);
	connect(m_ui, &scopy::adc::AcqInstrument::stopped, this, &M2kLogicController::onStopped);
	connect(m_source, &M2kSource::sampleRateChanged, this, [this](int input, double hz) {
		if(input != M2kSource::Digital) {
			return;
		}
		m_sweep.sampleRate = hz;
		m_horizontal->setSweep(m_sweep);
		applyXTransform();
		syncDecoderRates();
	});

	setupCalibrationGate();
	applySweep();
	applyTrackScroll();
	general->click();
}

void M2kLogicController::setupDecoders()
{
	createDecoders();
	InstrumentTemplate *it = m_ui->shell();
	MenuControlButton *row = it->addChannelRow(it->addChannelGroup(kDecodersGroup), tr("Add decoder"), QColor(),
						   QStringLiteral("decoders"));
	row->checkBox()->setVisible(false);
	MenuSectionCollapseWidget *section = it->createMenuSection(tr("DECODERS"), SO_CH);
	section->add(decoders());
	it->addMenuPage(QStringLiteral("decoders"), withStretch({section}));

	scopy::decoder::DecoderManager *mgr = decoders()->manager();
	mgr->setDecoderWindowSize(m_sweep.plotWindow);
	connect(mgr, &scopy::decoder::DecoderManager::outKeyAdded, this, &M2kLogicController::addDecode);
	connect(mgr, &scopy::decoder::DecoderManager::stagesRemoved, this,
		[this](const QString &, const QList<DataKey> &keys) { removeDecodes(keys); });
	// Queued: the panel builds the editor after decoderAdded, and removal starts in the editor.
	connect(mgr, &scopy::decoder::DecoderManager::decoderAdded, this, &M2kLogicController::addDecoderRow,
		Qt::QueuedConnection);
	connect(mgr, &scopy::decoder::DecoderManager::decoderRemoved, this, &M2kLogicController::removeDecoderRow,
		Qt::QueuedConnection);

	QList<DataKey> keys;
	for(int pin = 0; pin < CHANNELS; ++pin) {
		keys.append(m_source->rawKey(M2kSource::Digital, pin));
	}
	decoders()->refreshKeys(keys);
}

void M2kLogicController::setupCenter()
{
	InstrumentTemplate *it = m_ui->shell();
	auto *center = new QWidget(m_ui);
	auto *lay = new QHBoxLayout(center);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(0);

	m_plot = new scopy::adc::AcqPlot(tr("Logic Analyzer"), scopy::adc::AcqPlotKind::Basic, 0, center);
	PlotWidget *pw = m_plot->plot();
	lay->addWidget(pw);
	m_scroll = new QScrollBar(Qt::Vertical, center);
	m_scroll->setVisible(false);
	lay->addWidget(m_scroll);
	it->setCenterWidget(center);

	pw->navigator()->setYAxesEn(false);
	m_plotStatus = pw->getPlotInfo()->addLabelInfo(IP_LEFT, IP_TOP);
	m_sweepInfo = pw->getPlotInfo()->addLabelInfo(IP_LEFT, IP_TOP);
	m_continuityWarning = pw->getPlotInfo()->addLabelInfo(IP_LEFT, IP_BOTTOM);
	m_continuityWarning->setText(tr("Data might not be continuous, lower the sample rate or the plot window"));
	Style::setStyle(m_continuityWarning, style::properties::label::warning);
	for(QLabel *l : {m_plotStatus.data(), m_sweepInfo.data(), m_continuityWarning.data()}) {
		l->setIndent(kHandleClearPx);
	}
	connect(pw->navigator(), &PlotNavigator::rectChanged, this, &M2kLogicController::updateSweepInfo);
	pw->xAxis()->setVisible(false);
	pw->yAxis()->setVisible(false);
	pw->plot()->canvas()->installEventFilter(this);
	connect(m_scroll, &QScrollBar::valueChanged, this, &M2kLogicController::applyTrackScroll);

	// Tracks are placed once, below the two info lines; after that they stay where dragged.
	const int tracksTop = kTopPx + 2 * (QFontMetrics(QApplication::font()).height() + kInfoSpacingPx);
	for(int pin = 0; pin < CHANNELS; ++pin) {
		scopy::adc::AcqChannel::Args args;
		args.store = m_ui->store();
		args.engine = m_ui->engine();
		args.key = m_source->rawKey(M2kSource::Digital, pin);
		args.info = m_ui->engine()->streamInfo(args.key).value_or(scopy::acq::StreamInfo{});
		args.uid = m_uid++;
		m_dio[pin] = new scopy::adc::AcqDigitalChannel(args);
		m_dio[pin]->setName(args.info.label);
		m_plot->addChannelRef(m_dio[pin]);
		m_dio[pin]->attach(m_plot);
		m_dio[pin]->setKeyPresent(true);
		m_dio[pin]->setTriggerHeadroom(false);
		m_dio[pin]->xAxis()->setUnit(QStringLiteral("s"));
		m_dio[pin]->handle()->setPositionSilent(-(tracksTop + pin * kTrackPitchPx));
	}

	PlotAxis *x = m_dio[0]->xAxis()->plotAxis();
	x->getFormatter()->setTrimZeroes(true);
	x->setVisible(true);
	// Fixed room for the end labels, so a delay change does not resize the canvas.
	QwtScaleWidget *scale = pw->plot()->axisWidget(x->axisId());
	const int labelPx = QFontMetrics(scale->font()).horizontalAdvance(QStringLiteral("-888.888 ms"));
	scale->setMinBorderDist(labelPx / 2, labelPx / 2);

	m_trigHandle = new PlotAxisHandle(pw, x);
	m_trigHandle->handle()->setColor(Style::getColor(json::theme::interactive_primary_idle));
	pw->addPlotAxisHandle(m_trigHandle);
	m_delayTimer = new QTimer(this);
	m_delayTimer->setSingleShot(true);
	m_delayTimer->setInterval(kTrigDebounceMs);
	// Like the software trigger's scan mode: pan the axis under the handle, keeping the width.
	connect(m_trigHandle, &PlotAxisHandle::scalePosChanged, this, [this, x](double pos) {
		const QwtScaleMap map = m_plot->plot()->plot()->canvasMap(x->axisId());
		const double width = map.s2() - map.s1();
		const double maxFrac =
			std::min(1.0, -static_cast<double>(sweep::delayMin(m_sweep, m_limits)) / m_sweep.plotWindow);
		m_trigFrac = std::clamp((pos - map.s1()) / width, 0.0, maxFrac);
		x->setInterval(-m_trigFrac * width, (1.0 - m_trigFrac) * width);
		m_trigHandle->setPositionSilent(0.0);
		QMetaObject::invokeMethod(
			m_trigHandle, [this]() { m_trigHandle->setPositionSilent(0.0); }, Qt::QueuedConnection);
		m_plot->replot();
		m_delayTimer->start();
	});
	connect(m_delayTimer, &QTimer::timeout, this, [this]() { setDelay(qRound(-m_trigFrac * m_sweep.plotWindow)); });

	m_frameTimer = new QTimer(this);
	m_frameTimer->setInterval(kFrameIntervalMs);
	connect(m_frameTimer, &QTimer::timeout, this, [this]() {
		if(m_dirty) {
			m_dirty = false;
			m_plot->replot();
		}
	});
}

void M2kLogicController::setupChannels()
{
	InstrumentTemplate *it = m_ui->shell();
	MenuSectionCollapseWidget *group = it->addChannelGroup(kChannelsGroup);
	// The all-channels switch sits in the group header, left of the collapse arrow.
	auto *header = qobject_cast<QBoxLayout *>(group->collapseSection()->header()->layout());
	auto *all = new SmallOnOffSwitch(group);
	all->setToolTip(tr("All channels"));
	header->insertWidget(header->count() - 1, all);
	auto syncAll = [this, all]() {
		QSignalBlocker block(all);
		all->setChecked(std::all_of(std::begin(m_dio), std::end(m_dio),
					    [](scopy::adc::AcqDigitalChannel *c) { return c->isEnabled(); }));
	};
	for(int pin = 0; pin < CHANNELS; ++pin) {
		MenuControlButton *row = it->addChannelRow(group, m_dio[pin]->name(), m_dio[pin]->color(), QString(), 0,
							   InstrumentTemplate::CRS_SMALL);
		row->checkBox()->setChecked(true);
		connect(row->checkBox(), &QAbstractButton::toggled, this, [this, pin, syncAll](bool on) {
			m_dio[pin]->setEnabled(on);
			m_source->enableChannel(M2kSource::channelId(M2kSource::Digital, pin), on);
			applyTrackScroll();
			syncAll();
			if(std::none_of(std::begin(m_dio), std::end(m_dio),
					[](scopy::adc::AcqDigitalChannel *c) { return c->isEnabled(); })) {
				AcqInstrumentController::stop();
			}
		});
		connect(m_dio[pin], &scopy::adc::AcqChannel::nameChanged, row, &MenuControlButton::setName);
		m_chRow[pin] = row;
	}
	connect(all, &QAbstractButton::toggled, this, [this](bool on) {
		for(MenuControlButton *row : m_chRow) {
			row->checkBox()->setChecked(on);
		}
	});
	syncAll();
}

void M2kLogicController::setupTopRail()
{
	InstrumentTemplate *it = m_ui->shell();
	connect(it->infoBtn(), &QAbstractButton::clicked, this, []() {
		QDesktopServices::openUrl(QUrl(
			QStringLiteral("https://analogdevicesinc.github.io/scopy/plugins/m2k/logic_analyzer.html")));
	});
	auto *printManager = new PrintPlotManager(this);
	connect(it->printBtn(), &QAbstractButton::clicked, this, [this, printManager]() {
		printManager->printPlots({m_plot->plot()}, QStringLiteral("Logic Analyzer"));
	});
	connect(it->exportBtn(), &QAbstractButton::clicked, this, &M2kLogicController::exportFile);
}

QWidget *M2kLogicController::buildGeneralPage()
{
	MenuSectionCollapseWidget *section = m_ui->shell()->createMenuSection(tr("SWEEP"), SO_DEV);
	const int divs = qRound(m_dio[0]->xAxis()->plotAxis()->divs());
	m_horizontal = new HorizontalSettings(m_limits, divs, HorizontalSettings::Samples, section);
	m_horizontal->setSweep(m_sweep);
	m_buffersSwitch = new MenuOnOffSwitch(tr("Show buffers"), section, false);
	auto *hint = new QLabel(tr("A plot window wider than the buffer streams: it triggers once, then joins "
				   "consecutive buffers. The delay applies to triggered buffers only."),
				section);
	hint->setWordWrap(true);
	Style::setStyle(hint, style::properties::label::subtle);

	section->add(m_horizontal);
	section->add(m_buffersSwitch);
	section->add(hint);

	connect(m_horizontal, &HorizontalSettings::sweepRequested, this, &M2kLogicController::setSweep);
	connect(m_horizontal, &HorizontalSettings::timeBaseSamplesChanged, this,
		[this](int n) { m_limits.timeBaseSamples = n; });
	connect(m_buffersSwitch->onOffswitch(), &QAbstractButton::toggled, m_plot,
		&scopy::adc::AcqPlot::setShowBufferStarts);
	return withStretch({section});
}

QWidget *M2kLogicController::buildTriggerPage()
{
	InstrumentTemplate *it = m_ui->shell();
	auto *w = new QWidget();
	auto *lay = new QVBoxLayout(w);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(10);

	MenuSectionCollapseWidget *logic = it->createMenuSection(tr("TRIGGER"), SO_DEV, w);
	m_modeCombo = new MenuCombo(tr("Trigger logic"), logic);
	m_modeCombo->combo()->addItems({QStringLiteral("OR"), QStringLiteral("AND")});
	logic->add(m_modeCombo);
	auto *grid = new QWidget(logic);
	auto *gridLay = new QGridLayout(grid);
	gridLay->setContentsMargins(0, 0, 0, 0);
	const int rows = (CHANNELS + 1) / 2;
	for(int pin = 0; pin < CHANNELS; ++pin) {
		m_trigger[pin] = conditionCombo(m_dio[pin]->name(), grid);
		connect(m_dio[pin], &scopy::adc::AcqChannel::nameChanged, m_trigger[pin]->label(), &QLabel::setText);
		connect(m_trigger[pin]->combo(), &QComboBox::currentIndexChanged, this,
			[this, pin](int i) { setTriggerCondition(pin, i); });
		gridLay->addWidget(m_trigger[pin], pin % rows, pin / rows);
	}
	logic->add(grid);
	lay->addWidget(logic);
	connect(m_modeCombo->combo(), &QComboBox::currentIndexChanged, this, [this](int mode) {
		m_source->set(digital(), "trigger_mode", trigger::toName(trigger::DIGITAL_MODE_OPTIONS, mode),
			      M2kSource::Restart);
	});

	MenuSectionCollapseWidget *ext = it->createMenuSection(tr("EXTERNAL TRIGGER"), SO_DEV, w);
	m_extSwitch = new MenuOnOffSwitch(tr("Enable"), ext, false);
	m_extSourceCombo = new MenuCombo(tr("Source"), ext);
	m_extSourceCombo->combo()->addItem(tr("External Trigger In"), kSourceTriggerIn);
	m_extSourceCombo->combo()->addItem(tr("Oscilloscope"), kSourceAnalogIn);
	m_extConditionCombo = conditionCombo(tr("Condition"), ext);
	ext->add(m_extSwitch);
	ext->add(m_extSourceCombo);
	ext->add(m_extConditionCombo);
	if(!hasExternalTrigger()) {
		auto *warning = new QLabel(tr("The firmware does not support external triggering."), ext);
		warning->setWordWrap(true);
		Style::setStyle(warning, style::properties::label::subtle);
		ext->add(warning);
		m_extSwitch->setEnabled(false);
	}
	lay->addWidget(ext);
	lay->addStretch();

	auto applyExternal = [this]() {
		const bool on = m_extSwitch->onOffswitch()->isChecked();
		const int source = m_extSourceCombo->combo()->currentData().toInt();
		const bool fromScope = source == kSourceAnalogIn;
		m_extSourceCombo->setEnabled(on);
		m_extConditionCombo->setEnabled(on && !fromScope);
		const int condition = fromScope ? trigger::DIGITAL_CONDITION_NONE
						: m_extConditionCombo->combo()->currentData().toInt();
		m_source->set(digital(), "trigger_source",
			      trigger::toName(trigger::DIGITAL_SOURCE_OPTIONS,
					      on && m_trigEnable->isChecked() ? source : kSourceNone),
			      M2kSource::Restart);
		m_source->set(digital(), "trigger_external_condition",
			      trigger::toName(trigger::DIGITAL_CONDITION_OPTIONS, condition), M2kSource::Restart);
	};
	connect(m_extSwitch->onOffswitch(), &QAbstractButton::toggled, this, applyExternal);
	connect(m_trigEnable, &QAbstractButton::toggled, this, [this, applyExternal]() {
		for(int pin = 0; pin < CHANNELS; ++pin) {
			setTriggerCondition(pin, triggerCondition(pin));
		}
		applyExternal();
		applyXTransform();
	});
	connect(m_extSourceCombo->combo(), &QComboBox::currentIndexChanged, this, applyExternal);
	connect(m_extConditionCombo->combo(), &QComboBox::currentIndexChanged, this, applyExternal);
	m_extSourceCombo->setEnabled(false);
	m_extConditionCombo->setEnabled(false);
	return w;
}

QWidget *M2kLogicController::buildCursorsPage()
{
	MenuSectionCollapseWidget *section = m_ui->shell()->createMenuSection(tr("CURSORS"), SO_VIEW);
	CursorSettings *settings = m_plot->cursorSettings(section);
	settings->setXOnly(true);
	settings->getXEn()->setChecked(false);
	m_plot->setShowCursors(true);
	section->add(settings);
	return withStretch({section});
}

void M2kLogicController::onStarted()
{
	for(scopy::adc::AcqDigitalChannel *c : m_dio) {
		c->refreshSampleRate();
		c->reset();
	}
	for(scopy::adc::AcqAnnotationChannel *d : std::as_const(m_decodes)) {
		d->reset();
	}
	m_frameTimer->start();
	setTriggerStatus(tr("Waiting"));
}

void M2kLogicController::onCycleComplete()
{
	if(m_ui->engine()->isRunning()) {
		setTriggerStatus(tr("Triggered"));
		const double bufferMs = 1000.0 * m_sweep.bufferSize / m_sweep.sampleRate;
		m_trigWaitTimer->start(kTrigWaitMs + static_cast<int>(bufferMs));
		if(m_source->bufferDelay(M2kSource::Digital) != m_shownDelay) {
			m_shownDelay = m_source->bufferDelay(M2kSource::Digital);
			applyXTransform();
		}
	}
	for(scopy::adc::AcqDigitalChannel *c : m_dio) {
		c->pull(m_sweep.plotWindow);
	}
	QList<int> rows;
	for(scopy::adc::AcqAnnotationChannel *d : std::as_const(m_decodes)) {
		d->pull(m_sweep.plotWindow);
		rows.append(d->rowCount());
	}
	if(rows != m_decodeRowCounts) {
		m_decodeRowCounts = rows;
		applyTrackScroll();
	}
	m_dirty = true;
}

void M2kLogicController::onStopped()
{
	m_frameTimer->stop();
	// cycleComplete is rate-limited, so the last cycle may not have been drawn.
	onCycleComplete();
	for(scopy::adc::AcqDigitalChannel *c : m_dio) {
		c->onStopped();
	}
	m_trigWaitTimer->stop();
	setTriggerStatus(tr("Stop"));
	m_plot->replot();
}

void M2kLogicController::single()
{
	if(m_ui->engine()->isRunning()) {
		return;
	}
	m_ui->single(static_cast<unsigned int>((m_sweep.plotWindow + m_sweep.bufferSize - 1) / m_sweep.bufferSize));
}

void M2kLogicController::setSweep(sweep::Sweep s)
{
	s = sweep::normalized(s, m_limits);
	if(s.sampleRate == m_sweep.sampleRate && s.bufferSize == m_sweep.bufferSize &&
	   s.plotWindow == m_sweep.plotWindow) {
		setDelay(s.delay);
		return;
	}
	restartIfRunning([this, s]() {
		m_sweep = s;
		m_source->set(digital(), "sample_rate_in", rateValue(s.sampleRate), M2kSource::Reopen);
		m_ui->setBufferSize(s.bufferSize);
		applySweep();
		syncDecoderRates();
	});
}

void M2kLogicController::setDelay(int samples)
{
	m_sweep.delay = sweep::align4(samples, sweep::delayMin(m_sweep, m_limits), kDelayMax);
	m_horizontal->setSweep(m_sweep);
	m_source->set(digital(), "trigger_delay", QString::number(isStreaming() ? 0 : m_sweep.delay),
		      M2kSource::Restart);
}

double M2kLogicController::timeOffset() const { return isStreaming() ? 0.0 : m_shownDelay / m_sweep.sampleRate; }

void M2kLogicController::applySweep()
{
	setDelay(m_sweep.delay);
	m_shownDelay = m_sweep.delay;
	const bool streaming = isStreaming();
	m_source->set(digital(), "trigger_streaming_flag", streaming ? QStringLiteral("1") : QStringLiteral("0"),
		      M2kSource::Reopen);
	m_source->setKernelBuffers(M2kSource::Digital, streaming ? kStreamingKernelBuffers : 1);
	m_horizontal->setDelayEnabled(!streaming);
	for(scopy::adc::AcqDigitalChannel *c : m_dio) {
		c->reclaimDepth(m_sweep.plotWindow);
	}
	if(decoders()) {
		decoders()->manager()->setDecoderWindowSize(m_sweep.plotWindow);
	}
	applyXTransform();
}

void M2kLogicController::applyXTransform()
{
	const double sr = m_sweep.sampleRate;
	const bool streaming = isStreaming();
	const double offset = timeOffset();
	for(scopy::adc::AcqDigitalChannel *c : m_dio) {
		c->setXTransform(offset, 1.0 / sr);
	}
	for(scopy::adc::AcqAnnotationChannel *d : std::as_const(m_decodes)) {
		d->setXTransform(offset, 1.0 / sr);
	}
	m_trigHandle->handle()->setVisible(!streaming && m_trigEnable->isChecked());
	m_trigHandle->setPositionSilent(std::clamp(0.0, offset, offset + m_sweep.plotWindow / sr));
	m_continuityWarning->setVisible(streaming && sr > kMaxContinuousRate);
	updateSweepInfo();
	if(!m_ui->engine()->isRunning()) {
		onCycleComplete();
	}
	m_plot->replot();
}

void M2kLogicController::setTriggerStatus(const QString &status)
{
	m_trigStatus->setText(status);
	m_plotStatus->setText(status);
}

void M2kLogicController::updateSweepInfo()
{
	MetricPrefixFormatter fmt;
	fmt.setTrimZeroes(true);
	PlotAxis *x = m_dio[0]->xAxis()->plotAxis();
	const double perDiv = std::abs(x->visibleMax() - x->visibleMin()) / x->divs();
	m_sweepInfo->setText(tr("%1 samples at %2, %3/div")
				     .arg(m_sweep.plotWindow)
				     .arg(fmt.format(m_sweep.sampleRate, QStringLiteral("sps"), 2))
				     .arg(fmt.format(perDiv, QStringLiteral("s"), 2)));
}

void M2kLogicController::addDecode(const DataKey &key)
{
	scopy::adc::AcqChannel::Args args;
	args.store = m_ui->store();
	args.engine = m_ui->engine();
	args.key = key;
	args.info = m_ui->engine()->streamInfo(key).value_or(scopy::acq::StreamInfo{});
	args.info.kind = scopy::acq::ReprKind::Annotations;
	args.uid = m_uid++;
	QFont title = QApplication::font();
	title.setBold(true);
	const int top = contentBottomPx() + QFontMetrics(title).height() + 4;
	auto *ch = new scopy::adc::AcqAnnotationChannel(args);
	ch->setName(args.info.label.isEmpty() ? key.channelId() : args.info.label);
	m_plot->addChannelRef(ch);
	ch->attach(m_plot);
	ch->reclaimDepth(m_sweep.plotWindow);
	ch->setKeyPresent(true);
	ch->xAxis()->setUnit(QStringLiteral("s"));
	ch->handle()->setPositionSilent(-top);
	for(const scopy::decoder::DecoderInstance &d : decoders()->manager()->decoders()) {
		if(d.outKeys.contains(key) && m_decoderRows.contains(d.uid)) {
			ch->setEnabled(m_decoderRows[d.uid]->checkBox()->isChecked());
		}
	}
	m_decodes.append(ch);
	applyXTransform();
	applyTrackScroll();
}

void M2kLogicController::addDecoderRow(const QString &uid)
{
	scopy::decoder::DecoderEditor *editor = decoders()->editor(uid);
	if(!editor) {
		return;
	}
	InstrumentTemplate *it = m_ui->shell();
	const QString menuId = QStringLiteral("decoder:") + uid;
	MenuSectionCollapseWidget *section = it->createMenuSection(uid.toUpper(), SO_CH);
	section->add(editor);
	it->addMenuPage(menuId, withStretch({section}));
	MenuControlButton *row = it->addChannelRow(it->addChannelGroup(kDecodersGroup), uid, QColor(), menuId, 0,
						   InstrumentTemplate::CRS_SMALL);
	row->checkBox()->setChecked(true);
	connect(row->checkBox(), &QAbstractButton::toggled, this, [this, uid](bool on) {
		scopy::decoder::DecoderInstance *d = decoders()->manager()->find(uid);
		if(!d) {
			return;
		}
		if(d->proc) {
			d->proc->setEnabled(on);
		}
		for(scopy::adc::AcqAnnotationChannel *c : std::as_const(m_decodes)) {
			if(d->outKeys.contains(c->key())) {
				c->setEnabled(on);
			}
		}
		m_plot->replot();
	});
	m_decoderRows.insert(uid, row);
	editor->setSampleRate(m_sweep.sampleRate);
	row->click();
}

void M2kLogicController::removeDecoderRow(const QString &uid)
{
	if(MenuControlButton *row = m_decoderRows.take(uid)) {
		m_ui->shell()->removeChannelRow(m_ui->shell()->addChannelGroup(kDecodersGroup), row,
						QStringLiteral("decoder:") + uid, true);
	}
}

void M2kLogicController::removeDecodes(const QList<DataKey> &keys)
{
	for(int i = m_decodes.size() - 1; i >= 0; --i) {
		if(keys.contains(m_decodes[i]->key())) {
			m_plot->removeChannelRef(m_decodes[i]);
			delete m_decodes.takeAt(i);
		}
	}
	applyTrackScroll();
}

void M2kLogicController::syncDecoderRates()
{
	if(!decoders()) {
		return;
	}
	for(const scopy::decoder::DecoderInstance &d : decoders()->manager()->decoders()) {
		if(scopy::decoder::DecoderEditor *editor = decoders()->editor(d.uid)) {
			editor->setSampleRate(m_sweep.sampleRate);
		}
	}
}

int M2kLogicController::contentBottomPx() const
{
	int bottom = 0;
	for(scopy::adc::AcqDigitalChannel *c : m_dio) {
		if(c->isEnabled()) {
			bottom = std::max(bottom, qRound(-c->handle()->getPosition()) + kTrackPitchPx);
		}
	}
	for(scopy::adc::AcqAnnotationChannel *d : std::as_const(m_decodes)) {
		const int rows = std::max(1, d->rowCount());
		bottom = std::max(bottom, qRound(-d->handle()->getPosition()) + rows * kDecodeRowPx + kDecodeGapPx);
	}
	return bottom;
}

void M2kLogicController::applyTrackScroll()
{
	const int h = std::max(1, m_plot->plot()->plot()->canvas()->height());
	const int content = contentBottomPx();
	{
		QSignalBlocker block(m_scroll);
		m_scroll->setRange(0, std::max(0, content - h));
		m_scroll->setPageStep(h);
		m_scroll->setVisible(content > h);
	}
	const int top = m_scroll->value();
	m_plot->digitalAxis()->setInterval(-(top + h), -top);
	m_plot->replot();
}

bool M2kLogicController::eventFilter(QObject *watched, QEvent *event)
{
	if(event->type() == QEvent::Resize) {
		applyTrackScroll();
	}
	return AcqInstrumentController::eventFilter(watched, event);
}

void M2kLogicController::setChannelEnabled(int pin, bool on) { m_chRow[pin]->checkBox()->setChecked(on); }

bool M2kLogicController::isChannelEnabled(int pin) const { return m_dio[pin]->isEnabled(); }

void M2kLogicController::setChannelName(int pin, const QString &name)
{
	m_dio[pin]->setName(name.isEmpty() ? QStringLiteral("DIO %1").arg(pin) : name);
}

QString M2kLogicController::channelName(int pin) const { return m_dio[pin]->name(); }

void M2kLogicController::setTriggerCondition(int pin, int index)
{
	if(pin < 0 || pin >= CHANNELS || index < 0 || index >= kConditions.size()) {
		return;
	}
	{
		QSignalBlocker block(m_trigger[pin]->combo());
		m_trigger[pin]->combo()->setCurrentIndex(index);
	}
	m_source->set(m_source->channel(M2kSource::Digital, pin), "trigger_condition",
		      trigger::toName(trigger::DIGITAL_CONDITION_OPTIONS,
				      m_trigEnable->isChecked() ? kConditions[index].second
								: trigger::DIGITAL_CONDITION_NONE),
		      M2kSource::Restart);
}

int M2kLogicController::triggerCondition(int pin) const { return m_trigger[pin]->combo()->currentIndex(); }

void M2kLogicController::exportFile()
{
	const QString csv = tr("Comma-separated values files (*.csv)");
	const QString vcd = tr("Value Change Dump (*.vcd)");
	QString filter;
	const bool native = Preferences::get("general_use_native_dialogs").toBool();
	QString path = QFileDialog::getSaveFileName(m_ui, tr("Export"), QString(), csv + ";;" + vcd, &filter,
						    native ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog);
	if(path.isEmpty()) {
		return;
	}
	const QString ext = filter == vcd ? QStringLiteral(".vcd") : QStringLiteral(".csv");
	if(!path.endsWith(ext, Qt::CaseInsensitive)) {
		path += ext;
	}
	if(!exportData(path)) {
		StatusBarManager::pushMessage(tr("Logic Analyzer: nothing to export yet"));
	}
}

bool M2kLogicController::exportData(const QString &path) const
{
	QList<int> pins;
	QList<QVector<quint8>> bits;
	for(int pin = 0; pin < CHANNELS; ++pin) {
		if(!m_dio[pin]->isEnabled()) {
			continue;
		}
		QVector<quint8> b = m_ui->store()->windowAs<QVector<quint8>>(m_source->rawKey(M2kSource::Digital, pin),
									     m_sweep.plotWindow);
		if(!b.isEmpty()) {
			pins.append(pin);
			bits.append(std::move(b));
		}
	}
	if(pins.isEmpty()) {
		return false;
	}
	int n = bits.first().size();
	for(const QVector<quint8> &b : std::as_const(bits)) {
		n = std::min(n, static_cast<int>(b.size()));
	}

	QFile file(path);
	if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		return false;
	}
	QTextStream out(&file);
	const double sr = m_sweep.sampleRate;

	if(!path.endsWith(QStringLiteral(".vcd"), Qt::CaseInsensitive)) {
		const double offset = timeOffset();
		out << "Time (s)";
		for(int pin : std::as_const(pins)) {
			out << "," << channelName(pin);
		}
		out << "\n";
		for(int s = 0; s < n; ++s) {
			out << QString::number(offset + s / sr, 'g', 12);
			for(const QVector<quint8> &b : std::as_const(bits)) {
				out << "," << int(b[s]);
			}
			out << "\n";
		}
		return true;
	}

	auto id = [](int i) { return QChar('!' + i); };
	out << "$date " << QDateTime::currentDateTime().toString() << " $end\n";
	out << "$version Scopy $end\n";
	out << "$comment " << n << " samples acquired at " << sr << " Hz $end\n";
	out << "$timescale 1 ns $end\n";
	out << "$scope module Scopy $end\n";
	for(int i = 0; i < pins.size(); ++i) {
		const QString name = channelName(pins[i]).replace(QRegularExpression(QStringLiteral("\\s+")), "_");
		out << "$var wire 1 " << id(i) << " " << name << " $end\n";
	}
	out << "$upscope $end\n$enddefinitions $end\n";
	for(int s = 0; s < n; ++s) {
		QString changes;
		for(int i = 0; i < bits.size(); ++i) {
			if(s == 0 || bits[i][s] != bits[i][s - 1]) {
				changes += QStringLiteral(" %1%2").arg(int(bits[i][s])).arg(id(i));
			}
		}
		if(!changes.isEmpty()) {
			out << "#" << std::llround(s * 1e9 / sr) << changes << "\n";
		}
	}
	return true;
}

#include "moc_m2klogiccontroller.cpp"
