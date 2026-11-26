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
 */

#include "m2kosccontroller.h"

#include "horizontalsettings.h"
#include "m2kattributeio.h"
#include "m2kcomponentbuilder.h"
#include "m2kcontext.h"
#include "m2koschistogramprocessor.h"
#include "m2ktriggermath.h"
#include "uihelpers.h"

#include <core/acq_engine/acquisitionengine.h>
#include <core/acq_engine/datastore.h>
#include <core/acq_engine/genalyzerfftprocessor.h>
#include <core/acq_engine/spectrumprocessor.h>
#include <core/acq_engine/mathprocessor.h>
#include <core/acq_engine/scaleoffsetprocessor.h>
#include <core/acq_engine/snapshotsource.h>
#include <core/acq_ui/acqaxis.h>
#include <core/acq_ui/acqcurvechannel.h>
#include <core/acq_ui/acqinstrument.h>
#include <core/acq_ui/acqplot.h>
#include <core/acq_ui/filesourceblock.h>
#include <core/acq_ui/measuremarksitem.h>

#include <component/attribute.h>
#include <component/channel.h>
#include <component/device.h>

#include <gui/axishandle.h>
#include <gui/customSwitch.h>
#include <gui/cursorcontroller.h>
#include <gui/docking/dockablearea.h>
#include <gui/docking/dockwrapper.h>
#include <gui/instrumenttemplate.h>
#include <gui/plot_utils.hpp>
#include <gui/plotaxis.h>
#include <gui/plotaxishandle.h>
#include <gui/plotcursors.h>
#include <gui/plotnavigator.hpp>
#include <gui/plotwidget.h>
#include <gui/printplotmanager.h>
#include <gui/style.h>
#include <gui/widgets/cursorsettings.h>
#include <gui/widgets/measurementlabel.h>
#include <gui/widgets/flowlayout.h>
#include <gui/widgets/measurementselector.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menucombo.h>
#include <gui/widgets/menucontrolbutton.h>
#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menuplotchannelcurvestylecontrol.h>
#include <gui/widgets/menuspinbox.h>
#include <gui/widgets/plotinfo.h>
#include <gui/widgets/toolbuttons.h>
#include <pluginbase/preferences.h>
#include <pluginbase/statusbarmanager.h>

#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QFontMetrics>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
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
using scopy::acq::Measure;

namespace {
const QString kGeneralGroup = QStringLiteral("General");
const QString kChannelsGroup = QStringLiteral("Channels");
const QString kReferencesGroup = QStringLiteral("References");
constexpr int kMeasureListPx = 220;

constexpr int kFrameIntervalMs = 16;
constexpr int kMinSamples = 100;
constexpr int kMaxBuffer = 1 << 20;
constexpr int kTimeBaseSamples = 8192;
constexpr int kMaxWindow = 1 << 21;
constexpr double kStreamTimeBase = 0.1;
constexpr int kDelayMax = 1 << 20;
constexpr int kTrigDebounceMs = 300;
constexpr int kHwDebounceMs = 200;
constexpr int kTrigWaitMs = 250;
constexpr int kAutoTimeoutMs = 100;
constexpr unsigned kStreamingKernelBuffers = 4;
constexpr int kModeAlways = 0;
constexpr int kModeAnalog = 1;
constexpr int kModeExternal = 2;
constexpr int kModeDigitalOrAnalog = 3;
constexpr int kSourceCh1 = 0;
constexpr int kSourceDigital = 5;
constexpr int kSourceChOrDigital = 6;
constexpr int kHandleClearPx = 22;
constexpr double kDivsY = 10.0;
// The +-2.5 V range holds five low-range volts.
constexpr double kLowRangeSpan = 5.0;
constexpr double kHighRangeSpan = 50.0;
constexpr double kFftMinDb = -160.0;
constexpr int kMeasureMaxPx = 180;
// The widest a label gets, so they line up in columns.
const QString kMeasureTemplate = QStringLiteral("Cycle Mean :  -888.888 mV  ");
const QString kStatsTemplate = QStringLiteral("CH1 REF 8 Cycle Area  ");
constexpr double kFftMaxDb = 20.0;

const char *const kMeasureIcons[scopy::acq::MEASURE_COUNT] = {
	"period",      "frequency",   "min",	   "max",	 "peak_to_peak", "mean",    "cycle_mean", "rms",
	"cycle_rms",   "rms",	      "area",	   "cycle_area", "low",		 "high",    "amplitude",  "middle",
	"p_overshoot", "n_overshoot", "rise_time", "fall_time",	 "p_width",	 "n_width", "p_duty",	  "n_duty",
};

MenuSectionCollapseWidget *newSection(const QString &title)
{
	return new MenuSectionCollapseWidget(title, MenuCollapseSection::MHCW_ARROW,
					     MenuCollapseSection::MHW_BASEWIDGET);
}

QWidget *labeledRow(const QString &name, QWidget *control, QWidget *parent)
{
	auto *w = new QWidget(parent);
	auto *lay = new QHBoxLayout(w);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->addWidget(new QLabel(name, w));
	lay->addStretch();
	control->setParent(w);
	lay->addWidget(control);
	return w;
}

MenuCombo *conditionCombo(int count, QWidget *parent)
{
	auto *c = new MenuCombo(QObject::tr("Condition"), parent);
	const QString names[] = {QObject::tr("Rising edge"), QObject::tr("Falling edge"), QObject::tr("Low"),
				 QObject::tr("High"), QObject::tr("Any edge")};
	for(int i = 0; i < count; ++i) {
		c->combo()->addItem(names[i]);
	}
	return c;
}

int labelId(int uid, Measure m) { return uid * scopy::acq::MEASURE_COUNT + static_cast<int>(m); }

QList<double> availableRates(QObject *dev)
{
	QList<double> out;
	QString why;
	for(const QString &v : attr::readString(dev, "available_sample_rates", why).value_or(QString()).split(',')) {
		bool ok = false;
		const double rate = v.toDouble(&ok);
		if(ok && rate > 0) {
			out.append(rate);
		}
	}
	return out;
}

QString number(double v) { return QString::number(v, 'g', 12); }

} // namespace

M2kOscController::M2kOscController(ToolMenuEntry *tme, M2kContext *ctx, const QString &uri, QObject *parent)
	: M2kInstrumentController(tme, ctx, {QStringLiteral("adalm2000-adc:") + uri}, tr("Oscilloscope"), parent)
{
	m_limits.rates = sweep::series125(osc::MIN_SAMPLE_RATE, osc::MAX_SAMPLE_RATE);
	m_limits.minSamples = kMinSamples;
	m_limits.maxBuffer = kMaxBuffer;
	m_limits.maxWindow = kMaxWindow;
	m_limits.streamTimeBase = kStreamTimeBase;
	m_limits.timeBaseSamples = kTimeBaseSamples;
	m_limits.delayMin = trigger::DELAY_MIN;
	m_limits.delayMax = kDelayMax;
	m_sweep = sweep::fromTimeBase(100e-6, 0.0, 10, m_limits);
	m_shownDelay = m_sweep.delay;
}

M2kOscController::~M2kOscController()
{
	AcqInstrumentController::stop();
	for(Trace *t : std::as_const(m_traces)) {
		delete t->curve;
		delete t;
	}
	for(ViewState &v : m_views) {
		qDeleteAll(v.curves);
	}
}

void M2kOscController::setupBlocks(scopy::component::Context *ctx)
{
	Q_UNUSED(ctx)
	InstrumentTemplate *it = m_ui->shell();
	for(const QString &group : {kGeneralGroup, kChannelsGroup, kReferencesGroup}) {
		it->addChannelGroup(group);
	}

	m_source = new M2kSource(m_ctx, QStringLiteral("osc"), m_ui->engine());
	m_ui->engine()->addSource(m_source);
	component::Device *dev = analogIn();
	m_source->enableChannel(M2kSource::timeChannelId(), true);
	for(int ch = 0; ch < CHANNELS; ++ch) {
		m_source->enableChannel(M2kSource::channelId(M2kSource::Analog, ch), true);
	}
	QString why;
	m_hasExternal = attr::readDouble(dev, "has_external_trigger_in", why).value_or(0) != 0;
	component::Attribute *source = attr::find(dev, "trigger_source");
	m_hasLogic =
		source && source->options().contains(trigger::toName(trigger::ANALOG_SOURCE_OPTIONS, kSourceDigital));
	m_hasExternalOut = attr::find(dev, "trigger_external_out_select") != nullptr;
	m_hwRates = availableRates(dev);
	applyRate(m_sweep.sampleRate);
	applyStreaming();
	m_file = new scopy::adc::FileSourceBlock(QStringLiteral("file"), m_ui->engine());
	m_ui->engine()->addSource(m_file);
	m_snap = new scopy::acq::SnapshotSource(QStringLiteral("snapshot"), m_ui->engine());
	m_ui->engine()->addSource(m_snap);
	m_ui->setBufferSize(m_sweep.bufferSize);
}

void M2kOscController::setupProcessors()
{
	m_probe = new scopy::acq::ScaleOffsetProcessor(QStringLiteral("probe"), m_ui->engine());
	for(int ch = 0; ch < CHANNELS; ++ch) {
		m_probeCfg[ch] =
			m_probe->addChannel(m_source->rawKey(M2kSource::Analog, ch), probedKey(ch), channelName(ch));
	}
	m_ui->engine()->addProcessor(m_probe);
	m_hist = new M2kOscHistogramProcessor({probedKey(0), probedKey(1)}, m_ui->engine());
	m_hist->setEnabled(false);
	m_ui->engine()->addProcessor(m_hist);
	for(int ch = 0; ch < CHANNELS; ++ch) {
		m_fft[ch] = new scopy::acq::GenalyzerFFTProcessor(probedKey(ch), fftKey(ch, "fftdb"),
								  fftKey(ch, "fftf"), m_sweep.bufferSize,
								  m_sweep.sampleRate, GnWindowHann, m_ui->engine());
		m_spectrum[ch] = new scopy::acq::SpectrumProcessor(QStringLiteral("fft%1").arg(ch), fftKey(ch, "fftdb"),
								   fftKey(ch, "fft"), m_fft[ch], m_ui->engine());
		// Volts in, so 1 V full scale makes dBFS read as dBV.
		m_spectrum[ch]->setUnit(scopy::acq::SpectrumProcessor::Unit::DBV);
		m_spectrum[ch]->setFullScaleVolts(1.0);
		for(scopy::acq::ProcessorBlock *b : {static_cast<scopy::acq::ProcessorBlock *>(m_fft[ch]),
						     static_cast<scopy::acq::ProcessorBlock *>(m_spectrum[ch])}) {
			b->setEnabled(false);
			m_ui->engine()->addProcessor(b);
		}
	}
	m_measure = new scopy::acq::MeasureProcessor(m_ui->engine());
	m_measure->setSampleRate(m_sweep.sampleRate);
	m_ui->engine()->addProcessor(m_measure);
	connect(m_measure, &scopy::acq::MeasureProcessor::measured, this, &M2kOscController::onMeasured,
		Qt::QueuedConnection);
}

void M2kOscController::setupPlots()
{
	setupCenter();
	setupMeasurements();
	for(int ch = 0; ch < CHANNELS; ++ch) {
		addTrace(Kind::Channel, channelName(ch), probedKey(ch));
	}
	m_marksTrace = m_traces[0];
	setupViews();
	setupTopRail();
	InstrumentTemplate *it = m_ui->shell();
	MenuSectionCollapseWidget *group = it->addChannelGroup(kGeneralGroup);
	MenuControlButton *general =
		it->addChannelRow(group, tr("General settings"), QColor(), QStringLiteral("general"));
	general->checkBox()->setVisible(false);
	it->addMenuPage(QStringLiteral("general"), buildGeneralPage());
	MenuControlButton *trig = it->addChannelRow(group, tr("Trigger"), QColor(), QStringLiteral("trigger"));
	m_trigUi.enable = trig->checkBox();
	m_trigUi.enable->setChecked(m_trigger.enabled);
	m_trigStatus = addRowStatus(trig);
	m_trigWaitTimer = new QTimer(this);
	m_trigWaitTimer->setSingleShot(true);
	connect(m_trigWaitTimer, &QTimer::timeout, this, [this]() { setTriggerStatus(tr("Waiting")); });
	setTriggerStatus(tr("Stop"));
	it->addMenuPage(QStringLiteral("trigger"), buildTriggerPage());
	addHoverButton(tr("Cursors"), buildCursorsPage());

	MenuSectionCollapseWidget *refs = it->addChannelGroup(kReferencesGroup);
	for(scopy::acq::SourceBlock *src :
	    {static_cast<scopy::acq::SourceBlock *>(m_file), static_cast<scopy::acq::SourceBlock *>(m_snap)}) {
		const QString id = src->id();
		it->addChannelRow(refs, id.left(1).toUpper() + id.mid(1), QColor(), id)->checkBox()->setVisible(false);
		it->addMenuPage(id, withStretch({src->settingsWidget()}));
	}
	MenuSectionCollapseWidget *mathSection = newSection(tr("MATH"));
	auto *hint = new QLabel(tr("X1 is CH1, X2 is CH2 and S the sample index."), mathSection);
	hint->setWordWrap(true);
	Style::setStyle(hint, style::properties::label::subtle);
	QPushButton *add = button(tr("Add math channel"), mathSection);
	mathSection->add(hint);
	mathSection->add(add);
	it->addChannelRow(refs, tr("Math"), QColor(), QStringLiteral("addmath"))->checkBox()->setVisible(false);
	it->addMenuPage(QStringLiteral("addmath"), withStretch({mathSection}));
	connect(add, &QPushButton::clicked, this, [this]() { addMath(QStringLiteral("X1 - X2")); });
	connect(m_file, &scopy::adc::FileSourceBlock::slotsChanged, this, &M2kOscController::syncReferences);
	connect(m_file, &scopy::adc::FileSourceBlock::published, this, &M2kOscController::syncReferences);
	connect(m_snap, &scopy::acq::SnapshotSource::slotsChanged, this, &M2kOscController::syncReferences);
	connect(m_snap, &scopy::acq::SnapshotSource::slotCaptured, this, &M2kOscController::syncReferences);

	connect(m_ui, &scopy::adc::AcqInstrument::started, this, &M2kOscController::onStarted);
	connect(m_ui, &scopy::adc::AcqInstrument::cycleComplete, this, &M2kOscController::onCycleComplete);
	connect(m_ui, &scopy::adc::AcqInstrument::stopped, this, &M2kOscController::onStopped);
	connect(m_source, &M2kSource::sampleRateChanged, this, [this](int input, double hz) {
		if(input != M2kSource::Analog) {
			return;
		}
		m_sweep.sampleRate = hz;
		m_horizontal->setSweep(m_sweep);
		m_measure->setSampleRate(hz);
		for(scopy::acq::GenalyzerFFTProcessor *f : m_fft) {
			f->setSampleRate(hz);
		}
		applyXRange();
	});

	setupCalibrationGate();
	applyHardware();
	applyTriggerUi();
	applyXRange();
	general->click();
}

void M2kOscController::setupCenter()
{
	InstrumentTemplate *it = m_ui->shell();
	auto *center = new QWidget(m_ui);
	auto *lay = new QVBoxLayout(center);
	lay->setContentsMargins(0, 0, 0, 0);
	DockableAreaInterface *area = createDockableArea(center);
	Style::setBackgroundColor(area->asWidget(), json::theme::background_subtle, true);
	auto addDock = [area, center](const QString &name, int id, DockWrapperInterface *beside,
				      DockableAreaInterface::Direction dir) {
		auto *plot = new scopy::adc::AcqPlot(name, scopy::adc::AcqPlotKind::Basic, id, center);
		DockWrapperInterface *dock = createDockWrapper(name);
		dock->setTitle(name);
		dock->setInnerWidget(plot->plot());
		area->addDockWrapperBeside(dock, beside, dir);
		return std::pair{plot, dock};
	};
	DockWrapperInterface *time = nullptr;
	std::tie(m_plot, time) = addDock(tr("Time"), 0, nullptr, DockableAreaInterface::Direction_RIGHT);
	std::tie(m_views[ViewXY].plot, m_views[ViewXY].dock) =
		addDock(tr("XY"), 1, time, DockableAreaInterface::Direction_RIGHT);
	std::tie(m_views[ViewFFT].plot, m_views[ViewFFT].dock) =
		addDock(tr("FFT"), 3, time, DockableAreaInterface::Direction_BOTTOM);
	std::tie(m_views[ViewHistogram].plot, m_views[ViewHistogram].dock) =
		addDock(tr("Histogram"), 2, m_views[ViewXY].dock, DockableAreaInterface::Direction_BOTTOM);
	for(ViewState &v : m_views) {
		v.dock->setActivated(false);
	}
	lay->addWidget(area->asWidget());
	it->setCenterWidget(center);
	PlotWidget *pw = m_plot->plot();
	pw->xAxis()->setVisible(false);
	pw->yAxis()->setVisible(false);
	m_plotStatus = pw->getPlotInfo()->addLabelInfo(IP_LEFT, IP_TOP);
	m_sweepInfo = pw->getPlotInfo()->addLabelInfo(IP_LEFT, IP_TOP);
	for(QLabel *l : {m_plotStatus.data(), m_sweepInfo.data()}) {
		l->setIndent(kHandleClearPx);
	}

	m_delayTimer = new QTimer(this);
	m_delayTimer->setSingleShot(true);
	m_delayTimer->setInterval(kTrigDebounceMs);
	connect(m_delayTimer, &QTimer::timeout, this, [this]() { setDelay(qRound(-m_trigFrac * m_sweep.bufferSize)); });
	m_hwTimer = new QTimer(this);
	m_hwTimer->setSingleShot(true);
	m_hwTimer->setInterval(kHwDebounceMs);
	connect(m_hwTimer, &QTimer::timeout, this, &M2kOscController::applyHardware);
	m_frameTimer = new QTimer(this);
	m_frameTimer->setInterval(kFrameIntervalMs);
	connect(m_frameTimer, &QTimer::timeout, this, [this]() {
		if(m_dirty) {
			m_dirty = false;
			m_plot->replot();
			for(const ViewState &v : m_views) {
				if(v.on) {
					v.plot->replot();
				}
			}
		}
	});
}

M2kOscController::Trace *M2kOscController::addTrace(Kind kind, const QString &name, const DataKey &key,
						    scopy::acq::MathProcessor *math)
{
	InstrumentTemplate *it = m_ui->shell();
	auto *t = new Trace;
	t->kind = kind;
	t->uid = m_uid++;
	t->name = name;
	t->menuId = QStringLiteral("trace%1").arg(t->uid);
	t->key = key;
	t->math = math;

	const DataKey infoKey = kind == Kind::Channel ? m_source->rawKey(M2kSource::Analog, m_traces.size()) : key;
	t->curve = newCurve(m_plot, key, m_source->timeKey(), infoKey, m_traces.size(), m_sweep.plotWindow, t->uid);
	t->curve->setName(name);
	t->curve->xAxis()->setUnit(QStringLiteral("s"));
	t->curve->xAxis()->setRangeLocked(true);
	t->curve->yAxis()->setRangeLocked(true);
	m_traces.append(t);

	if(!m_timeHandle) {
		PlotWidget *pw = m_plot->plot();
		PlotAxis *x = t->curve->xAxis()->plotAxis();
		x->getFormatter()->setTrimZeroes(true);
		x->setVisible(true);
		QwtScaleWidget *scale = pw->plot()->axisWidget(x->axisId());
		const int labelPx = QFontMetrics(scale->font()).horizontalAdvance(QStringLiteral("-888.888 ms"));
		scale->setMinBorderDist(labelPx / 2, labelPx / 2);
		pw->selectChannel(t->curve->plotChannel());

		m_timeHandle = new PlotAxisHandle(pw, x);
		m_timeHandle->handle()->setColor(Style::getColor(json::theme::interactive_primary_idle));
		pw->addPlotAxisHandle(m_timeHandle);
		// Pan the axis under the handle, keeping the width.
		connect(m_timeHandle, &PlotAxisHandle::scalePosChanged, this, [this, x](double pos) {
			const QwtScaleMap map = m_plot->plot()->plot()->canvasMap(x->axisId());
			const double width = map.s2() - map.s1();
			m_trigFrac = std::clamp((pos - map.s1()) / width, 0.0, 1.0);
			x->setInterval(-m_trigFrac * width, (1.0 - m_trigFrac) * width);
			m_timeHandle->setPositionSilent(0.0);
			m_plot->replot();
			m_delayTimer->start();
		});
		m_levelHandle = new PlotAxisHandle(pw, t->curve->yAxis()->plotAxis());
		pw->addPlotAxisHandle(m_levelHandle);
		connect(m_levelHandle, &PlotAxisHandle::scalePosChanged, this, [this](double pos) {
			if(m_trigUi.level) {
				m_trigUi.level->setValue(pos);
			}
		});
	}

	t->row = it->addChannelRow(it->addChannelGroup(groupFor(kind)), name, t->curve->color(), t->menuId);
	it->addMenuPage(t->menuId, buildTracePage(t));
	t->row->checkBox()->setChecked(true);
	connect(t->row->checkBox(), &QAbstractButton::toggled, this, [this, t](bool on) { applyEnabled(t, on); });
	connect(t->row, &QAbstractButton::clicked, this, [this, t]() {
		m_plot->plot()->selectChannel(t->curve->plotChannel());
		m_marksTrace = t;
		drawMarks();
	});
	applyVertical(t);
	if(!m_ui->engine()->isRunning()) {
		t->curve->pull(m_sweep.plotWindow);
	}
	refreshXYSources();
	return t;
}

scopy::adc::AcqCurveChannel *M2kOscController::newCurve(scopy::adc::AcqPlot *plot, const DataKey &key,
							const DataKey &xKey, const DataKey &infoKey, int colorIndex,
							int depth, int uid)
{
	scopy::adc::AcqChannel::Args args;
	args.store = m_ui->store();
	args.engine = m_ui->engine();
	args.key = key;
	args.xKey = xKey;
	args.info = m_ui->engine()->streamInfo(infoKey).value_or(scopy::acq::StreamInfo{});
	args.info.kind = scopy::acq::ReprKind::Curve;
	args.info.colorIndex = colorIndex;
	args.uid = uid;
	auto *c = new scopy::adc::AcqCurveChannel(args);
	plot->addChannelRef(c);
	c->attach(plot);
	c->reclaimDepth(depth);
	c->setKeyPresent(true);
	c->xAxis()->setSourceFixed(true);
	c->yAxis()->setSourceFixed(true);
	c->xAxis()->setAutoscale(false);
	c->yAxis()->setAutoscale(false);
	c->xAxis()->setUnit(QStringLiteral("V"));
	c->yAxis()->setUnit(QStringLiteral("V"));
	return c;
}

void M2kOscController::setupViews()
{
	ViewState &hist = m_views[ViewHistogram];
	ViewState &fft = m_views[ViewFFT];
	for(int ch = 0; ch < CHANNELS; ++ch) {
		auto *h = newCurve(hist.plot, m_hist->countsKey(ch), m_hist->levelsKey(ch), m_hist->countsKey(ch), ch,
				   M2kOscHistogramProcessor::BINS, m_uid++);
		h->yAxis()->setAutoscale(true);
		h->yAxis()->setUnit(QString());
		auto *f = newCurve(fft.plot, fftKey(ch, "fft"), fftKey(ch, "fftf"), fftKey(ch, "fft"), ch,
				   m_sweep.bufferSize / 2 + 1, m_uid++);
		f->xAxis()->setUnit(QStringLiteral("Hz"));
		f->yAxis()->setUnit(QStringLiteral("dBV"));
		f->yAxis()->setInterval(kFftMinDb, kFftMaxDb);
		for(scopy::adc::AcqCurveChannel *c : {h, f}) {
			c->setName(channelName(ch));
			c->setEnabled(false);
		}
		hist.curves.append(h);
		fft.curves.append(f);
	}
	for(ViewState *v : {&hist, &fft}) {
		v->curves[0]->xAxis()->plotAxis()->setVisible(true);
		v->plot->plot()->selectChannel(v->curves[0]->plotChannel());
	}
	rebuildXY();
	applyViews();
}

void M2kOscController::rebuildXY()
{
	ViewState &xy = m_views[ViewXY];
	const Trace *x = nullptr;
	const Trace *y = nullptr;
	for(const Trace *t : std::as_const(m_traces)) {
		x = t->uid == m_xyUid[0] ? t : x;
		y = t->uid == m_xyUid[1] ? t : y;
	}
	if(!x || !y) {
		return;
	}
	for(scopy::adc::AcqCurveChannel *c : std::as_const(xy.curves)) {
		if(m_xyStyle) {
			m_xyStyle->removeChannels(c->plotChannel());
		}
		xy.plot->removeChannelRef(c);
		delete c;
	}
	auto *c = newCurve(xy.plot, y->key, x->key, y->key, 0, m_sweep.bufferSize, m_uid++);
	c->setName(tr("%1 vs %2").arg(y->name, x->name));
	c->xAxis()->plotAxis()->setVisible(true);
	xy.plot->plot()->selectChannel(c->plotChannel());
	xy.curves = {c};
	if(m_xyStyle) {
		m_xyStyle->addChannels(c->plotChannel());
	}
	applyViews();
}

void M2kOscController::refreshXYSources()
{
	for(int axis = 0; axis < 2; ++axis) {
		if(!m_xyCombo[axis]) {
			continue;
		}
		QComboBox *combo = m_xyCombo[axis]->combo();
		const QSignalBlocker block(combo);
		combo->clear();
		for(const Trace *t : std::as_const(m_traces)) {
			if(t->kind != Kind::Reference) {
				combo->addItem(t->name, t->uid);
			}
		}
		const int i = combo->findData(m_xyUid[axis]);
		if(i < 0) {
			m_xyUid[axis] = m_traces[axis]->uid;
			rebuildXY();
		}
		combo->setCurrentIndex(std::max(0, combo->findData(m_xyUid[axis])));
	}
}

void M2kOscController::setXYSources(int x, int y)
{
	const Trace *tx = trace(x);
	const Trace *ty = trace(y);
	if(!tx || !ty || tx->kind == Kind::Reference || ty->kind == Kind::Reference) {
		return;
	}
	m_xyUid[0] = tx->uid;
	m_xyUid[1] = ty->uid;
	refreshXYSources();
	rebuildXY();
}

int M2kOscController::xySource(int axis) const
{
	for(int i = 0; i < m_traces.size(); ++i) {
		if(m_traces[i]->uid == m_xyUid[std::clamp(axis, 0, 1)]) {
			return i;
		}
	}
	return -1;
}

void M2kOscController::setViewEnabled(int view, bool on)
{
	if(view < 0 || view >= VIEW_COUNT) {
		return;
	}
	m_views[view].on = on;
	if(m_views[view].sw && m_views[view].sw->isChecked() != on) {
		m_views[view].sw->setChecked(on);
	}
	applyViews();
}

void M2kOscController::applyViews()
{
	if(m_traces.size() < CHANNELS || m_views[ViewXY].curves.isEmpty()) {
		return;
	}
	const Trace *x = trace(xySource(0));
	const Trace *y = trace(xySource(1));
	ViewState &xy = m_views[ViewXY];
	if(x && y) {
		scopy::adc::AcqCurveChannel *c = xy.curves[0];
		c->setEnabled(xy.on && x->enabled && y->enabled);
		c->xAxis()->setInterval(yRange(x).first, yRange(x).second);
		c->yAxis()->setInterval(yRange(y).first, yRange(y).second);
	}

	QList<DataKey> histKeys;
	for(int ch = 0; ch < CHANNELS; ++ch) {
		const Trace *t = m_traces[ch];
		const auto [lo, hi] = yRange(t);
		const bool hist = m_views[ViewHistogram].on && t->enabled;
		if(hist) {
			histKeys.append(probedKey(ch));
		}
		m_views[ViewHistogram].curves[ch]->setEnabled(hist);
		m_hist->setRange(ch, lo, hi);
		m_views[ViewHistogram].curves[ch]->xAxis()->setInterval(lo, hi);

		const bool fft = m_views[ViewFFT].on && t->enabled;
		m_fft[ch]->setEnabled(fft);
		m_spectrum[ch]->setEnabled(fft);
		m_views[ViewFFT].curves[ch]->setEnabled(fft);
	}
	m_hist->setWatchedKeys(histKeys);
	m_hist->setEnabled(!histKeys.isEmpty());
	for(ViewState &v : m_views) {
		v.dock->setActivated(v.on);
		v.plot->replot();
	}
}

DataKey M2kOscController::fftKey(int ch, const QString &stage) const
{
	return DataKey::withStage(m_source->id(), M2kSource::channelId(M2kSource::Analog, ch), stage);
}

DataKey M2kOscController::probedKey(int ch) const
{
	return DataKey::withStage(m_source->id(), M2kSource::channelId(M2kSource::Analog, ch), QStringLiteral("probe"));
}

std::pair<double, double> M2kOscController::yRange(const Trace *t)
{
	const double half = t->voltsPerDiv * kDivsY / 2.0;
	return {-half - t->position, half - t->position};
}

QString M2kOscController::groupFor(Kind kind) const
{
	return kind == Kind::Channel ? kChannelsGroup : kReferencesGroup;
}

QString M2kOscController::traceName(int i) const
{
	const Trace *t = trace(i);
	return t ? t->name : QString();
}

void M2kOscController::setupTopRail()
{
	InstrumentTemplate *it = m_ui->shell();
	connect(it->infoBtn(), &QAbstractButton::clicked, this, []() {
		QDesktopServices::openUrl(
			QUrl(QStringLiteral("https://analogdevicesinc.github.io/scopy/plugins/m2k/oscilloscope.html")));
	});
	QPushButton *autosetBtn = button(tr("Autoset"), it);
	it->addEngineControl(autosetBtn);
	connect(autosetBtn, &QPushButton::clicked, this, &M2kOscController::autoset);
	connect(this, &M2kOscController::autosetFinished, autosetBtn, [autosetBtn]() { autosetBtn->setEnabled(true); });
	connect(autosetBtn, &QPushButton::clicked, autosetBtn, [autosetBtn]() { autosetBtn->setEnabled(false); });

	auto *printManager = new PrintPlotManager(this);
	connect(it->printBtn(), &QAbstractButton::clicked, this, [this, printManager]() {
		QList<PlotWidget *> plots{m_plot->plot()};
		for(const ViewState &v : m_views) {
			if(v.on) {
				plots.append(v.plot->plot());
			}
		}
		printManager->printPlots(plots, QStringLiteral("Oscilloscope"));
	});
	connect(it->exportBtn(), &QAbstractButton::clicked, this, &M2kOscController::exportFile);
}

QWidget *M2kOscController::buildTracePage(Trace *t)
{
	QList<QWidget *> sections;
	if(t->math) {
		auto *math = newSection(tr("FORMULA"));
		math->add(t->math->settingsWidget());
		sections.append(math);
	}
	auto *vertical = newSection(tr("VERTICAL"));
	t->voltsPerDivSpin = spin(tr("Volts/div"), t->voltsPerDiv, QStringLiteral("V"), 1e-3, 1e3,
				  gui::MenuSpinbox::IS_125, vertical);
	t->positionSpin =
		spin(tr("Position"), t->position, QStringLiteral("V"), -1e3, 1e3, gui::MenuSpinbox::IS_FIXED, vertical);
	vertical->add(t->voltsPerDivSpin);
	vertical->add(t->positionSpin);
	sections.append(vertical);
	connect(t->voltsPerDivSpin, &gui::MenuSpinbox::valueChanged, this, [this, t](double v) {
		t->voltsPerDiv = v;
		applyVertical(t);
	});
	connect(t->positionSpin, &gui::MenuSpinbox::valueChanged, this, [this, t](double v) {
		t->position = v;
		applyVertical(t);
	});

	if(t->kind == Kind::Channel) {
		const int ch = m_traces.indexOf(t);
		auto *input = newSection(tr("INPUT"));
		m_probeUi[ch] =
			spin(tr("Probe"), probe(ch), QStringLiteral("x"), 1e-3, 1e4, gui::MenuSpinbox::IS_125, input);
		m_probeUi[ch]->setScalingEnabled(false);
		m_couplingUi[ch] = new CustomSwitch(tr("DC"), tr("AC"), input);
		m_couplingUi[ch]->setChecked(true);
		input->add(m_probeUi[ch]);
		input->add(labeledRow(tr("Coupling"), m_couplingUi[ch], input));
		sections.append(input);
		connect(m_probeUi[ch], &gui::MenuSpinbox::valueChanged, this,
			[this, ch](double v) { setProbe(ch, v); });
		connect(m_couplingUi[ch], &QAbstractButton::toggled, this,
			[this, ch](bool dc) { setAcCoupled(ch, !dc); });
	}

	auto *curve = newSection(tr("CURVE"));
	auto *style = new gui::MenuPlotChannelCurveStyleControl(curve);
	style->addChannels(t->curve->plotChannel());
	curve->add(style);
	sections.append(curve);

	auto *actions = newSection(t->kind == Kind::Reference ? tr("REFERENCE") : tr("ACTIONS"));
	if(t->kind != Kind::Reference) {
		QPushButton *snap = button(tr("Snapshot as reference"), actions);
		actions->add(snap);
		connect(snap, &QPushButton::clicked, this, [this, t]() {
			if(snapshot(m_traces.indexOf(t)) < 0) {
				StatusBarManager::pushMessage(
					tr("Oscilloscope: run first, %1 has no data yet").arg(t->name));
			}
		});
	}
	if(t->kind != Kind::Channel) {
		QPushButton *remove = button(tr("Remove"), actions);
		actions->add(remove);
		connect(remove, &QPushButton::clicked, this, [this, t]() { removeTrace(m_traces.indexOf(t)); });
	}
	sections.append(actions);
	if(t->kind != Kind::Reference) {
		sections.append(buildMeasureSection(t));
	}
	return withStretch(sections);
}

QWidget *M2kOscController::buildGeneralPage()
{
	MenuSectionCollapseWidget *section = newSection(tr("HORIZONTAL"));
	const int divs = qRound(m_traces[0]->curve->xAxis()->plotAxis()->divs());
	m_horizontal = new HorizontalSettings(m_limits, divs, HorizontalSettings::TimeBase, section);
	m_horizontal->setSweep(m_sweep);
	section->add(m_horizontal);
	connect(m_horizontal, &HorizontalSettings::sweepRequested, this, &M2kOscController::setSweep);
	connect(m_horizontal, &HorizontalSettings::timeBaseSamplesChanged, this,
		[this](int n) { m_limits.timeBaseSamples = n; });

	QList<QWidget *> sections{section};
	const QString names[VIEW_COUNT] = {tr("XY PLOT"), tr("HISTOGRAM PLOT"), tr("FFT PLOT")};
	for(int v : {ViewXY, ViewFFT, ViewHistogram}) {
		auto *plot = new MenuSectionCollapseWidget(names[v], MenuCollapseSection::MHCW_ONOFF,
							   MenuCollapseSection::MHW_BASEWIDGET);
		if(v == ViewXY) {
			const QString axes[2] = {tr("X source"), tr("Y source")};
			for(int axis = 0; axis < 2; ++axis) {
				m_xyCombo[axis] = new MenuCombo(axes[axis], plot);
				plot->add(m_xyCombo[axis]);
				connect(m_xyCombo[axis]->combo(), &QComboBox::currentIndexChanged, this,
					[this, axis](int i) {
						m_xyUid[axis] = m_xyCombo[axis]->combo()->itemData(i).toInt();
						rebuildXY();
					});
			}
		}
		auto *style = new gui::MenuPlotChannelCurveStyleControl(plot);
		for(scopy::adc::AcqCurveChannel *c : std::as_const(m_views[v].curves)) {
			style->addChannels(c->plotChannel());
		}
		plot->add(style);
		if(v == ViewXY) {
			m_xyStyle = style;
		}
		m_views[v].sw = plot->collapseSection()->header();
		m_views[v].sw->setChecked(m_views[v].on);
		connect(m_views[v].sw, &QAbstractButton::toggled, this, [this, v](bool on) { setViewEnabled(v, on); });
		sections.append(plot);
	}
	refreshXYSources();
	return withStretch(sections);
}

QWidget *M2kOscController::buildTriggerPage()
{
	MenuSectionCollapseWidget *section = newSection(tr("TRIGGER"));
	m_trigUi.mode = new CustomSwitch(tr("Auto"), tr("Normal"), section);
	m_trigUi.mode->setChecked(m_trigger.autoMode);
	m_trigUi.mode->setToolTip(tr("Auto draws an untriggered buffer when no trigger comes"));
	section->add(labeledRow(tr("Mode"), m_trigUi.mode, section));

	auto *analog = new MenuSectionCollapseWidget(tr("ANALOG"), MenuCollapseSection::MHCW_ONOFF,
						     MenuCollapseSection::MHW_BASEWIDGET);
	m_trigUi.analog = analog->collapseSection()->header();
	m_trigUi.analog->setChecked(m_trigger.analog);
	m_trigUi.channel = new CustomSwitch(channelName(0), channelName(1), analog);
	m_trigUi.channel->setChecked(m_trigger.channel == 0);
	m_trigUi.condition = conditionCombo(4, analog);
	m_trigUi.level =
		spin(tr("Level"), m_trigger.level, QStringLiteral("V"), -25, 25, gui::MenuSpinbox::IS_FIXED, analog);
	m_trigUi.hysteresis = spin(tr("Hysteresis"), m_trigger.hysteresis, QStringLiteral("V"), 0.0, 2.5,
				   gui::MenuSpinbox::IS_125, analog);
	analog->add(labeledRow(tr("Source"), m_trigUi.channel, analog));
	analog->add(m_trigUi.condition);
	analog->add(m_trigUi.level);
	analog->add(m_trigUi.hysteresis);

	MenuSectionCollapseWidget *digital = newSection(tr("DIGITAL"));
	m_trigUi.digital = new MenuCombo(tr("Source"), digital);
	m_trigUi.digital->combo()->addItem(tr("None"), osc::DigitalOff);
	if(m_hasExternal) {
		m_trigUi.digital->combo()->addItem(tr("Trigger in"), osc::DigitalExternal);
	}
	if(m_hasLogic) {
		m_trigUi.digital->combo()->addItem(tr("Logic Analyzer"), osc::DigitalLogic);
	}
	m_trigUi.externalCondition = conditionCombo(5, digital);
	m_trigUi.logic = new MenuCombo(tr("Analog / digital"), digital);
	m_trigUi.logic->combo()->addItems({"OR", "AND", "XOR", "NOR", "NAND", "XNOR"});
	digital->add(m_trigUi.digital);
	digital->add(m_trigUi.externalCondition);
	digital->add(m_trigUi.logic);
	if(m_hasExternalOut) {
		m_trigUi.out = new MenuCombo(tr("Trigger out"), digital);
		m_trigUi.out->combo()->addItem(tr("None"), 0);
		m_trigUi.out->combo()->addItem(tr("Trigger in"), 2);
		m_trigUi.out->combo()->addItem(tr("Oscilloscope"), 3);
		m_trigUi.out->combo()->addItem(tr("Logic Analyzer"), 4);
		digital->add(m_trigUi.out);
		connect(m_trigUi.out->combo(), &QComboBox::currentIndexChanged, this,
			&M2kOscController::applyTriggerUi);
	}
	digital->setVisible(m_trigUi.digital->combo()->count() > 1 || m_trigUi.out);

	connect(m_trigUi.enable, &QAbstractButton::toggled, this, &M2kOscController::applyTriggerUi);
	for(QAbstractButton *b : {m_trigUi.analog.data(), static_cast<QAbstractButton *>(m_trigUi.mode.data()),
				  static_cast<QAbstractButton *>(m_trigUi.channel.data())}) {
		connect(b, &QAbstractButton::toggled, this, &M2kOscController::applyTriggerUi);
	}
	for(MenuCombo *c : {m_trigUi.condition.data(), m_trigUi.digital.data(), m_trigUi.externalCondition.data(),
			    m_trigUi.logic.data()}) {
		connect(c->combo(), &QComboBox::currentIndexChanged, this, &M2kOscController::applyTriggerUi);
	}
	for(gui::MenuSpinbox *s : {m_trigUi.level.data(), m_trigUi.hysteresis.data()}) {
		connect(s, &gui::MenuSpinbox::valueChanged, this, &M2kOscController::applyTriggerUi);
	}
	return withStretch({section, analog, digital});
}

QWidget *M2kOscController::buildCursorsPage()
{
	MenuSectionCollapseWidget *section = newSection(tr("CURSORS"));
	CursorSettings *settings = m_plot->cursorSettings(section);
	settings->getXEn()->setChecked(false);
	settings->getYEn()->setChecked(false);
	m_plot->setShowCursors(true);
	section->add(settings);
	m_gateSwitch = new MenuOnOffSwitch(tr("Gate measurements"), section, false);
	m_gateSwitch->setToolTip(tr("Measure only between the X cursors"));
	section->add(m_gateSwitch);
	m_cursorXEn = settings->getXEn();
	m_cursorYEn = settings->getYEn();
	connect(m_gateSwitch->onOffswitch(), &QAbstractButton::toggled, this, &M2kOscController::setGated);
	connect(m_cursorXEn, &QAbstractButton::toggled, this, &M2kOscController::applyGate);
	PlotCursors *cursors = m_plot->cursors()->getPlotCursors();
	for(PlotAxisHandle *h : {cursors->getX1Cursor(), cursors->getX2Cursor()}) {
		connect(h, &PlotAxisHandle::scalePosChanged, this, &M2kOscController::applyGate);
	}
	return withStretch({section});
}

void M2kOscController::applyEnabled(Trace *t, bool on)
{
	t->enabled = on;
	t->curve->setEnabled(on);
	if(t->kind == Kind::Channel) {
		m_source->enableChannel(M2kSource::channelId(M2kSource::Analog, m_traces.indexOf(t)), on);
		QList<DataKey> keys;
		for(int ch = 0; ch < CHANNELS; ++ch) {
			if(m_traces[ch]->enabled) {
				keys.append(m_source->rawKey(M2kSource::Analog, ch));
			}
		}
		m_probe->setWatchedKeys(keys);
		if(keys.isEmpty()) {
			AcqInstrumentController::stop();
		}
	} else if(t->math) {
		t->math->setEnabled(on);
	}
	syncMeasureKeys();
	applyViews();
	drawMarks();
	m_plot->replot();
}

int M2kOscController::addMath(const QString &formula)
{
	Trace *t = nullptr;
	restartIfRunning([this, &t, &formula]() {
		const int n = ++m_mathCount;
		auto *math = new scopy::acq::MathProcessor(QStringLiteral("math%1").arg(n), m_ui->engine());
		const DataKey key = DataKey::raw(QStringLiteral("math"), QStringLiteral("math%1").arg(n));
		math->setOutputKey(key);
		math->setWatchedKeys({probedKey(0), probedKey(1)});
		math->setFormula(formula);
		m_ui->engine()->addProcessor(math);
		t = addTrace(Kind::Math, tr("Math %1").arg(n), key, math);
	});
	connect(t->math, &scopy::acq::Block::enabledChanged, t->row->checkBox(), &QAbstractButton::setChecked);
	syncMeasureKeys();
	t->row->click();
	return m_traces.indexOf(t);
}

QStringList M2kOscController::mathFormulas() const
{
	QStringList out;
	for(const Trace *t : m_traces) {
		if(t->math) {
			out.append(t->math->evaluator().formula());
		}
	}
	return out;
}

int M2kOscController::snapshot(int i)
{
	const Trace *src = trace(i);
	if(!src || src->kind == Kind::Reference || !m_ui->store()->latestFloat(src->key)) {
		return -1;
	}
	const int slot = m_snap->addSlot();
	m_snap->setSlotTitle(slot,
			     scopy::acq::SnapshotSource::sanitizeTitle(
				     QStringLiteral("%1 REF %2").arg(src->name).arg(++m_refCount)));
	m_snap->setSlotSource(slot, src->key);
	m_snap->setSlotCyclic(slot, false);
	m_snap->capture(slot);
	const DataKey key = scopy::acq::SnapshotSource::keyFor(m_snap->id(), m_snap->slotAt(slot)->title);
	for(int k = 0; k < m_traces.size(); ++k) {
		if(m_traces[k]->key == key) {
			m_traces[k]->voltsPerDivSpin->setValue(src->voltsPerDiv);
			m_traces[k]->positionSpin->setValue(src->position);
			return k;
		}
	}
	return -1;
}

int M2kOscController::importReferences(const QString &path)
{
	const int before = m_traces.size();
	const int slot = m_file->addSlot();
	if(!m_file->setSlotFile(slot, path)) {
		m_file->removeSlot(slot);
	}
	return m_traces.size() - before;
}

void M2kOscController::syncReferences()
{
	QList<DataKey> wanted;
	const QStringList enabled = m_file->enabledChannels();
	for(const scopy::adc::FileSourceBlock::SlotInfo &slot : m_file->slotInfos()) {
		for(const QString &c : enabled) {
			if(c.startsWith(slot.title + '-')) {
				wanted.append(DataKey::raw(m_file->id(), c));
			}
		}
	}
	for(const scopy::acq::SnapshotSource::Slot &slot : m_snap->allSlots()) {
		if(slot.captured) {
			const DataKey key = scopy::acq::SnapshotSource::keyFor(m_snap->id(), slot.title);
			// Published here too, so a capture shows with the engine stopped.
			m_ui->store()->copy(key, slot.data);
			wanted.append(key);
		}
	}
	for(int i = m_traces.size() - 1; i >= CHANNELS; --i) {
		if(m_traces[i]->kind == Kind::Reference && !wanted.contains(m_traces[i]->key)) {
			removeTrace(i);
		}
	}
	for(const DataKey &key : std::as_const(wanted)) {
		const bool shown = std::any_of(m_traces.cbegin(), m_traces.cend(),
					       [&key](const Trace *t) { return t->key == key; });
		if(!shown) {
			addTrace(Kind::Reference, key.channelId(), key)->curve->pull(m_sweep.plotWindow);
		}
	}
	m_plot->replot();
}

void M2kOscController::removeTrace(int i)
{
	Trace *t = trace(i);
	if(!t || t->kind == Kind::Channel) {
		return;
	}
	for(int m = 0; m < scopy::acq::MEASURE_COUNT; ++m) {
		setMeasured(i, static_cast<Measure>(m), false);
		setStatsShown(i, static_cast<Measure>(m), false);
	}
	m_traces.removeOne(t);
	if(t->math) {
		restartIfRunning([this, t]() {
			m_ui->engine()->removeProcessor(t->math);
			delete t->math;
		});
	} else if(t->key.sourceId() == m_file->id()) {
		m_file->enableChannel(t->key.channelId(), false);
	} else {
		const QList<scopy::acq::SnapshotSource::Slot> all = m_snap->allSlots();
		for(int s = 0; s < all.size(); ++s) {
			if(scopy::acq::SnapshotSource::keyFor(m_snap->id(), all[s].title) == t->key) {
				m_snap->removeSlot(s);
				break;
			}
		}
	}
	if(m_marksTrace == t) {
		m_marksTrace = m_traces[0];
	}
	m_plot->removeChannelRef(t->curve);
	delete t->curve;
	// Deferred: the click that asked for this came from the trace's own page.
	QTimer::singleShot(0, this, [this, row = t->row, menuId = t->menuId, group = groupFor(t->kind)]() {
		m_ui->shell()->removeChannelRow(m_ui->shell()->addChannelGroup(group), row, menuId, true);
	});
	delete t;
	refreshXYSources();
	syncMeasureKeys();
	drawMarks();
	m_plot->replot();
}

void M2kOscController::onStarted()
{
	for(Trace *t : std::as_const(m_traces)) {
		t->curve->refreshSampleRate();
		t->curve->reset();
	}
	m_frameTimer->start();
	setTriggerStatus(m_trigger.enabled ? tr("Waiting") : tr("Running"));
}

void M2kOscController::onCycleComplete()
{
	if(m_ui->engine()->isRunning()) {
		trackTriggerDc();
		if(m_trigger.enabled) {
			setTriggerStatus(m_source->lastTimedOut(M2kSource::Analog) ? tr("Auto") : tr("Triggered"));
			const double bufferMs = 1000.0 * m_sweep.bufferSize / m_sweep.sampleRate;
			m_trigWaitTimer->start(kTrigWaitMs + static_cast<int>(bufferMs));
		}
		if(isStreaming()) {
			rollXRange();
		} else if(m_source->bufferDelay(M2kSource::Analog) != m_shownDelay) {
			m_shownDelay = m_source->bufferDelay(M2kSource::Analog);
			applyXRange();
		}
	}
	for(Trace *t : std::as_const(m_traces)) {
		t->curve->pull(m_sweep.plotWindow);
	}
	const int depth[VIEW_COUNT] = {m_sweep.bufferSize, M2kOscHistogramProcessor::BINS, m_sweep.bufferSize / 2 + 1};
	for(int v = 0; v < VIEW_COUNT; ++v) {
		if(m_views[v].on) {
			for(scopy::adc::AcqCurveChannel *c : std::as_const(m_views[v].curves)) {
				c->pull(depth[v]);
			}
		}
	}
	m_dirty = true;
}

void M2kOscController::onStopped()
{
	m_frameTimer->stop();
	onCycleComplete();
	for(Trace *t : std::as_const(m_traces)) {
		t->curve->onStopped();
	}
	m_trigWaitTimer->stop();
	setTriggerStatus(tr("Stop"));
	m_plot->replot();
}

void M2kOscController::single()
{
	if(!m_ui->engine()->isRunning()) {
		m_ui->single((m_sweep.plotWindow + m_sweep.bufferSize - 1) / m_sweep.bufferSize);
	}
}

void M2kOscController::setSweep(sweep::Sweep s)
{
	s = sweep::normalized(s, m_limits);
	if(s.sampleRate == m_sweep.sampleRate && s.bufferSize == m_sweep.bufferSize &&
	   s.plotWindow == m_sweep.plotWindow) {
		setDelay(s.delay);
		return;
	}
	restartIfRunning([this, s]() {
		m_sweep = s;
		const bool streaming = isStreaming();
		applyStreaming();
		m_horizontal->setDelayEnabled(!streaming);
		applyRate(s.sampleRate);
		m_measure->setSampleRate(s.sampleRate);
		m_ui->setBufferSize(s.bufferSize);
		for(Trace *t : std::as_const(m_traces)) {
			t->curve->reclaimDepth(s.plotWindow);
		}
		m_views[ViewXY].curves[0]->reclaimDepth(s.bufferSize);
		for(scopy::adc::AcqCurveChannel *c : std::as_const(m_views[ViewFFT].curves)) {
			c->reclaimDepth(s.bufferSize / 2 + 1);
		}
		setDelay(streaming ? 0 : s.delay);
		m_shownDelay = m_sweep.delay;
		applyXRange();
		applyGate();
	});
}

void M2kOscController::setDelay(int samples)
{
	m_sweep.delay = sweep::align4(samples, m_limits.delayMin, m_limits.delayMax);
	m_horizontal->setSweep(m_sweep);
	setHw(analogIn(), "trigger_delay", QString::number(isStreaming() ? 0 : m_sweep.delay), M2kSource::Restart);
	if(!m_ui->engine()->isRunning()) {
		m_shownDelay = m_sweep.delay;
		applyXRange();
	}
}

void M2kOscController::applyXRange()
{
	const double sr = m_sweep.sampleRate;
	const bool streaming = isStreaming();
	const double t0 = streaming ? 0.0 : m_shownDelay / sr;
	PlotAxis *x = m_traces[0]->curve->xAxis()->plotAxis();
	m_traces[0]->curve->xAxis()->setInterval(t0, t0 + m_sweep.plotWindow / sr);
	m_timeHandle->handle()->setVisible(m_trigger.enabled && !streaming);
	m_timeHandle->setPositionSilent(0.0);

	MetricPrefixFormatter fmt;
	fmt.setTrimZeroes(true);
	m_sweepInfo->setText(tr("%1 samples at %2, %3/div%4")
				     .arg(m_sweep.plotWindow)
				     .arg(fmt.format(sr, QStringLiteral("sps"), 2))
				     .arg(fmt.format(m_sweep.plotWindow / sr / x->divs(), QStringLiteral("s"), 2))
				     .arg(streaming ? tr(", streaming") : QString()));
	m_plot->replot();
}

void M2kOscController::rollXRange()
{
	const std::optional<QVector<float>> time = m_ui->store()->latestFloat(m_source->timeKey());
	if(!time || time->isEmpty()) {
		return;
	}
	const double span = m_sweep.plotWindow / m_sweep.sampleRate;
	const double start = std::max(0.0, double(time->last()) - span);
	m_traces[0]->curve->xAxis()->setInterval(start, start + span);
}

void M2kOscController::applyVertical(Trace *t)
{
	const auto [lo, hi] = yRange(t);
	t->curve->yAxis()->setInterval(lo, hi);
	if(t->kind == Kind::Channel) {
		m_hwTimer->start();
		placeLevelHandle();
		applyViews();
	}
	m_plot->replot();
}

void M2kOscController::applyHardware()
{
	for(int ch = 0; ch < CHANNELS; ++ch) {
		const Trace *t = m_traces[ch];
		const double p = probe(ch);
		const double reach = (std::abs(t->position) + t->voltsPerDiv * kDivsY / 2) / p;
		m_lowRange[ch] = reach <= kLowRangeSpan / 2;
		setHw(input(ch), "range", QLatin1String(m_lowRange[ch] ? PLUS_2_5V : PLUS_25V), M2kSource::Restart);
		setHw(input(ch), "vertical_offset", number(t->position / p), M2kSource::Restart);
	}
	applyTriggerLimits();
}

void M2kOscController::setVoltsPerDiv(int i, double v)
{
	if(Trace *t = trace(i)) {
		t->voltsPerDivSpin->setValue(v);
	}
}

double M2kOscController::voltsPerDiv(int i) const
{
	const Trace *t = trace(i);
	return t ? t->voltsPerDiv : 0.0;
}

void M2kOscController::setPosition(int i, double v)
{
	if(Trace *t = trace(i)) {
		t->positionSpin->setValue(v);
	}
}

double M2kOscController::position(int i) const
{
	const Trace *t = trace(i);
	return t ? t->position : 0.0;
}

void M2kOscController::setProbe(int ch, double attenuation)
{
	if(ch < 0 || ch >= CHANNELS || !(attenuation > 0)) {
		return;
	}
	m_probeUi[ch]->setValueSilent(attenuation);
	m_probeValue[ch] = attenuation;
	m_probeCfg[ch]->scale = static_cast<float>(attenuation);
	m_measure->clearStats();
	applyVertical(m_traces[ch]);
	applyTriggerUi();
}

double M2kOscController::probe(int ch) const { return m_probeValue[ch]; }

void M2kOscController::setAcCoupled(int ch, bool on)
{
	if(ch < 0 || ch >= CHANNELS) {
		return;
	}
	{
		QSignalBlocker block(m_couplingUi[ch]);
		m_couplingUi[ch]->setChecked(!on);
	}
	m_probeCfg[ch]->acCoupled = on;
	m_measure->clearStats();
	pushTrigger();
}

bool M2kOscController::isAcCoupled(int ch) const { return m_probeCfg[ch]->acCoupled.load(); }

void M2kOscController::setChannelEnabled(int i, bool on)
{
	if(Trace *t = trace(i)) {
		t->row->checkBox()->setChecked(on);
	}
}

bool M2kOscController::isChannelEnabled(int i) const
{
	const Trace *t = trace(i);
	return t && t->enabled;
}

void M2kOscController::setTrigger(const osc::Trigger &t)
{
	auto select = [](MenuCombo *c, const QVariant &data) {
		const int i = c->combo()->findData(data);
		if(i >= 0) {
			c->combo()->setCurrentIndex(i);
		}
	};
	const QSignalBlocker b0(m_trigUi.enable), b1(m_trigUi.mode), b3(m_trigUi.channel),
		b4(m_trigUi.condition->combo()), b5(m_trigUi.digital->combo()), b6(m_trigUi.externalCondition->combo()),
		b7(m_trigUi.logic->combo());
	m_trigUi.enable->setChecked(t.enabled);
	m_trigUi.mode->setChecked(t.autoMode);
	m_trigUi.channel->setChecked(t.channel == 0);
	m_trigUi.condition->combo()->setCurrentIndex(t.condition);
	m_trigUi.externalCondition->combo()->setCurrentIndex(t.externalCondition);
	select(m_trigUi.digital, t.digital);
	m_trigUi.logic->combo()->setCurrentIndex(t.logic);
	m_trigUi.level->setValueSilent(t.level);
	m_trigUi.hysteresis->setValueSilent(t.hysteresis);
	if(m_trigUi.out) {
		const QSignalBlocker b8(m_trigUi.out->combo());
		select(m_trigUi.out, t.outSelect);
	}
	// Unblocked: the header's toggle is what folds the section.
	m_trigUi.analog->setChecked(t.analog);
	applyTriggerUi();
}

void M2kOscController::applyTriggerUi()
{
	if(!m_trigUi.mode) {
		return;
	}
	osc::Trigger t;
	t.enabled = m_trigUi.enable->isChecked();
	t.autoMode = m_trigUi.mode->isChecked();
	t.analog = m_trigUi.analog->isChecked();
	t.channel = m_trigUi.channel->isChecked() ? 0 : 1;
	t.condition = m_trigUi.condition->combo()->currentIndex();
	t.level = m_trigUi.level->value();
	t.hysteresis = m_trigUi.hysteresis->value();
	t.digital = m_trigUi.digital->combo()->currentData().toInt();
	t.externalCondition = m_trigUi.externalCondition->combo()->currentIndex();
	t.logic = m_trigUi.logic->combo()->currentIndex();
	t.outSelect = m_trigUi.out ? m_trigUi.out->combo()->currentData().toInt() : 0;
	m_trigger = t;

	m_trigUi.externalCondition->setVisible(t.digital == osc::DigitalExternal);
	m_trigUi.logic->setVisible(t.analog && t.digital == osc::DigitalExternal);
	applyTriggerLimits();
	pushTrigger();
	placeLevelHandle();
	applyXRange();
	if(!m_ui->engine()->isRunning()) {
		setTriggerStatus(tr("Stop"));
	}
}

void M2kOscController::applyTriggerLimits()
{
	if(!m_trigUi.level) {
		return;
	}
	const int ch = m_trigger.channel;
	const double p = probe(ch);
	const double span = m_lowRange[ch] ? kLowRangeSpan / 2 : kHighRangeSpan / 2;
	const double offset = m_traces[ch]->position;
	m_trigUi.level->setMinValue((-span) * p - offset);
	m_trigUi.level->setMaxValue(span * p - offset);
	m_trigUi.hysteresis->setMaxValue(span * p);
}

void M2kOscController::pushTrigger()
{
	const osc::Trigger &t = m_trigger;
	int mode[CHANNELS] = {kModeAlways, kModeAlways};
	int source = kSourceCh1;
	if(t.enabled) {
		if(t.analog) {
			source = kSourceCh1 + t.channel;
			mode[t.channel] = kModeAnalog;
			if(t.digital == osc::DigitalExternal) {
				mode[t.channel] = kModeDigitalOrAnalog + t.logic;
			} else if(t.digital == osc::DigitalLogic) {
				source = kSourceChOrDigital + t.channel;
			}
		} else if(t.digital == osc::DigitalExternal) {
			mode[0] = kModeExternal;
		} else if(t.digital == osc::DigitalLogic) {
			source = kSourceDigital;
		}
	}
	// The level is shown after the probe and the AC coupling; the hardware compares the input.
	const double p = probe(t.channel);
	const double level = t.level / p + (isAcCoupled(t.channel) ? m_trigDc : 0.0);
	const auto always = trigger::toName(trigger::MODE_OPTIONS, kModeAlways);
	const auto ch1 = trigger::toName(trigger::ANALOG_SOURCE_OPTIONS, kSourceCh1);
	component::Device *dev = analogIn();
	setHw(dev, "trigger_source", trigger::toName(trigger::ANALOG_SOURCE_OPTIONS, source), M2kSource::Restart);
	QList<M2kSource::Setting> untriggered{{dev, "trigger_source", ch1}};
	for(int ch = 0; ch < CHANNELS; ++ch) {
		component::Channel *in = input(ch);
		setHw(in, "trigger_mode", trigger::toName(trigger::MODE_OPTIONS, mode[ch]), M2kSource::Restart);
		setHw(in, "trigger_level", number(level), M2kSource::Restart);
		setHw(in, "trigger_hysteresis", number(t.hysteresis / p), M2kSource::Restart);
		setHw(in, "trigger_condition", trigger::toName(trigger::ANALOG_CONDITION_OPTIONS, t.condition),
		      M2kSource::Restart);
		setHw(in, "trigger_external_condition",
		      trigger::toName(trigger::DIGITAL_CONDITION_OPTIONS, t.externalCondition), M2kSource::Restart);
		untriggered.append({in, "trigger_mode", always});
	}
	if(m_hasExternalOut) {
		setHw(dev, "trigger_external_out_select", trigger::toName(trigger::OUT_SELECT_OPTIONS, t.outSelect),
		      M2kSource::Restart);
	}
	m_source->setAutoTrigger(M2kSource::Analog, t.enabled && t.autoMode ? kAutoTimeoutMs : 0, untriggered);
}

void M2kOscController::setHw(QObject *node, const char *attr, const QString &value, M2kSource::Apply apply)
{
	if(m_source->value(node, attr) != value) {
		m_source->set(node, attr, value, apply);
	}
}

void M2kOscController::applyRate(double hz)
{
	const osc::HwRate hw = osc::hwRate(hz, m_hwRates);
	setHw(analogIn(), "sample_rate", number(hw.rate), M2kSource::Reopen);
	setHw(analogIn(), "oversampling_ratio", QString::number(hw.oversampling), M2kSource::Reopen);
}

void M2kOscController::applyStreaming()
{
	const bool streaming = isStreaming();
	setHw(analogIn(), "trigger_streaming_flag", streaming ? QStringLiteral("1") : QStringLiteral("0"),
	      M2kSource::Reopen);
	m_source->setKernelBuffers(M2kSource::Analog, streaming ? kStreamingKernelBuffers : 1);
}

void M2kOscController::trackTriggerDc()
{
	const int ch = m_trigger.channel;
	if(!m_trigger.enabled || !m_trigger.analog || !isAcCoupled(ch)) {
		return;
	}
	const std::optional<QVector<float>> raw = m_ui->store()->latestFloat(m_source->rawKey(M2kSource::Analog, ch));
	if(!raw || raw->isEmpty()) {
		return;
	}
	const double dc = std::accumulate(raw->cbegin(), raw->cend(), 0.0) / raw->size();
	if(std::abs(dc - m_trigDc) > std::max(1e-3, m_trigger.hysteresis / probe(ch) / 4)) {
		m_trigDc = dc;
		pushTrigger();
	}
}

void M2kOscController::placeLevelHandle()
{
	if(!m_levelHandle || m_traces.size() < CHANNELS) {
		return;
	}
	m_levelHandle->handle()->setVisible(m_trigger.enabled && m_trigger.analog);
	scopy::adc::AcqCurveChannel *c = m_traces[m_trigger.channel]->curve;
	m_levelHandle->setAxis(c->yAxis()->plotAxis());
	m_levelHandle->handle()->setColor(c->color());
	m_levelHandle->setPositionSilent(m_trigger.level);
}

void M2kOscController::setupMeasurements()
{
	InstrumentTemplate *it = m_ui->shell();
	m_measureArea = new QScrollArea(it);
	m_measureArea->setWidgetResizable(true);
	m_measureArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_measureArea->setFrameShape(QFrame::NoFrame);
	m_measureArea->setVisible(false);
	auto *box = new QWidget(m_measureArea);
	Style::setBackgroundColor(box, json::theme::background_subtle);
	auto *lay = new QVBoxLayout(box);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(6);
	auto *header = new QHBoxLayout();
	QPushButton *reset = button(tr("Reset statistics"), box);
	QPushButton *clear = button(tr("Clear all"), box);
	for(QPushButton *b : {reset, clear}) {
		Style::setStyle(b, style::properties::button::transparentButton);
	}
	header->addStretch();
	header->addWidget(reset);
	header->addWidget(clear);
	lay->addLayout(header);
	for(QWidget **flow : {&m_measureFlow, &m_statsFlow}) {
		*flow = new QWidget(box);
		new FlowLayout(*flow);
		lay->addWidget(*flow);
	}
	m_measureArea->setWidget(box);
	m_measureArea->viewport()->installEventFilter(this);
	it->addToSlot(PS_BOTTOM, m_measureArea);
	connect(reset, &QPushButton::clicked, m_measure, &scopy::acq::MeasureProcessor::clearStats);
	connect(clear, &QPushButton::clicked, this, [this]() {
		for(Trace *t : std::as_const(m_traces)) {
			if(t->selector) {
				t->selector->toggleAllMeasurement(false);
				t->selector->toggleAllStats(false);
			}
		}
	});
	m_marks = new scopy::acq::MeasureMarksItem();
	m_marks->attach(m_plot->plot()->plot());
}

void M2kOscController::fitMeasureArea()
{
	m_measureArea->setVisible(!m_labels.isEmpty() || !m_statLabels.isEmpty());
	m_statsFlow->setVisible(!m_statLabels.isEmpty());
	m_measureFlow->setVisible(!m_labels.isEmpty());
	const int content = m_measureArea->widget()->heightForWidth(m_measureArea->viewport()->width());
	m_measureArea->setFixedHeight(std::min(content, kMeasureMaxPx));
}

bool M2kOscController::eventFilter(QObject *watched, QEvent *event)
{
	if(m_measureArea && watched == m_measureArea->viewport() && event->type() == QEvent::Resize) {
		QTimer::singleShot(0, this, &M2kOscController::fitMeasureArea);
	}
	return M2kInstrumentController::eventFilter(watched, event);
}

QWidget *M2kOscController::buildMeasureSection(Trace *t)
{
	MenuSectionCollapseWidget *section = newSection(tr("MEASUREMENTS"));
	t->selector = new MeasurementSelector(section);
	for(int i = 0; i < scopy::acq::MEASURE_COUNT; ++i) {
		const Measure m = static_cast<Measure>(i);
		const QString name = scopy::acq::measureName(m);
		t->selector->addMeasurement(name,
					    QStringLiteral(":/gui/icons/measurements/%1.svg").arg(kMeasureIcons[i]));
		MeasurementSelectorItem *item = t->selector->measurement(name);
		connect(item->measureCheckbox(), &QAbstractButton::toggled, this,
			[this, t, m](bool on) { setMeasured(m_traces.indexOf(t), m, on); });
		connect(item->statsCheckbox(), &QAbstractButton::toggled, this,
			[this, t, m](bool on) { setStatsShown(m_traces.indexOf(t), m, on); });
	}
	auto *scroll = new QScrollArea(section);
	scroll->setWidgetResizable(true);
	scroll->setFrameShape(QFrame::NoFrame);
	scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scroll->setWidget(t->selector);
	scroll->setFixedHeight(kMeasureListPx);
	section->add(scroll);
	section->setCollapsed(true);
	return section;
}

void M2kOscController::setMeasured(int i, Measure m, bool on)
{
	Trace *t = trace(i);
	if(!t || !t->selector) {
		return;
	}
	const int id = labelId(t->uid, m);
	if(on == m_labels.contains(id)) {
		return;
	}
	MeasurementSelectorItem *item = t->selector->measurement(scopy::acq::measureName(m));
	{
		QSignalBlocker block(item->measureCheckbox());
		item->measureCheckbox()->setChecked(on);
	}
	if(on) {
		auto *label = new MeasurementLabel(m_measureFlow);
		label->setFixedWidth(QFontMetrics(label->font()).horizontalAdvance(kMeasureTemplate));
		label->setName(scopy::acq::measureName(m));
		label->setUnit(scopy::acq::measureUnit(m));
		label->setColor(t->curve->color());
		label->setIdx(t->uid);
		label->setMeasurementValueFormatter(new MetricPrefixFormatter(label));
		label->setValue(t->lastResult.value(m));
		m_labels.insert(id, label);
		m_measureFlow->layout()->addWidget(label);
	} else {
		MeasurementLabel *label = m_labels.take(id);

		delete label;
	}
	fitMeasureArea();
	syncMeasureKeys();
	drawMarks();
}

bool M2kOscController::isMeasured(int i, Measure m) const
{
	const Trace *t = trace(i);
	return t && m_labels.contains(labelId(t->uid, m));
}

void M2kOscController::setStatsShown(int i, Measure m, bool on)
{
	Trace *t = trace(i);
	if(!t || !t->selector) {
		return;
	}
	const int id = labelId(t->uid, m);
	if(on == m_statLabels.contains(id)) {
		return;
	}
	MeasurementSelectorItem *item = t->selector->measurement(scopy::acq::measureName(m));
	{
		QSignalBlocker block(item->statsCheckbox());
		item->statsCheckbox()->setChecked(on);
	}
	if(on) {
		auto *label = new StatsLabel(m_statsFlow);
		label->setFixedWidth(QFontMetrics(label->font()).horizontalAdvance(kStatsTemplate));
		label->setName(QStringLiteral("%1 %2").arg(t->name, scopy::acq::measureName(m)));
		label->setUnit(scopy::acq::measureUnit(m));
		label->setColor(t->curve->color());
		label->setIdx(t->uid);
		label->setMeasurementValueFormatter(new MetricPrefixFormatter(label));
		m_statLabels.insert(id, label);
		m_statsFlow->layout()->addWidget(label);
	} else {
		StatsLabel *label = m_statLabels.take(id);

		delete label;
	}
	fitMeasureArea();
	syncMeasureKeys();
}

bool M2kOscController::isStatsShown(int i, Measure m) const
{
	const Trace *t = trace(i);
	return t && m_statLabels.contains(labelId(t->uid, m));
}

void M2kOscController::syncMeasureKeys()
{
	// Only keys written every cycle: a watched key that never arrives stalls the processor.
	auto live = [this](const Trace *t) {
		if(!t->enabled || t->kind == Kind::Reference) {
			return false;
		}
		return !t->math || (m_traces[0]->enabled && m_traces[1]->enabled);
	};
	QList<DataKey> keys;
	for(const Trace *t : std::as_const(m_traces)) {
		if(!live(t)) {
			continue;
		}
		for(int i = 0; i < scopy::acq::MEASURE_COUNT; ++i) {
			const int id = labelId(t->uid, static_cast<Measure>(i));
			if(m_labels.contains(id) || m_statLabels.contains(id)) {
				keys.append(t->key);
				break;
			}
		}
	}
	m_measure->setWatchedKeys(keys);
}

double M2kOscController::measurement(int i, Measure m) const
{
	const Trace *t = trace(i);
	return t ? t->lastResult.value(m) : std::numeric_limits<double>::quiet_NaN();
}

QVariantMap M2kOscController::statistics(int i, Measure m) const
{
	const Trace *t = trace(i);
	if(!t) {
		return {};
	}
	const scopy::acq::MeasureStat &s = t->lastStats[static_cast<int>(m)];
	return {{"average", s.average}, {"min", s.min}, {"max", s.max}, {"count", s.count}};
}

void M2kOscController::onMeasured(const scopy::acq::MeasureSnapshot &snap)
{
	auto it = std::find_if(m_traces.cbegin(), m_traces.cend(),
			       [&snap](const Trace *t) { return t->key == snap.key; });
	if(it == m_traces.cend()) {
		return;
	}
	Trace *t = *it;
	t->lastResult = snap.result;
	t->lastStats = snap.stats;
	for(int i = 0; i < scopy::acq::MEASURE_COUNT; ++i) {
		const int id = labelId(t->uid, static_cast<Measure>(i));
		if(MeasurementLabel *label = m_labels.value(id)) {
			label->setValue(snap.result.values[i]);
		}
		if(StatsLabel *label = m_statLabels.value(id)) {
			const scopy::acq::MeasureStat &s = snap.stats[i];
			label->setValue(s.count ? s.average : std::numeric_limits<double>::quiet_NaN(), s.min, s.max);
		}
	}
	if(t == m_marksTrace) {
		drawMarks();
	}
}

PlotCursors *M2kOscController::plotCursors() const { return m_plot->cursors()->getPlotCursors(); }

void M2kOscController::setGated(bool on)
{
	m_gated = on;
	if(m_gateSwitch) {
		QSignalBlocker block(m_gateSwitch->onOffswitch());
		m_gateSwitch->onOffswitch()->setChecked(on);
	}
	applyGate();
}

void M2kOscController::applyGate()
{
	if(!m_gated || !m_cursorXEn || !m_cursorXEn->isChecked() || isStreaming()) {
		m_measure->setGate(0, -1);
		return;
	}
	PlotCursors *cursors = m_plot->cursors()->getPlotCursors();
	const double sr = m_sweep.sampleRate;
	auto index = [this, sr](double t) { return static_cast<int>(std::lround(t * sr)) - m_shownDelay; };
	const int a = index(cursors->getX1Cursor()->getPosition());
	const int b = index(cursors->getX2Cursor()->getPosition());
	m_measure->setGate(std::max(0, std::min(a, b)), std::max(0, std::max(a, b)));
}

void M2kOscController::drawMarks()
{
	const Trace *t = m_marksTrace;
	if(!t) {
		return;
	}
	const int i = m_traces.indexOf(t);
	const scopy::acq::MeasureMarks &mk = t->lastResult.marks;
	const double sr = m_sweep.sampleRate;
	QVector<double> times;
	auto add = [&](int idx) {
		if(idx >= 0) {
			times.append((m_shownDelay + idx) / sr);
		}
	};
	if(isMeasured(i, Measure::Period) || isMeasured(i, Measure::Frequency)) {
		add(mk.periodStart);
		add(mk.periodEnd);
	}
	if(isMeasured(i, Measure::Rise)) {
		add(mk.lowRising);
		add(mk.highRising);
	}
	if(isMeasured(i, Measure::Fall)) {
		add(mk.highFalling);
		add(mk.lowFalling);
	}
	for(Measure m : {Measure::PosWidth, Measure::NegWidth, Measure::PosDuty, Measure::NegDuty}) {
		if(isMeasured(i, m)) {
			add(mk.midRising);
			add(mk.midFalling);
			break;
		}
	}
	QVector<double> levels;
	for(Measure m : {Measure::Low, Measure::High, Measure::Middle}) {
		const double v = t->lastResult.value(m);
		if(isMeasured(i, m) && !std::isnan(v)) {
			levels.append(v);
		}
	}
	m_marks->setAxes(t->curve->xAxis()->plotAxis()->axisId(), t->curve->yAxis()->plotAxis()->axisId());
	m_marks->setColor(t->curve->color());
	m_marks->setVisible(t->enabled);
	m_marks->setMarks(times, levels);
	m_dirty = true;
}

void M2kOscController::autoset()
{
	if(isAutosetting() ||
	   std::none_of(m_traces.cbegin(), m_traces.cbegin() + CHANNELS, [](const Trace *t) { return t->enabled; })) {
		Q_EMIT autosetFinished();
		return;
	}
	m_autosetResume = m_ui->engine()->isRunning();
	if(m_autosetResume) {
		m_ui->stop();
	}
	osc::Trigger t = m_trigger;
	t.enabled = false;
	setTrigger(t);
	// The high range, centered, sees anything the inputs take.
	for(int ch = 0; ch < CHANNELS; ++ch) {
		setVoltsPerDiv(ch, 5.0 * probe(ch));
		setPosition(ch, 0.0);
	}
	m_autosetStep = 0;
	m_autosetConn = connect(m_ui, &scopy::adc::AcqInstrument::stopped, this, &M2kOscController::onAutosetCapture,
				Qt::QueuedConnection);
	runAutosetStep();
}

void M2kOscController::runAutosetStep()
{
	sweep::Sweep s = m_sweep;
	s.sampleRate = osc::AUTOSET_RATES.at(m_autosetStep);
	s.bufferSize = kTimeBaseSamples;
	s.delay = 0;
	setSweep(s);
	m_hwTimer->stop();
	applyHardware();
	single();
	if(!m_ui->engine()->isRunning()) {
		finishAutoset();
	}
}

void M2kOscController::onAutosetCapture()
{
	const double sr = m_sweep.sampleRate;
	scopy::acq::MeasureParams p;
	p.sampleRate = sr;
	int best = -1;
	scopy::acq::MeasureResult results[CHANNELS];
	for(int ch = 0; ch < CHANNELS; ++ch) {
		const std::optional<QVector<float>> d = m_ui->store()->latestFloat(m_traces[ch]->key);
		if(!m_traces[ch]->enabled || !d || d->isEmpty()) {
			continue;
		}
		results[ch] = scopy::acq::measure(d->constData(), d->size(), p);
		if(best < 0 || results[ch].value(Measure::PeakPeak) > results[best].value(Measure::PeakPeak)) {
			best = ch;
		}
	}
	const double period = best >= 0 ? results[best].value(Measure::Period) : std::nan("");
	const bool fewCycles = std::isnan(period) || m_sweep.bufferSize / sr < 3 * period;
	if(best >= 0 && fewCycles && m_autosetStep + 1 < osc::AUTOSET_RATES.size()) {
		++m_autosetStep;
		runAutosetStep();
		return;
	}
	for(int ch = 0; ch < CHANNELS && best >= 0; ++ch) {
		if(std::isnan(results[ch].value(Measure::PeakPeak))) {
			continue;
		}
		setVoltsPerDiv(ch, osc::autosetVoltsPerDiv(results[ch].value(Measure::PeakPeak)));
		setPosition(ch, -results[ch].value(Measure::Middle));
	}
	const int divs = qRound(m_traces[0]->curve->xAxis()->plotAxis()->divs());
	setSweep(
		sweep::fromTimeBase(osc::autosetTimeBase(std::isnan(period) ? -1 : period, divs), 0.0, divs, m_limits));
	osc::Trigger t = m_trigger;
	t.enabled = true;
	t.autoMode = true;
	if(best >= 0) {
		t.analog = true;
		t.channel = best;
		t.condition = 0;
		t.level = results[best].value(Measure::Middle);
	}
	setTrigger(t);
	finishAutoset();
	if(m_autosetResume) {
		m_ui->run();
	}
}

void M2kOscController::finishAutoset()
{
	disconnect(m_autosetConn);
	m_autosetStep = -1;
	Q_EMIT autosetFinished();
}

void M2kOscController::setTriggerStatus(const QString &status)
{
	m_trigStatus->setText(status);
	m_plotStatus->setText(status);
}

void M2kOscController::exportFile()
{
	const bool native = Preferences::get("general_use_native_dialogs").toBool();
	QString filter;
	QString path = QFileDialog::getSaveFileName(
		m_ui, tr("Export"), QString(), tr("Comma-separated values (*.csv);;Tab-delimited text (*.txt)"),
		&filter, native ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog);
	if(path.isEmpty()) {
		return;
	}
	const QString ext = filter.contains(QStringLiteral("*.txt")) ? QStringLiteral(".txt") : QStringLiteral(".csv");
	if(!path.endsWith(ext, Qt::CaseInsensitive)) {
		path += ext;
	}
	if(!exportData(path)) {
		StatusBarManager::pushMessage(tr("Oscilloscope: nothing to export yet"));
	}
}

bool M2kOscController::exportData(const QString &path) const
{
	const std::optional<QVector<float>> time = m_ui->store()->latestFloat(m_source->timeKey());
	if(!time) {
		return false;
	}
	QStringList header{QStringLiteral("Time (s)")};
	QList<QVector<float>> columns;
	for(const Trace *t : std::as_const(m_traces)) {
		std::optional<QVector<float>> d = m_ui->store()->latestFloat(t->key);
		if(t->enabled && d && !d->isEmpty()) {
			header.append(t->name + QStringLiteral(" (V)"));
			columns.append(std::move(*d));
		}
	}
	QFile file(path);
	if(columns.isEmpty() || !file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		return false;
	}
	const QString sep =
		path.endsWith(QStringLiteral(".txt"), Qt::CaseInsensitive) ? QStringLiteral("\t") : QStringLiteral(",");
	QTextStream out(&file);
	out << header.join(sep) << "\n";
	for(int r = 0; r < time->size(); ++r) {
		QStringList cells{QString::number((*time)[r], 'g', 9)};
		for(const QVector<float> &v : std::as_const(columns)) {
			cells.append(r < v.size() ? QString::number(v[r], 'g', 7) : QString());
		}
		out << cells.join(sep) << "\n";
	}
	return true;
}

#include "moc_m2kosccontroller.cpp"
