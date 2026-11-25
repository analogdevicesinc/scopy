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

#include "rawgeneratorview.h"

#include "nodepanel.h"
#include "readpolicy.h"
#include "sidesplitter.h"
#include "streamutils.h"

#include <QComboBox>
#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include <limits>

#include <component/attribute.h>
#include <component/channel.h>
#include <component/device.h>
#include <component/outputstream.h>
#include <dcdebugger/style_properties.h>
#include <hoverwidget.h>
#include <iiowidget.h>
#include <iiowidgetbuilder.h>
#include <menucombo.h>
#include <menuonoffswitch.h>
#include <menusectionwidget.h>
#include <menuspinbox.h>
#include <mousewheelwidgetguard.h>
#include <plotaxis.h>
#include <plotchannel.h>
#include <plotlegend.h>
#include <plotnavigator.hpp>
#include <plotwidget.h>
#include <pluginbase/statusbarmanager.h>
#include <style.h>
#include <stylehelper.h>
#include <toolbuttons.h>
#include <tooltemplate.h>

using namespace scopy;
using namespace scopy::dcdebugger;
using namespace scopy::component;

namespace {
const char *const SETTINGS_KEY = "settings";
// Codes reach 2^31 on 32-bit DACs; the code range is clamped when the buffer is written.
constexpr double MaxCode = 2147483647.0;

gui::MenuSpinbox *makeSpin(const QString &name, double value, const QString &unit, double min, double max, double step,
			   QWidget *parent)
{
	auto *sb = new gui::MenuSpinbox(name, value, unit, min, max, true, false, false, parent);
	sb->setIncrementMode(gui::MenuSpinbox::IS_FIXED);
	if(auto *fixed = dynamic_cast<gui::IncrementStrategyFixed *>(sb->incrementStrategy())) {
		fixed->setK(step);
	}
	sb->setScalingEnabled(false);
	// Hidden, the unit prefix still scales the step: unit only.
	sb->setScaleList(QList<gui::UnitPrefix>{{QString(), 1.0}});
	sb->setScaleRange(1, 1); // setScaleList keeps the scales inside [min, max]
	return sb;
}
} // namespace

RawGeneratorView::RawGeneratorView(OutputStream *stream, QWidget *parent)
	: QWidget(parent)
	, m_stream(stream)
	, m_gen(new StreamGeneration(stream, this))
	, m_applyTimer(new QTimer(this))
{
	m_sampleRateAttrs = memberSampleRates(stream, true);
	m_gen->setRawMode(true);
	// Raw defaults: DefaultPeriods periods of DefaultAmplitude codes; the engine's 0 / 90 degree
	// phases (an I/Q pair on quadrature DACs) are kept.
	QHash<QString, ToneSpec> tones = m_gen->tones();
	for(ToneSpec &t : tones) {
		t.enabled = true;
		t.frequency = DefaultPeriods;
		t.amplitude = DefaultAmplitude;
		t.offset = 0;
	}
	m_gen->setTones(tones);

	m_applyTimer->setSingleShot(true);
	m_applyTimer->setInterval(150);
	connect(m_applyTimer, &QTimer::timeout, this, &RawGeneratorView::apply);

	auto *lay = new QVBoxLayout(this);
	lay->setContentsMargins(0, 0, 0, 0);
	// ToolTemplate starts with every container hidden.
	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->centralContainer()->setVisible(true);
	m_tool->topContainerMenuControl()->setVisible(false);
	lay->addWidget(m_tool);
	// Preview | Settings menu (everything is in the menu: no channel list).
	m_sides = new SideSplitter(0, 320, QStringLiteral("rawgenerator/sides"), this);
	m_tool->addWidgetToCentralContainerHelper(m_sides);

	m_status = new QLabel("Stopped", this);
	Style::setStyle(m_status, style::properties::label::subtle);
	m_runBtn = new RunBtn(this);
	m_runBtn->setText("Start");
	disconnect(m_runBtn, &QPushButton::toggled, m_runBtn, nullptr); // RunBtn's own Run / Stop text
	connect(m_runBtn, &QPushButton::toggled, m_runBtn,
		[this](bool on) { m_runBtn->setText(on ? "Stop" : "Start"); });
	auto *gearBtn = new GearBtn(this);
	gearBtn->setChecked(m_sides->isRightOpen());
	m_tool->addWidgetToTopContainerHelper(m_status, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);
	m_tool->addWidgetToTopContainerHelper(gearBtn, TTA_RIGHT);
	connect(gearBtn, &QPushButton::toggled, m_sides, &SideSplitter::setRightOpen);
	connect(m_sides, &SideSplitter::rightOpenChanged, gearBtn, &QAbstractButton::setChecked);

	buildPreview();
	m_sides->rightStack()->add(SETTINGS_KEY, buildSettings(this));
	m_sides->rightStack()->show(SETTINGS_KEY);

	connect(m_runBtn, &QPushButton::toggled, this, [this](bool on) {
		if(on) {
			apply();
			m_gen->start();
		} else {
			m_gen->stop();
		}
	});
	connect(m_gen, &StreamGeneration::runningChanged, this, &RawGeneratorView::onRunningChanged);
	connect(m_gen, &StreamGeneration::pushed, this, [this](int channels, quint64 samples, bool cyclic) {
		setStatus(QStringLiteral("Running: %1 channel(s), %2 samples, %3")
				  .arg(channels)
				  .arg(samples)
				  .arg(cyclic ? QStringLiteral("cyclic")
					      : QStringLiteral("streaming (push %1)").arg(m_gen->pushes())));
	});
	connect(m_gen, &StreamGeneration::errorOccurred, this, [this](const QString &msg) {
		setStatus(msg, true);
		StatusBarManager::pushMessage("DC Debugger: " + msg, 5000);
	});
	if(m_gen->channels().isEmpty()) {
		setStatus("This stream exposes no streamable channels.", true);
		m_runBtn->setEnabled(false);
	}

	apply();
}

RawGeneratorView::~RawGeneratorView() = default;

QWidget *RawGeneratorView::buildSettings(QWidget *parent)
{
	auto *scroll = new QScrollArea(parent);
	scroll->setWidgetResizable(true);
	scroll->setFrameShape(QFrame::NoFrame);
	scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scroll->setObjectName("rawGeneratorSettingsMenu");
	auto *content = new QWidget(scroll);
	auto *outer = new QVBoxLayout(content);
	outer->setContentsMargins(0, 0, 0, 0);
	outer->setSpacing(10);
	outer->addWidget(buildOutputSection(content));

	const QList<Channel *> elems = m_gen->channels();
	const QHash<QString, ToneSpec> tones = m_gen->tones();
	for(int i = 0; i < elems.size(); ++i) {
		ChannelControls ctrl;
		ctrl.id = elems[i]->id();
		outer->addWidget(buildChannelSection(ctrl, i, tones.value(ctrl.id), content));
		m_channels.append(ctrl);
	}
	outer->addStretch();

	// Scrolling the menu must not change the values under the cursor.
	auto *guard = new MouseWheelWidgetGuard(content);
	guard->installEventRecursively(content);
	scroll->setWidget(content);
	return scroll;
}

QWidget *RawGeneratorView::buildOutputSection(QWidget *parent)
{
	MenuSectionCollapseWidget *section = makeSection("Output", parent);
	QVBoxLayout *lay = section->contentLayout();
	lay->setSpacing(8);
	if(auto *dev = qobject_cast<Device *>(m_stream->parent())) {
		auto *devLabel = new QLabel(
			"Device: " + (dev->label().isEmpty() ? dev->name() : dev->name() + " (" + dev->label() + ")"),
			section);
		devLabel->setWordWrap(true);
		Style::setStyle(devLabel, style::properties::label::subtle);
		lay->addWidget(devLabel);
	}

	m_samples = new gui::MenuSpinbox("Buffer samples", double(m_gen->samples()), "", 2, 4 * 1024 * 1024, true,
					 false, false, section);
	m_samples->setObjectName("rawGeneratorSamples");
	m_samples->setIncrementMode(gui::MenuSpinbox::IS_POW2);
	m_samples->setScalingEnabled(false);
	connect(m_samples, &gui::MenuSpinbox::valueChanged, this, &RawGeneratorView::scheduleApply);
	lay->addWidget(m_samples);

	// One control for the stream: shows the first member's rate, a write is applied to every member.
	if(Attribute *first = m_sampleRateAttrs.value(0)) {
		IIOWidget *rate = IIOWidgetBuilder(section).attribute(first).title("Sample rate").buildSingle();
		if(rate) {
			rate->setObjectName("rawGeneratorSampleRate");
			lay->addWidget(rate);
			if(!ReadPolicy::suppressed()) { // e.g. built by a "go to origin" from the log
				rate->readAsync();
			}
		}
		linkSampleRates(this, m_sampleRateAttrs, [this](const QString &msg) { setStatus(msg, true); });
		connect(first, &Attribute::valueChanged, this, &RawGeneratorView::updateFrequencies);
	}

	m_cyclic = new MenuOnOffSwitch("Cyclic", section);
	m_cyclic->setObjectName("rawGeneratorCyclic");
	m_cyclic->onOffswitch()->setChecked(m_gen->cyclic());
	m_cyclic->setEnabled(m_gen->supportsCyclic());
	m_cyclic->setToolTip(m_gen->supportsCyclic()
				     ? "On: one buffer is pushed and replayed by the device. Off: the buffer is "
				       "pushed continuously."
				     : "This stream cannot replay a buffer: it is pushed continuously.");
	connect(m_cyclic->onOffswitch(), &QAbstractButton::toggled, this, &RawGeneratorView::scheduleApply);
	lay->addWidget(m_cyclic);
	return section;
}

QWidget *RawGeneratorView::buildChannelSection(ChannelControls &ctrl, int index, const ToneSpec &tone, QWidget *parent)
{
	MenuSectionCollapseWidget *section = makeSection(streamChannelName(m_stream, ctrl.id, true), parent);
	section->setObjectName("rawGeneratorChannel:" + ctrl.id);
	QVBoxLayout *lay = section->contentLayout();
	lay->setSpacing(8);

	// The channel's curve color, so a section matches its curve in the preview and the legend.
	auto *swatch = new QWidget(section);
	swatch->setFixedHeight(3);
	swatch->setStyleSheet(QStringLiteral("background-color: %1;").arg(StyleHelper::getChannelColor(index)));
	lay->addWidget(swatch);

	ctrl.waveform = new MenuCombo("Waveform", section);
	ctrl.waveform->combo()->setObjectName("rawGeneratorWaveform:" + ctrl.id);
	ctrl.waveform->combo()->addItems(waveformNames());
	ctrl.waveform->combo()->setCurrentIndex(int(tone.waveform));
	const int maxP = maxPeriods(int(m_gen->samples()));
	ctrl.periods = makeSpin("Periods", std::min<double>(tone.frequency, maxP), "", 0, maxP, 1, section);
	ctrl.periods->setObjectName("rawGeneratorPeriods:" + ctrl.id);
	ctrl.periods->setToolTip("Whole periods in one buffer (0: a constant level)");
	ctrl.amplitude = makeSpin("Amplitude", tone.amplitude, "codes", 0, MaxCode, 64, section);
	ctrl.amplitude->setObjectName("rawGeneratorAmplitude:" + ctrl.id);
	ctrl.offset = makeSpin("Offset", tone.offset, "codes", -MaxCode, MaxCode, 64, section);
	ctrl.offset->setObjectName("rawGeneratorOffset:" + ctrl.id);
	ctrl.phase = makeSpin("Phase", tone.phase, "°", -360, 360, 15, section);
	ctrl.phase->setObjectName("rawGeneratorPhase:" + ctrl.id);
	ctrl.frequency = new QLabel(section);
	ctrl.frequency->setObjectName("rawGeneratorFrequency:" + ctrl.id);
	Style::setStyle(ctrl.frequency, style::properties::label::subtle);

	lay->addWidget(ctrl.waveform);
	lay->addWidget(ctrl.periods);
	lay->addWidget(ctrl.amplitude);
	lay->addWidget(ctrl.offset);
	lay->addWidget(ctrl.phase);
	lay->addWidget(ctrl.frequency);

	connect(ctrl.waveform->combo(), qOverload<int>(&QComboBox::currentIndexChanged), this,
		&RawGeneratorView::scheduleApply);
	for(gui::MenuSpinbox *sb : {ctrl.periods, ctrl.amplitude, ctrl.offset, ctrl.phase}) {
		connect(sb, &gui::MenuSpinbox::valueChanged, this, &RawGeneratorView::scheduleApply);
	}
	return section;
}

void RawGeneratorView::buildPreview()
{
	m_plot = new PlotWidget(this);
	m_plot->xAxis()->scaleDraw()->setFloatPrecision(0);
	m_plot->yAxis()->scaleDraw()->setFloatPrecision(0);
	m_plot->yAxis()->setInterval(-1, 1);
	m_plot->setShowXAxisLabels(true);
	m_plot->setShowYAxisLabels(true);
	m_plot->showAxisLabels();
	m_plot->navigator()->setBounded(false);

	const QList<Channel *> elems = m_gen->channels();
	for(int c = 0; c < elems.size(); ++c) {
		QPen pen(QColor(StyleHelper::getChannelColor(c)), 1);
		auto *ch = new PlotChannel(streamChannelName(m_stream, elems[c]->id(), true), pen, m_plot->xAxis(),
					   m_plot->yAxis(), m_plot);
		m_plot->addPlotChannel(ch);
		ch->enable();
		m_plotChannels.insert(elems[c]->id(), ch);
	}
	auto *legend = new PlotLegend(m_plot, m_plot);
	placeLegend();
	legend->setVisible(!elems.isEmpty());
	m_sides->setCentralWidget(m_plot);
}

void RawGeneratorView::placeLegend()
{
	// PlotLegend keeps its overlay private: it is the HoverWidget it added to the plot. Top right,
	// half transparent so the curves under it stay visible.
	for(HoverWidget *hover : m_plot->findChildren<HoverWidget *>(QString(), Qt::FindDirectChildrenOnly)) {
		if(!hover->findChild<QLabel *>()) {
			continue; // not the legend (its panel holds the channel names)
		}
		hover->setAnchorPos(HoverPosition::HP_TOPRIGHT);
		hover->setContentPos(HoverPosition::HP_BOTTOMLEFT);
		hover->setAnchorOffset(QPoint(-10, 10));
		auto *opacity = new QGraphicsOpacityEffect(hover);
		opacity->setOpacity(0.5);
		hover->setGraphicsEffect(opacity);
	}
}

double RawGeneratorView::sampleRate() const { return sampleRateOf(m_sampleRateAttrs.value(0)); }

void RawGeneratorView::scheduleApply() { m_applyTimer->start(); }

void RawGeneratorView::apply()
{
	m_applyTimer->stop();
	const int samples = int(std::max(2.0, m_samples->value()));
	const int maxP = maxPeriods(samples);
	QHash<QString, ToneSpec> tones;
	for(const ChannelControls &ctrl : std::as_const(m_channels)) {
		ctrl.periods->setMaxValue(maxP);
		ToneSpec t;
		t.enabled = true;
		t.waveform = Waveform(ctrl.waveform->combo()->currentIndex());
		t.frequency = std::clamp(std::round(ctrl.periods->value()), 0.0, double(maxP));
		if(t.frequency != ctrl.periods->value()) {
			ctrl.periods->setValueSilent(t.frequency);
		}
		t.amplitude = ctrl.amplitude->value();
		t.offset = ctrl.offset->value();
		t.phase = ctrl.phase->value();
		tones.insert(ctrl.id, t);
	}
	m_gen->setSamples(size_t(samples));
	m_gen->setCyclic(m_cyclic->onOffswitch()->isChecked());
	m_gen->setTones(tones);
	updateFrequencies();
	updatePreview();
}

void RawGeneratorView::updateFrequencies()
{
	const double fs = sampleRate();
	const double samples = double(m_gen->samples());
	const QHash<QString, ToneSpec> tones = m_gen->tones();
	for(const ChannelControls &ctrl : std::as_const(m_channels)) {
		const double periods = tones.value(ctrl.id).frequency;
		ctrl.frequency->setText(fs > 0 ? QStringLiteral("Tone: %1 Hz at %2 Hz sample rate")
							 .arg(periods * fs / samples, 0, 'g', 6)
							 .arg(fs, 0, 'g', 6)
					       : QStringLiteral("Tone: %1 period(s) per buffer").arg(periods));
	}
}

void RawGeneratorView::updatePreview()
{
	const int samples = int(m_gen->samples());
	const QHash<QString, ToneSpec> tones = m_gen->tones();
	QVector<float> x(samples);
	for(int i = 0; i < samples; ++i) {
		x[i] = float(i);
	}
	double ylo = std::numeric_limits<double>::max();
	double yhi = std::numeric_limits<double>::lowest();
	for(auto it = m_plotChannels.cbegin(); it != m_plotChannels.cend(); ++it) {
		const QVector<double> v = generateRawTone(tones.value(it.key()), samples);
		QVector<float> y(v.size());
		for(int i = 0; i < v.size(); ++i) {
			y[i] = float(v[i]);
			ylo = std::min(ylo, v[i]);
			yhi = std::max(yhi, v[i]);
		}
		it.value()->setSamples(x.data(), y.data(), size_t(y.size()), true);
	}
	if(!m_plot->navigator()->isZoomed()) {
		m_plot->xAxis()->setInterval(0, samples);
		if(ylo <= yhi) {
			const double span = yhi > ylo ? yhi - ylo : std::max(1.0, std::abs(yhi));
			m_plot->yAxis()->setInterval(ylo - span * 0.1, yhi + span * 0.1);
		}
	}
	m_plot->replot();
}

void RawGeneratorView::onRunningChanged(bool running)
{
	if(!running) {
		QSignalBlocker blocker(m_runBtn);
		m_runBtn->setChecked(false);
		m_runBtn->setText("Start");
		if(!m_status->text().contains("failed") && !m_status->text().startsWith("No channel")) {
			setStatus("Stopped");
		}
	} else {
		setStatus("Starting");
	}
}

void RawGeneratorView::setStatus(const QString &text, bool error)
{
	m_status->setText(text);
	Style::setStyle(m_status, style::properties::label::warning, error);
	Style::setStyle(m_status, style::properties::label::subtle, !error);
}

#include "moc_rawgeneratorview.cpp"
