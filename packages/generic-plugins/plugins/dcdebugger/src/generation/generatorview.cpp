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

#include "generatorview.h"

#include "nodepanel.h"
#include "streamutils.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

#include <component/attribute.h>
#include <component/attributereader.h>
#include <component/device.h>
#include <component/outputstream.h>
#include <component/scanelement.h>
#include <iiowidget.h>
#include <iiowidgetbuilder.h>
#include <menucombo.h>
#include <menucontrolbutton.h>
#include <menuonoffswitch.h>
#include <menusectionwidget.h>
#include <menuspinbox.h>
#include <menuwidget.h>
#include <mousewheelwidgetguard.h>
#include <plot_utils.hpp>
#include <plotaxis.h>
#include <plotchannel.h>
#include <plotnavigator.hpp>
#include <plotwidget.h>
#include <pluginbase/statusbarmanager.h>
#include <style.h>
#include <stylehelper.h>
#include <toolbuttons.h>
#include <tooltemplate.h>
#include <verticalchannelmanager.h>

using namespace scopy;
using namespace scopy::dcdebugger;
using namespace scopy::component;

namespace {
const char *const SETTINGS_KEY = "settings";
const char *const CHANNELS_KEY = "channels";

QString channelMenuKey(const QString &id) { return "channel:" + id; }

gui::MenuSpinbox *makeSpin(const QString &name, double value, const QString &unit, double min, double max, double step,
			   QWidget *parent)
{
	auto *sb = new gui::MenuSpinbox(name, value, unit, min, max, true, false, false, parent);
	sb->setIncrementMode(gui::MenuSpinbox::IS_FIXED);
	if(auto *fixed = dynamic_cast<gui::IncrementStrategyFixed *>(sb->incrementStrategy())) {
		fixed->setK(step);
	}
	sb->setScalingEnabled(false);
	return sb;
}
} // namespace

GeneratorView::GeneratorView(OutputStream *stream, QWidget *parent)
	: QWidget(parent)
	, m_stream(stream)
	, m_gen(new StreamGeneration(stream, this))
	, m_applyTimer(new QTimer(this))
{
	m_sampleRateAttr = findSampleRate(stream, true);
	m_applyTimer->setSingleShot(true);
	m_applyTimer->setInterval(150);
	connect(m_applyTimer, &QTimer::timeout, this, &GeneratorView::apply);

	auto *lay = new QVBoxLayout(this);
	lay->setContentsMargins(0, 0, 0, 0);
	// ToolTemplate starts with every container hidden.
	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->centralContainer()->setVisible(true);
	m_tool->rightContainer()->setVisible(true);
	m_tool->leftContainer()->setVisible(true);
	m_tool->topContainerMenuControl()->setVisible(false);
	m_tool->setRightContainerWidth(300);
	m_tool->setLeftContainerWidth(200);
	lay->addWidget(m_tool);

	m_status = new QLabel("Stopped", this);
	Style::setStyle(m_status, style::properties::label::subtle);
	m_runBtn = new RunBtn(this);
	m_runBtn->setText("Start");
	disconnect(m_runBtn, &QPushButton::toggled, m_runBtn, nullptr); // RunBtn's own Run / Stop text
	connect(m_runBtn, &QPushButton::toggled, m_runBtn,
		[this](bool on) { m_runBtn->setText(on ? "Stop" : "Start"); });
	auto *gearBtn = new GearBtn(this);
	gearBtn->setChecked(true);
	m_tool->addWidgetToTopContainerHelper(m_status, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);
	m_tool->addWidgetToTopContainerHelper(gearBtn, TTA_RIGHT);
	m_tool->rightStack()->add(SETTINGS_KEY, buildSettings(this));
	m_tool->openRightContainerHelper(true);
	connect(gearBtn, &QPushButton::toggled, m_tool, &ToolTemplate::openRightContainerHelper);

	buildPreview();
	buildChannels();
	m_tool->openLeftContainerHelper(true);
	m_tool->addWidgetToCentralContainerHelper(m_plot);

	connect(m_runBtn, &QPushButton::toggled, this, [this](bool on) {
		if(on) {
			apply();
			m_gen->start();
		} else {
			m_gen->stop();
		}
	});
	connect(m_gen, &StreamGeneration::runningChanged, this, &GeneratorView::onRunningChanged);
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

	apply();
}

GeneratorView::~GeneratorView() = default;

QWidget *GeneratorView::buildSettings(QWidget *parent)
{
	auto *scroll = new QScrollArea(parent);
	scroll->setWidgetResizable(true);
	scroll->setFrameShape(QFrame::NoFrame);
	scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	auto *content = new QWidget(scroll);
	auto *outer = new QVBoxLayout(content);
	outer->setContentsMargins(0, 0, 0, 0);

	MenuSectionCollapseWidget *section = makeSection("Output", content);
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
	m_samples->setIncrementMode(gui::MenuSpinbox::IS_POW2);
	m_samples->setScalingEnabled(false);
	connect(m_samples, &gui::MenuSpinbox::valueChanged, this, &GeneratorView::scheduleApply);
	lay->addWidget(m_samples);

	if(m_sampleRateAttr) {
		IIOWidget *rate =
			IIOWidgetBuilder(section).attribute(m_sampleRateAttr).title("Sample rate").buildSingle();
		if(rate) {
			lay->addWidget(rate);
			rate->readAsync();
		}
		connect(m_sampleRateAttr, &Attribute::valueChanged, this, &GeneratorView::scheduleApply);
	} else {
		// No sample-rate attribute: the value only places the tones in the buffer.
		m_manualRate = new gui::MenuSpinbox("Sample rate", 1e6, "Hz", 1, 1e12, true, false, false, section);
		connect(m_manualRate, &gui::MenuSpinbox::valueChanged, this, &GeneratorView::scheduleApply);
		lay->addWidget(m_manualRate);
	}

	m_cyclic = new MenuOnOffSwitch("Cyclic", section);
	m_cyclic->onOffswitch()->setChecked(m_gen->cyclic());
	m_cyclic->setEnabled(m_gen->supportsCyclic());
	m_cyclic->setToolTip(m_gen->supportsCyclic()
				     ? "On: one buffer is pushed and replayed by the device. Off: the buffer is "
				       "pushed continuously."
				     : "This stream cannot replay a buffer: it is pushed continuously.");
	connect(m_cyclic->onOffswitch(), &QAbstractButton::toggled, this, &GeneratorView::scheduleApply);
	lay->addWidget(m_cyclic);

	outer->addWidget(section);
	outer->addStretch();
	scroll->setWidget(content);
	return scroll;
}

void GeneratorView::buildChannels()
{
	// Left: "Output" (general settings) on top, then one button per scan element. Selecting a
	// button shows its menu on the right; its checkbox enables the channel.
	auto *vcm = new VerticalChannelManager(this);
	m_menuGroup = new QButtonGroup(this);
	m_menuGroup->setExclusive(true);

	auto *settingsBtn = new MenuControlButton(vcm);
	settingsBtn->setName("Output");
	settingsBtn->setOpenMenuChecksThis(true);
	settingsBtn->setDoubleClickToOpenMenu(false);
	settingsBtn->checkBox()->setVisible(false);
	settingsBtn->button()->setVisible(false);
	settingsBtn->setCheckable(true);
	settingsBtn->setChecked(true);
	vcm->addTop(settingsBtn);
	m_menuGroup->addButton(settingsBtn);
	connect(settingsBtn, &QAbstractButton::clicked, this, [this]() { m_tool->rightStack()->show(SETTINGS_KEY); });

	const QList<ScanElement *> elems = m_gen->scanElements();
	const QHash<QString, ToneSpec> tones = m_gen->tones();
	for(int i = 0; i < elems.size(); ++i) {
		const QString id = elems[i]->id();
		const ToneSpec tone = tones.value(id);
		ChannelControls ctrl;
		ctrl.id = id;
		ctrl.button = new MenuControlButton(vcm);
		ctrl.button->setName(scanElementName(m_stream, id, true));
		ctrl.button->setCheckBoxStyle(MenuControlButton::CS_CIRCLE);
		ctrl.button->setColor(QColor(StyleHelper::getChannelColor(i)));
		ctrl.button->setOpenMenuChecksThis(true);
		ctrl.button->setDoubleClickToOpenMenu(true);
		ctrl.button->button()->setVisible(false);
		ctrl.button->setCheckable(true);
		ctrl.button->checkBox()->setChecked(tone.enabled);
		ctrl.button->enableToolTip(true);
		vcm->add(ctrl.button);
		m_menuGroup->addButton(ctrl.button);

		m_tool->rightStack()->add(channelMenuKey(id), buildChannelMenu(ctrl, i, tone));
		connect(ctrl.button, &QAbstractButton::clicked, this,
			[this, id]() { m_tool->rightStack()->show(channelMenuKey(id)); });
		connect(ctrl.button->checkBox(), &QCheckBox::toggled, this, &GeneratorView::scheduleApply);
		m_channels.append(ctrl);
	}
	m_tool->leftStack()->add(CHANNELS_KEY, vcm);
	m_tool->rightStack()->show(SETTINGS_KEY);
}

QWidget *GeneratorView::buildChannelMenu(ChannelControls &ctrl, int index, const ToneSpec &tone)
{
	auto *menu = new gui::MenuWidget(scanElementName(m_stream, ctrl.id, true),
					 QPen(QColor(StyleHelper::getChannelColor(index))), this);

	auto *signal = new MenuSectionCollapseWidget("SIGNAL", MenuCollapseSection::MHCW_NONE,
						     MenuCollapseSection::MHW_BASEWIDGET, menu);
	signal->contentLayout()->setSpacing(8);
	ctrl.waveform = new MenuCombo("Waveform", signal);
	ctrl.waveform->combo()->addItems(waveformNames());
	ctrl.waveform->combo()->setCurrentIndex(int(tone.waveform));
	ctrl.frequency = new gui::MenuSpinbox("Frequency", tone.frequency, "Hz", 0, 1e12, true, false, false, signal);
	ctrl.frequency->setIncrementMode(gui::MenuSpinbox::IS_125);
	ctrl.amplitude = makeSpin("Amplitude", tone.amplitude, "FS", 0, 1, 0.05, signal);
	ctrl.offset = makeSpin("Offset", tone.offset, "FS", -1, 1, 0.05, signal);
	ctrl.phase = makeSpin("Phase", tone.phase, "°", -360, 360, 15, signal);
	for(QWidget *w :
	    std::initializer_list<QWidget *>{ctrl.waveform, ctrl.frequency, ctrl.amplitude, ctrl.offset, ctrl.phase}) {
		signal->contentLayout()->addWidget(w);
	}
	menu->add(signal, "signal", gui::MenuWidget::MA_TOPLAST);

	connect(ctrl.waveform->combo(), qOverload<int>(&QComboBox::currentIndexChanged), this,
		&GeneratorView::scheduleApply);
	for(gui::MenuSpinbox *sb : {ctrl.frequency, ctrl.amplitude, ctrl.offset, ctrl.phase}) {
		connect(sb, &gui::MenuSpinbox::valueChanged, this, &GeneratorView::scheduleApply);
	}

	// Scrolling the menu must not change the values under the cursor.
	auto *guard = new MouseWheelWidgetGuard(menu);
	guard->installEventRecursively(menu);
	return menu;
}

void GeneratorView::buildPreview()
{
	m_plot = new PlotWidget(this);
	m_plot->xAxis()->scaleDraw()->setFormatter(new MetricPrefixFormatter());
	m_plot->xAxis()->scaleDraw()->setFloatPrecision(2);
	m_plot->xAxis()->scaleDraw()->setUnitType("s");
	m_plot->yAxis()->scaleDraw()->setUnitType("%FS");
	m_plot->yAxis()->setInterval(-110, 110);
	m_plot->xAxis()->setInterval(0, 1);
	m_plot->setShowXAxisLabels(true);
	m_plot->setShowYAxisLabels(true);
	m_plot->navigator()->setBounded(false);
	const QList<ScanElement *> elems = m_gen->scanElements();
	for(int c = 0; c < elems.size(); ++c) {
		QPen pen(QColor(StyleHelper::getChannelColor(c)), 1);
		auto *ch = new PlotChannel(elems[c]->id(), pen, m_plot->xAxis(), m_plot->yAxis(), m_plot);
		m_plot->addPlotChannel(ch);
		ch->enable();
		m_plotChannels.insert(elems[c]->id(), ch);
	}
}

double GeneratorView::sampleRate() const
{
	if(m_sampleRateAttr) {
		return sampleRateOf(m_sampleRateAttr);
	}
	return m_manualRate ? m_manualRate->value() : 0;
}

void GeneratorView::scheduleApply() { m_applyTimer->start(); }

void GeneratorView::apply()
{
	m_applyTimer->stop();
	QHash<QString, ToneSpec> tones;
	for(const ChannelControls &ctrl : std::as_const(m_channels)) {
		ToneSpec t;
		t.enabled = ctrl.button->checkBox()->isChecked();
		t.waveform = Waveform(ctrl.waveform->combo()->currentIndex());
		t.frequency = ctrl.frequency->value();
		t.amplitude = ctrl.amplitude->value();
		t.offset = ctrl.offset->value();
		t.phase = ctrl.phase->value();
		tones.insert(ctrl.id, t);
	}
	m_gen->setSampleRate(sampleRate());
	m_gen->setSamples(size_t(std::max(2.0, m_samples->value())));
	m_gen->setCyclic(m_cyclic->onOffswitch()->isChecked());
	m_gen->setTones(tones);

	// Show the frequency that is actually generated.
	const QHash<QString, ToneSpec> snapped = m_gen->snappedTones();
	for(const ChannelControls &ctrl : std::as_const(m_channels)) {
		const double f = snapped.value(ctrl.id).frequency;
		if(std::abs(f - ctrl.frequency->value()) > 1e-9) {
			ctrl.frequency->setValueSilent(f);
		}
		ctrl.frequency->setToolTip(
			QStringLiteral("Generated: %1 Hz (whole periods in the buffer)").arg(f, 0, 'f', 3));
	}
	updatePreview();
}

void GeneratorView::updatePreview()
{
	const double fs = sampleRate();
	const int samples = int(m_gen->samples());
	const QHash<QString, ToneSpec> tones = m_gen->snappedTones();

	// A few periods of the slowest enabled tone (the whole buffer at most).
	double slowest = 0;
	for(const ToneSpec &t : tones) {
		if(t.enabled && t.frequency > 0 && t.waveform != Waveform::Constant) {
			slowest = slowest > 0 ? std::min(slowest, t.frequency) : t.frequency;
		}
	}
	int shown = samples;
	if(fs > 0 && slowest > 0) {
		shown = std::clamp(int(std::ceil(4 * fs / slowest)), 2, samples);
	}

	QVector<float> x(shown);
	for(int i = 0; i < shown; ++i) {
		x[i] = fs > 0 ? float(i / fs) : float(i);
	}
	m_plot->xAxis()->scaleDraw()->setUnitType(fs > 0 ? "s" : "");
	for(auto it = m_plotChannels.cbegin(); it != m_plotChannels.cend(); ++it) {
		const ToneSpec t = tones.value(it.key());
		if(!t.enabled) {
			it.value()->curve()->setSamples(QVector<QPointF>());
			continue;
		}
		const QVector<double> v = generateTone(t, shown, fs);
		QVector<float> y(shown);
		for(int i = 0; i < shown; ++i) {
			y[i] = float(v[i] * 100);
		}
		it.value()->setSamples(x.data(), y.data(), size_t(shown), true);
	}
	if(!m_plot->navigator()->isZoomed()) {
		m_plot->xAxis()->setInterval(0, shown > 1 ? x.last() : 1);
	}
	m_plot->replot();
}

void GeneratorView::onRunningChanged(bool running)
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

void GeneratorView::setStatus(const QString &text, bool error)
{
	m_status->setText(text);
	Style::setStyle(m_status, style::properties::label::warning, error);
	Style::setStyle(m_status, style::properties::label::subtle, !error);
}

#include "moc_generatorview.cpp"
