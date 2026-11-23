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

#include "acquisitionview.h"

#include "measurementstable.h"
#include "nodepanel.h"
#include "sidesplitter.h"
#include "streamutils.h"

#include <QCheckBox>
#include <algorithm>
#include <cmath>
#include <limits>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QVBoxLayout>

#include <component/attribute.h>
#include <dcdebugger/style_properties.h>
#include <component/channel.h>
#include <component/context.h>
#include <component/device.h>
#include <component/inputstream.h>
#include <component/navigation.h>
#include <component/scanelement.h>
#include <component/tags.h>
#include <component/trigger.h>
#include <filemanager.h>
#include <iiowidget.h>
#include <iiowidgetbuilder.h>
#include <menucombo.h>
#include <menucontrolbutton.h>
#include <menuonoffswitch.h>
#include <menusectionwidget.h>
#include <menuspinbox.h>
#include <plot_utils.hpp>
#include <plotaxis.h>
#include <plotchannel.h>
#include <plotnavigator.hpp>
#include <plotwidget.h>
#include <pluginbase/statusbarmanager.h>
#include <qcoro/qcorotask.h>
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
} // namespace

AcquisitionView::AcquisitionView(InputStream *stream, QWidget *parent)
	: QWidget(parent)
	, m_stream(stream)
	, m_acq(new StreamAcquisition(stream, this))
{
	m_sampleRateAttr = findSampleRate(stream, false);
	if(stream->parent()) {
		m_triggerable = stream->parent()->findChild<Triggerable *>(QString(), Qt::FindDirectChildrenOnly);
	}

	auto *lay = new QVBoxLayout(this);
	lay->setContentsMargins(0, 0, 0, 0);
	// ToolTemplate starts with every container hidden.
	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->centralContainer()->setVisible(true);
	m_tool->topContainerMenuControl()->setVisible(false);
	lay->addWidget(m_tool);
	// Channels | plot | menus, resizable; a side dragged to the edge hides.
	m_sides = new SideSplitter(200, 300, this);
	m_tool->addWidgetToCentralContainerHelper(m_sides);

	m_status = new QLabel("Stopped", this);
	Style::setStyle(m_status, style::properties::label::subtle);
	m_runBtn = new RunBtn(this);
	m_singleBtn = new SingleShotBtn(this);
	auto *gearBtn = new GearBtn(this);
	gearBtn->setChecked(true);
	m_tool->addWidgetToTopContainerHelper(m_status, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);
	m_tool->addWidgetToTopContainerHelper(m_singleBtn, TTA_RIGHT);
	m_exportBtn = new QPushButton("Export...", this);
	m_exportBtn->setToolTip("Save what the plot shows as CSV");
	// basicButton look at the Run / Single height.
	Style::setStyle(m_exportBtn, style::properties::dcdebugger::topButton);
	m_exportBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_tool->addWidgetToTopContainerHelper(m_exportBtn, TTA_RIGHT);
	connect(m_exportBtn, &QPushButton::clicked, this, &AcquisitionView::exportData);
	m_tool->addWidgetToTopContainerHelper(gearBtn, TTA_RIGHT);

	buildChannels();
	m_sides->rightStack()->add(SETTINGS_KEY, buildSettings(this));
	connect(gearBtn, &QPushButton::toggled, m_sides, &SideSplitter::setRightOpen);
	connect(m_sides, &SideSplitter::rightOpenChanged, gearBtn, &QAbstractButton::setChecked);

	buildPlot();
	applyChannelSelection();

	connect(m_runBtn, &QPushButton::toggled, this, [this](bool on) {
		m_singleBtn->setEnabled(!on);
		if(on) {
			m_acq->start(false);
		} else {
			m_acq->stop();
		}
	});
	connect(m_singleBtn, &QPushButton::toggled, this, [this](bool on) {
		m_runBtn->setEnabled(!on);
		if(on) {
			m_acq->start(true);
		}
	});
	connect(m_acq, &StreamAcquisition::runningChanged, this, &AcquisitionView::onRunningChanged);
	connect(m_acq, &StreamAcquisition::frameReady, this, &AcquisitionView::onFrame);
	connect(m_acq, &StreamAcquisition::errorOccurred, this, [this](const QString &msg) {
		setStatus(msg, true);
		StatusBarManager::pushMessage("DC Debugger: " + msg, 5000);
	});
}

AcquisitionView::~AcquisitionView() = default;

QWidget *AcquisitionView::buildSettings(QWidget *parent)
{
	auto *scroll = new QScrollArea(parent);
	scroll->setWidgetResizable(true);
	scroll->setFrameShape(QFrame::NoFrame);
	scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	auto *content = new QWidget(scroll);
	auto *lay = new QVBoxLayout(content);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(10);
	lay->addWidget(buildAcquisitionSection(content));
	lay->addWidget(buildViewSection(content));
	lay->addWidget(buildSoftwareTriggerSection(content));
	lay->addStretch();
	scroll->setWidget(content);
	return scroll;
}

QWidget *AcquisitionView::buildAcquisitionSection(QWidget *parent)
{
	MenuSectionCollapseWidget *section = makeSection("Acquisition", parent);
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

	m_samples = new gui::MenuSpinbox("Samples", double(m_acq->samples()), "", 2, 4 * 1024 * 1024, true, false,
					 false, section);
	m_samples->setIncrementMode(gui::MenuSpinbox::IS_POW2);
	m_samples->setScalingEnabled(false);
	connect(m_samples, &gui::MenuSpinbox::valueChanged, this,
		[this](double v) { m_acq->setSamples(size_t(std::max(2.0, v))); });
	lay->addWidget(m_samples);

	if(m_sampleRateAttr) {
		IIOWidget *rate =
			IIOWidgetBuilder(section).attribute(m_sampleRateAttr).title("Sample rate").buildSingle();
		if(rate) {
			lay->addWidget(rate);
			rate->readAsync();
		}
	} else {
		// No sample-rate attribute: the value only scales the time axis.
		m_manualRate = new gui::MenuSpinbox("Sample rate (time axis)", 1e6, "Hz", 1, 1e12, true, false, false,
						    section);
		lay->addWidget(m_manualRate);
	}

	if(m_triggerable) {
		lay->addWidget(buildHardwareTrigger(section));
	}
	return section;
}

void AcquisitionView::buildChannels()
{
	// The channel list on the left, as in the Scopy instruments: one colored checkbox per scan
	// element (the plot channel shares the color), in a group whose switch toggles them all.
	auto *vcm = new VerticalChannelManager(this);
	const QList<ScanElement *> elems = m_acq->scanElements();
	const QList<int> enabled = m_acq->enabledChannels();
	QString name = QStringLiteral("Channels");
	if(auto *dev = qobject_cast<Device *>(m_stream->parent())) {
		name = dev->name().isEmpty() ? dev->id() : dev->name();
	}
	auto *group = new CollapsableMenuControlButton(vcm);
	group->getControlBtn()->setName(name);
	group->getControlBtn()->button()->setVisible(false);
	group->getControlBtn()->setCheckable(false);
	group->enableOnOffSwitch(true);
	group->onOffSwitch()->setChecked(!enabled.isEmpty());
	vcm->add(group);
	for(int i = 0; i < elems.size(); ++i) {
		ScanElement *el = elems[i];
		auto *btn = new MenuControlButton(group);
		btn->setName(displayName(el->id()));
		btn->setCheckBoxStyle(MenuControlButton::CS_CIRCLE);
		btn->setColor(QColor(StyleHelper::getChannelColor(i)));
		btn->button()->setVisible(false);
		btn->setCheckable(false);
		btn->enableToolTip(true);
		btn->checkBox()->setChecked(enabled.contains(int(el->index())));
		group->add(btn);
		QCheckBox *box = btn->checkBox();
		// No channel menu: a click on the row toggles the channel.
		connect(btn, &QAbstractButton::clicked, box, &QCheckBox::toggle);
		connect(group->onOffSwitch(), &QAbstractButton::toggled, box, &QCheckBox::setChecked);
		connect(box, &QCheckBox::toggled, this, &AcquisitionView::applyChannelSelection);
		m_channelBoxes.insert(el->id(), box);
		m_channelIndex.insert(el->id(), int(el->index()));
	}
	if(elems.isEmpty()) {
		auto *none = new QLabel("This stream exposes no scan elements.", vcm);
		none->setWordWrap(true);
		vcm->add(none);
	}
	m_sides->leftStack()->add(CHANNELS_KEY, vcm);
}

QWidget *AcquisitionView::buildHardwareTrigger(QWidget *parent)
{
	m_hwTrigger = new MenuCombo("Trigger", parent);
	refreshTriggerSources();
	connect(m_hwTrigger->combo(), qOverload<int>(&QComboBox::activated), this, [this](int idx) {
		auto *source = m_hwTrigger->combo()->itemData(idx).value<QObject *>();
		auto *trigger = qobject_cast<Trigger *>(source);
		QCoro::connect(m_triggerable->setTriggerAsync(trigger), this, [this](const CommandResponse<void> &r) {
			if(!r) {
				setStatus("trigger: " + r.error().errorString(), true);
			}
			refreshTriggerSources();
		});
	});
	return m_hwTrigger;
}

void AcquisitionView::refreshTriggerSources()
{
	if(!m_hwTrigger || !m_triggerable) {
		return;
	}
	QComboBox *combo = m_hwTrigger->combo();
	QSignalBlocker blocker(combo);
	combo->clear();
	combo->addItem("(none)", QVariant::fromValue<QObject *>(nullptr));
	const QString assigned = m_triggerable->assignedTriggerName();
	if(Context *ctx = contextOf(m_stream)) {
		for(Trigger *t : ctx->findChildren<Trigger *>()) {
			combo->addItem(t->name(), QVariant::fromValue<QObject *>(t));
			if(t->name() == assigned) {
				combo->setCurrentIndex(combo->count() - 1);
			}
		}
	}
}

QWidget *AcquisitionView::buildSoftwareTriggerSection(QWidget *parent)
{
	m_swTrigger = new MenuSectionCollapseWidget("Software trigger", MenuCollapseSection::MHCW_ONOFF,
						    MenuCollapseSection::MHW_BASEWIDGET, parent);
	Style::setStyle(m_swTrigger, style::properties::widget::border_interactive);
	m_swTrigger->contentLayout()->setSpacing(8);
	m_swTrigger->setCollapsed(true);

	m_swSource = new MenuCombo("Source", m_swTrigger);
	for(ScanElement *el : m_acq->scanElements()) {
		m_swSource->combo()->addItem(displayName(el->id()), el->id());
	}
	m_swMode = new MenuCombo("Mode", m_swTrigger);
	m_swMode->combo()->addItem("Normal");
	m_swMode->combo()->addItem("Auto");
	m_swMode->setToolTip("Normal: only triggered buffers are shown, the plot holds the last one.\n"
			     "Auto: buffers without a trigger are shown untriggered.");
	m_swEdge = new MenuCombo("Edge", m_swTrigger);
	m_swEdge->combo()->addItem("Rising");
	m_swEdge->combo()->addItem("Falling");
	m_swLevel = new gui::MenuSpinbox("Level", 0, "", -1e12, 1e12, true, false, false, m_swTrigger);
	m_swLevel->setIncrementMode(gui::MenuSpinbox::IS_FIXED);
	m_swLevel->setToolTip("The edge is searched in the first half of the buffer; the plot shows the half "
			      "buffer that follows it.");

	m_swTrigger->contentLayout()->addWidget(m_swMode);
	m_swTrigger->contentLayout()->addWidget(m_swSource);
	m_swTrigger->contentLayout()->addWidget(m_swEdge);
	m_swTrigger->contentLayout()->addWidget(m_swLevel);
	return m_swTrigger;
}

void AcquisitionView::buildPlot()
{
	QWidget *wrapper = new QWidget(this);
	QVBoxLayout *lay = new QVBoxLayout(wrapper);
	lay->setContentsMargins(0, 0, 0, 0);

	m_plot = new PlotWidget(wrapper);
	m_plot->xAxis()->scaleDraw()->setFormatter(new MetricPrefixFormatter());
	m_plot->xAxis()->scaleDraw()->setFloatPrecision(2);
	m_plot->yAxis()->scaleDraw()->setFormatter(new MetricPrefixFormatter());
	m_plot->yAxis()->scaleDraw()->setFloatPrecision(2);
	m_plot->xAxis()->setInterval(0, 1);
	m_plot->yAxis()->setInterval(-1, 1);
	m_plot->setShowXAxisLabels(true);
	m_plot->setShowYAxisLabels(true);
	// Wheel zoom-out may go past the data range.
	m_plot->navigator()->setBounded(false);

	const QList<ScanElement *> elems = m_acq->scanElements();
	for(int c = 0; c < elems.size(); ++c) {
		QPen pen(QColor(StyleHelper::getChannelColor(c)), 1);
		auto *ch = new PlotChannel(displayName(elems[c]->id()), pen, m_plot->xAxis(), m_plot->yAxis(), m_plot);
		m_plot->addPlotChannel(ch);
		m_plotChannels.insert(elems[c]->id(), ch);
	}
	m_plot->replot();
	lay->addWidget(m_plot);

	m_table = new MeasurementsTable(this);
	m_sides->setCentralWidget(new PlotMeasurementsArea(wrapper, m_table, this));
}

void AcquisitionView::applyChannelSelection()
{
	QList<int> indices;
	for(auto it = m_channelBoxes.cbegin(); it != m_channelBoxes.cend(); ++it) {
		const bool on = it.value()->isChecked();
		if(on) {
			indices.append(m_channelIndex.value(it.key()));
		}
		if(PlotChannel *ch = m_plotChannels.value(it.key())) {
			on ? ch->enable() : ch->disable();
		}
	}
	m_acq->setEnabledChannels(indices);
	if(m_hasFrame) {
		render();
	} else {
		m_plot->replot();
	}
}

double AcquisitionView::sampleRate() const
{
	if(m_sampleRateAttr) {
		return sampleRateOf(m_sampleRateAttr);
	}
	return m_manualRate ? m_manualRate->value() : 0;
}

QString AcquisitionView::displayName(const QString &id) const { return scanElementName(m_stream, id, false); }

void AcquisitionView::onFrame(const AcquiredFrame &frame)
{
	int total = 0;
	for(const QVector<double> &d : frame.data) {
		total = std::max(total, int(d.size()));
	}
	if(total == 0) {
		return;
	}

	// Software trigger: show half a buffer starting at the first matching edge of the source.
	int start = 0;
	int count = total;
	const bool swTrigger = !m_swTrigger->collapsed();
	bool triggered = false;
	QString why;
	if(swTrigger) {
		count = std::max(1, total / 2);
		const int src = frame.ids.indexOf(m_swSource->combo()->currentData().toString());
		if(src < 0) {
			why = "source channel not acquired";
		} else {
			const QVector<double> &d = frame.data[src];
			const auto [lo, hi] = std::minmax_element(d.cbegin(), d.cend());
			const double level = m_swLevel->value();
			// 2% of the swing: noise around the level neither fires nor re-arms the trigger.
			const double hysteresis = (*hi - *lo) * 0.02;
			const int at = findEdge(d, level, m_swEdge->combo()->currentIndex() == 0, 0, total - count + 1,
						hysteresis);
			if(at >= 0) {
				start = at;
				triggered = true;
			} else if(level <= *lo || level >= *hi) {
				why = QStringLiteral("level %1 outside the source range [%2, %3]")
					      .arg(level, 0, 'g', 4)
					      .arg(*lo, 0, 'g', 4)
					      .arg(*hi, 0, 'g', 4);
			} else {
				why = QStringLiteral("no edge in the first %1 samples (period too long? raise Samples)")
					      .arg(total - count + 1);
			}
		}
		if(!triggered && m_swMode->combo()->currentIndex() == 0) {
			// Normal mode: keep showing the last triggered buffer.
			setStatus(QStringLiteral("Buffer %1 · waiting for trigger: %2").arg(frame.index).arg(why));
			return;
		}
	}

	m_frame = frame;
	m_hasFrame = true;
	m_start = start;
	m_count = count;
	render();

	QString text = QStringLiteral("Buffer %1 · %2 samples · %3 channel(s)")
			       .arg(frame.index)
			       .arg(total)
			       .arg(frame.ids.size());
	if(swTrigger) {
		text += triggered ? QStringLiteral(" · triggered") : QStringLiteral(" · not triggered: ") + why;
	}
	setStatus(text);
}

QWidget *AcquisitionView::buildViewSection(QWidget *parent)
{
	MenuSectionCollapseWidget *section = makeSection("View", parent);
	QVBoxLayout *lay = section->contentLayout();
	lay->setSpacing(8);

	m_mode = new MenuCombo("Mode", section);
	m_mode->combo()->addItems({"Time", "Spectrum", "XY"});
	m_units = new MenuCombo("Units", section);
	m_units->combo()->addItems({"Scaled", "Raw"});
	m_units->setToolTip("Scaled: the stream's scale / offset applied. Raw: decoded integers; the spectrum "
			    "is then in dBFS.");
	m_window = new MenuCombo("Window", section);
	m_window->combo()->addItems(windowNames());
	m_window->combo()->setCurrentIndex(int(WindowType::BlackmanHarris));
	m_averages = new gui::MenuSpinbox("Averages", 1, "", 1, 1000, true, false, false, section);
	m_averages->setIncrementMode(gui::MenuSpinbox::IS_FIXED);
	m_averages->setScalingEnabled(false);
	m_complexIQ = new MenuOnOffSwitch("Complex I/Q (pair channels)", section);
	m_complexIQ->setToolTip("Spectrum of consecutive enabled channels taken as I + jQ (1st + 2nd, 3rd + 4th, "
				"...), from -fs/2 to +fs/2.");
	m_complexIQ->onOffswitch()->setChecked(false);
	m_xySource = new MenuCombo("X channel", section);
	m_xySourceY = new MenuCombo("Y channel", section);
	for(ScanElement *el : m_acq->scanElements()) {
		m_xySource->combo()->addItem(displayName(el->id()), el->id());
		m_xySourceY->combo()->addItem(displayName(el->id()), el->id());
	}
	m_xySourceY->combo()->setCurrentIndex(m_xySourceY->combo()->count() > 1 ? 1 : 0);

	m_autoscale = new MenuOnOffSwitch("Y autoscale", section);
	m_autoscale->onOffswitch()->setChecked(true);
	m_yMin = new gui::MenuSpinbox("Y min", -1, "", -1e12, 1e12, true, false, false, section);
	m_yMax = new gui::MenuSpinbox("Y max", 1, "", -1e12, 1e12, true, false, false, section);
	for(gui::MenuSpinbox *sb : {m_yMin, m_yMax}) {
		sb->setIncrementMode(gui::MenuSpinbox::IS_FIXED);
		sb->setEnabled(false);
	}

	lay->addWidget(m_mode);
	lay->addWidget(m_xySource);
	lay->addWidget(m_xySourceY);
	lay->addWidget(m_units);
	lay->addWidget(m_window);
	lay->addWidget(m_averages);
	lay->addWidget(m_complexIQ);
	lay->addWidget(m_autoscale);
	lay->addWidget(m_yMin);
	lay->addWidget(m_yMax);

	for(MenuCombo *c : {m_mode, m_units, m_window, m_xySource, m_xySourceY}) {
		connect(c->combo(), qOverload<int>(&QComboBox::currentIndexChanged), this,
			&AcquisitionView::onViewChanged);
	}
	connect(m_averages, &gui::MenuSpinbox::valueChanged, this, &AcquisitionView::onViewChanged);
	connect(m_complexIQ->onOffswitch(), &QAbstractButton::toggled, this, &AcquisitionView::onViewChanged);
	connect(m_autoscale->onOffswitch(), &QAbstractButton::toggled, this, [this](bool on) {
		m_yMin->setEnabled(!on);
		m_yMax->setEnabled(!on);
		if(on) {
			render();
		} else {
			applyFixedYRange();
		}
	});
	for(gui::MenuSpinbox *sb : {m_yMin, m_yMax}) {
		connect(sb, &gui::MenuSpinbox::valueChanged, this, [this]() {
			if(!m_autoscale->onOffswitch()->isChecked()) {
				applyFixedYRange();
			}
		});
	}
	onViewChanged();
	return section;
}

void AcquisitionView::onViewChanged()
{
	const int mode = m_mode->combo()->currentIndex();
	m_window->setEnabled(mode == SpectrumMode);
	m_averages->setEnabled(mode == SpectrumMode);
	m_complexIQ->setEnabled(mode == SpectrumMode);
	m_xySource->setVisible(mode == XYMode);
	m_xySourceY->setVisible(mode == XYMode);

	// Each mode keeps its own fixed Y range (a dB range makes no sense in time mode).
	m_yRange[m_lastMode] = {m_yMin->value(), m_yMax->value()};
	if(mode != m_lastMode) {
		const auto range =
			m_yRange.value(mode, mode == SpectrumMode ? qMakePair(-140.0, 0.0) : qMakePair(-1.0, 1.0));
		m_yMin->setValueSilent(range.first);
		m_yMax->setValueSilent(range.second);
		for(PlotChannel *ch : std::as_const(m_plotChannels)) {
			ch->setStyle(mode == XYMode ? PlotChannel::PCS_DOTS : PlotChannel::PCS_LINES);
		}
		m_lastMode = mode;
	}

	m_averager.setCount(int(m_averages->value()));
	m_averager.reset();
	if(m_hasFrame) {
		render();
	}
}

void AcquisitionView::render()
{
	if(!m_hasFrame) {
		return;
	}
	const AcquiredFrame &f = m_frame;
	const bool raw = m_units->combo()->currentIndex() == 1;
	const QVector<QVector<double>> &src = raw ? f.raw : f.data;
	const int mode = m_mode->combo()->currentIndex();
	const double fs = sampleRate();
	const WindowType window = WindowType(m_window->combo()->currentIndex());

	m_shownX.clear();
	m_shownNames.clear();
	m_shownY.clear();
	double xlo = 0;
	double xhi = 1;
	double ylo = std::numeric_limits<double>::max();
	double yhi = std::numeric_limits<double>::lowest();
	QList<MeasurementRow> rows;

	int xChannel = -1;
	int yChannel = -1;
	if(mode == XYMode) {
		xChannel = f.ids.indexOf(m_xySource->combo()->currentData().toString());
		yChannel = f.ids.indexOf(m_xySourceY->combo()->currentData().toString());
		m_shownXLabel = m_xySource->combo()->currentText();
		if(xChannel >= 0) {
			m_shownX = src[xChannel].mid(m_start, m_count);
			const auto [lo, hi] = std::minmax_element(m_shownX.cbegin(), m_shownX.cend());
			if(lo != m_shownX.cend()) {
				xlo = *lo;
				xhi = *hi;
			}
		}
	} else if(mode == TimeMode) {
		m_shownXLabel = fs > 0 ? "time [s]" : "sample";
		m_shownX.resize(m_count);
		for(int i = 0; i < m_count; ++i) {
			m_shownX[i] = fs > 0 ? i / fs : i;
		}
		xhi = m_count > 1 ? m_shownX.last() : 1;
	}

	const QList<ScanElement *> elems = m_acq->scanElements();
	const bool iq = mode == SpectrumMode && m_complexIQ->onOffswitch()->isChecked();
	if(iq) {
		const auto xr = renderIQSpectrum(raw, fs, rows, ylo, yhi);
		xlo = xr.first;
		xhi = xr.second;
	}
	for(int e = 0; !iq && e < elems.size(); ++e) {
		const QString id = elems[e]->id();
		PlotChannel *pc = m_plotChannels.value(id);
		const int c = f.ids.indexOf(id);
		QCheckBox *box = m_channelBoxes.value(id);
		// XY: only the selected X and Y channels take part (both must be acquired).
		const bool xyMember = c >= 0 && (c == xChannel || c == yChannel) && xChannel >= 0 && yChannel >= 0;
		if(!pc || c < 0 || (mode == XYMode ? !xyMember : (!box || !box->isChecked()))) {
			if(pc) {
				pc->curve()->setSamples(QVector<QPointF>());
			}
			continue;
		}

		const QVector<double> &d = src[c];
		MeasurementRow row;
		row.channel = displayName(id);
		row.color = QColor(StyleHelper::getChannelColor(e));
		// Statistics follow what the plot shows: the (triggered) window, or the whole buffer.
		row.values = mode == SpectrumMode ? measure(d, 0, d.size(), fs) : measure(d, m_start, m_count, fs);

		QVector<double> y;
		if(mode == SpectrumMode) {
			QVector<double> amp =
				m_averager.push(id + (raw ? ":raw" : ":scaled"), amplitudeSpectrum(d, window));
			const int n = floorPow2(int(d.size()));
			const double ref = raw && f.bits.value(c) > 1 ? std::ldexp(1.0, f.bits.value(c) - 1) : 1.0;
			y.resize(amp.size());
			if(m_shownX.size() != amp.size()) {
				m_shownX.resize(amp.size());
				for(int k = 0; k < amp.size(); ++k) {
					m_shownX[k] = fs > 0 ? k * fs / n : k;
				}
			}
			int peak = amp.size() > 1 ? 1 : 0;
			for(int k = 0; k < amp.size(); ++k) {
				y[k] = toDb(amp[k], ref);
				if(k >= 1 && amp[k] > amp[peak]) {
					peak = k;
				}
			}
			if(!y.isEmpty()) {
				row.peak = QStringLiteral("%1 %2 @ %3")
						   .arg(y[peak], 0, 'f', 1)
						   .arg(raw ? "dBFS" : "dB")
						   .arg(fs > 0 ? siFormat(m_shownX[peak], "Hz")
							       : QStringLiteral("bin %1").arg(peak));
			}
			m_shownXLabel = fs > 0 ? "frequency [Hz]" : "bin";
			xhi = m_shownX.isEmpty() ? 1 : m_shownX.last();
		} else {
			y = d.mid(m_start, m_count);
		}
		rows.append(row);

		if(mode == XYMode && c != yChannel) {
			pc->curve()->setSamples(QVector<QPointF>());
			continue; // the X channel: in the table, not plotted
		}
		const int n = std::min(int(y.size()), int(m_shownX.size()));
		QVector<float> xf(n);
		QVector<float> yf(n);
		for(int i = 0; i < n; ++i) {
			xf[i] = float(m_shownX[i]);
			yf[i] = float(y[i]);
			ylo = std::min(ylo, y[i]);
			yhi = std::max(yhi, y[i]);
		}
		pc->setSamples(xf.data(), yf.data(), size_t(n), true);
		m_shownNames.append(row.channel);
		m_shownY.append(y);
	}

	m_table->setRows(rows, QString());
	m_plot->xAxis()->scaleDraw()->setUnitType(mode == SpectrumMode ? (fs > 0 ? "Hz" : "")
								       : (mode == TimeMode && fs > 0 ? "s" : ""));
	m_plot->yAxis()->scaleDraw()->setUnitType(mode == SpectrumMode ? (raw ? "dBFS" : "dB") : "");
	if(mode == SpectrumMode && ylo <= yhi) {
		ylo = std::max(ylo, yhi - 200); // empty bins sit at -300 dB
	}
	applyAxes(xlo, xhi, ylo, yhi);
	m_plot->replot();
}

QPair<double, double> AcquisitionView::renderIQSpectrum(bool raw, double fs, QList<MeasurementRow> &rows, double &ylo,
							double &yhi)
{
	const AcquiredFrame &f = m_frame;
	const QVector<QVector<double>> &src = raw ? f.raw : f.data;
	const WindowType window = WindowType(m_window->combo()->currentIndex());
	const QList<ScanElement *> elems = m_acq->scanElements();

	struct On
	{
		int element; // color / order
		int column;  // in the frame
		QString id;
	};
	QList<On> on;
	for(int e = 0; e < elems.size(); ++e) {
		const QString id = elems[e]->id();
		if(PlotChannel *pc = m_plotChannels.value(id)) {
			pc->curve()->setSamples(QVector<QPointF>());
		}
		const int c = f.ids.indexOf(id);
		QCheckBox *box = m_channelBoxes.value(id);
		if(c >= 0 && box && box->isChecked()) {
			on.append({e, c, id});
		}
	}

	double xlo = 0;
	double xhi = 1;
	m_shownXLabel = fs > 0 ? "frequency [Hz]" : "bin";
	for(int p = 0; p < on.size(); p += 2) {
		const On &i = on[p];
		const On *q = p + 1 < on.size() ? &on[p + 1] : nullptr; // an odd one out stays real
		const QString key = "iq:" + i.id + "+" + (q ? q->id : QString()) + (raw ? ":raw" : ":scaled");
		const QVector<double> amp = m_averager.push(
			key, complexAmplitudeSpectrum(src[i.column], q ? src[q->column] : QVector<double>(), window));
		const int n = int(amp.size());
		if(n < 2) {
			continue;
		}
		const double ref =
			raw && f.bits.value(i.column) > 1 ? std::ldexp(1.0, f.bits.value(i.column) - 1) : 1.0;
		QVector<double> y(n);
		if(m_shownX.size() != n) {
			m_shownX.resize(n);
			for(int k = 0; k < n; ++k) {
				m_shownX[k] = fs > 0 ? (k - n / 2) * fs / n : k - n / 2;
			}
		}
		int peak = -1;
		for(int k = 0; k < n; ++k) {
			y[k] = toDb(amp[k], ref);
			if(k != n / 2 && (peak < 0 || amp[k] > amp[peak])) { // DC excluded
				peak = k;
			}
		}
		const QString peakText = QStringLiteral("%1 %2 @ %3")
						 .arg(y[peak], 0, 'f', 1)
						 .arg(raw ? "dBFS" : "dB")
						 .arg(fs > 0 ? siFormat(m_shownX[peak], "Hz")
							     : QStringLiteral("bin %1").arg(int(m_shownX[peak])));

		const QString name = q ? displayName(i.id) + " + j·" + displayName(q->id) : displayName(i.id);
		QVector<float> xf(n);
		QVector<float> yf(n);
		for(int k = 0; k < n; ++k) {
			xf[k] = float(m_shownX[k]);
			yf[k] = float(y[k]);
			ylo = std::min(ylo, y[k]);
			yhi = std::max(yhi, y[k]);
		}
		m_plotChannels.value(i.id)->setSamples(xf.data(), yf.data(), size_t(n), true);
		m_shownNames.append(name);
		m_shownY.append(y);

		for(const On *member : {&i, q}) {
			if(!member) {
				continue;
			}
			const QVector<double> &d = src[member->column];
			MeasurementRow row;
			row.channel = displayName(member->id);
			row.color = QColor(StyleHelper::getChannelColor(member->element));
			row.values = measure(d, 0, d.size(), fs);
			row.peak = peakText + (q ? " (I/Q)" : QString());
			rows.append(row);
		}
		xlo = m_shownX.first();
		xhi = m_shownX.last();
	}
	return {xlo, xhi};
}

void AcquisitionView::applyAxes(double xlo, double xhi, double ylo, double yhi)
{
	// While the user is zoomed / panned the axes are theirs; right click (undo) gives them back.
	if(m_plot->navigator()->isZoomed()) {
		return;
	}
	const bool autoscale = m_autoscale->onOffswitch()->isChecked();
	if(autoscale && ylo <= yhi) {
		const double span = yhi > ylo ? yhi - ylo : std::max(1.0, std::abs(yhi));
		ylo -= span * 0.1;
		yhi += span * 0.1;
		m_yMin->setValueSilent(ylo);
		m_yMax->setValueSilent(yhi);
	} else {
		ylo = m_yMin->value();
		yhi = m_yMax->value();
	}
	if(m_mode->combo()->currentIndex() == XYMode) {
		if(autoscale && xhi > xlo) {
			const double span = xhi - xlo;
			xlo -= span * 0.1;
			xhi += span * 0.1;
		} else {
			xlo = ylo; // fixed range: square view
			xhi = yhi;
		}
	}
	if(xhi <= xlo) {
		xhi = xlo + 1;
	}
	if(yhi <= ylo) {
		yhi = ylo + 1;
	}
	if(m_plot->xAxis()->min() != xlo || m_plot->xAxis()->max() != xhi) {
		m_plot->xAxis()->setInterval(xlo, xhi);
	}
	if(m_plot->yAxis()->min() != ylo || m_plot->yAxis()->max() != yhi) {
		m_plot->yAxis()->setInterval(ylo, yhi);
	}
}

void AcquisitionView::applyFixedYRange()
{
	const double lo = m_yMin->value();
	const double hi = std::max(m_yMax->value(), lo + 1e-12);
	m_plot->yAxis()->setInterval(lo, hi);
	if(m_mode->combo()->currentIndex() == XYMode) {
		m_plot->xAxis()->setInterval(lo, hi);
	}
	m_plot->replot();
}

void AcquisitionView::exportData()
{
	const QString csv = csvData();
	if(csv.isEmpty()) {
		setStatus("Nothing to export: acquire a buffer first", true);
		return;
	}
	if(FileManagerHelper::saveDataToFile(this, csv, "Export plot data")) {
		setStatus("Exported " + QString::number(m_shownX.size()) + " rows");
	}
}

QString AcquisitionView::csvData() const
{
	if(m_shownY.isEmpty()) {
		return {};
	}
	QString csv = m_shownXLabel;
	for(const QString &name : std::as_const(m_shownNames)) {
		csv += "," + name;
	}
	csv += "\n";
	for(int i = 0; i < m_shownX.size(); ++i) {
		csv += QString::number(m_shownX[i], 'g', 12);
		for(const QVector<double> &y : std::as_const(m_shownY)) {
			csv += "," + (i < y.size() ? QString::number(y[i], 'g', 12) : QString());
		}
		csv += "\n";
	}
	return csv;
}

void AcquisitionView::onRunningChanged(bool running)
{
	if(!running) {
		// Not signal-blocked: the buttons switch their text ("Stop" back to "Run" / "Single") on
		// toggled. The handlers above only stop an acquisition that is no longer running.
		m_runBtn->setChecked(false);
		m_singleBtn->setChecked(false);
		m_runBtn->setEnabled(true);
		m_singleBtn->setEnabled(true);
	}
	if(running) {
		setStatus("Running");
	}
}

void AcquisitionView::setStatus(const QString &text, bool error)
{
	m_status->setText(text);
	Style::setStyle(m_status, style::properties::label::warning, error);
	Style::setStyle(m_status, style::properties::label::subtle, !error);
}

#include "moc_acquisitionview.cpp"
