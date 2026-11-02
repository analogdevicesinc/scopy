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

#include "nodepanel.h"

#include <QCheckBox>
#include <algorithm>
#include <QLabel>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QVBoxLayout>

#include <component/attribute.h>
#include <component/channel.h>
#include <component/context.h>
#include <component/device.h>
#include <component/inputstream.h>
#include <component/navigation.h>
#include <component/scanelement.h>
#include <component/tags.h>
#include <component/trigger.h>
#include <iiowidget.h>
#include <iiowidgetbuilder.h>
#include <menucombo.h>
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

using namespace scopy;
using namespace scopy::dcdebugger;
using namespace scopy::component;

namespace {
const char *const SETTINGS_KEY = "settings";

// Sample-rate attribute of a stream: tagged on the stream, its device or one of the device's
// input channels; the conventional name as fallback.
Attribute *findSampleRate(InputStream *stream)
{
	QObject *dev = stream->parent();
	for(QObject *scope : {static_cast<QObject *>(stream), dev}) {
		const QList<Attribute *> tagged = findByTag<Attribute>(scope, tags::SampleRate, false);
		if(!tagged.isEmpty()) {
			return tagged.first();
		}
	}
	if(dev) {
		for(Channel *ch : dev->findChildren<Channel *>(QString(), Qt::FindDirectChildrenOnly)) {
			const QList<Attribute *> tagged = findByTag<Attribute>(ch, tags::SampleRate, false);
			if(!ch->isOutput() && !tagged.isEmpty()) {
				return tagged.first();
			}
		}
	}
	for(QObject *scope : {static_cast<QObject *>(stream), dev}) {
		if(Attribute *a = attributeByName(scope, QStringLiteral("sampling_frequency"))) {
			return a;
		}
	}
	return nullptr;
}

Context *contextOf(QObject *obj)
{
	for(QObject *o = obj; o; o = o->parent()) {
		if(auto *ctx = qobject_cast<Context *>(o)) {
			return ctx;
		}
	}
	return nullptr;
}
} // namespace

AcquisitionView::AcquisitionView(InputStream *stream, QWidget *parent)
	: QWidget(parent)
	, m_stream(stream)
	, m_acq(new StreamAcquisition(stream, this))
{
	m_sampleRateAttr = findSampleRate(stream);
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
	m_tool->rightContainer()->setVisible(true);
	m_tool->topContainerMenuControl()->setVisible(false);
	m_tool->setRightContainerWidth(300);
	lay->addWidget(m_tool);

	m_status = new QLabel("Stopped", this);
	Style::setStyle(m_status, style::properties::label::subtle);
	m_runBtn = new RunBtn(this);
	m_singleBtn = new SingleShotBtn(this);
	auto *gearBtn = new GearBtn(this);
	gearBtn->setChecked(true);
	m_tool->addWidgetToTopContainerHelper(m_status, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);
	m_tool->addWidgetToTopContainerHelper(m_singleBtn, TTA_RIGHT);
	m_tool->addWidgetToTopContainerHelper(gearBtn, TTA_RIGHT);

	m_tool->rightStack()->add(SETTINGS_KEY, buildSettings(this));
	m_tool->openRightContainerHelper(true);
	connect(gearBtn, &QPushButton::toggled, m_tool, &ToolTemplate::openRightContainerHelper);

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

	// One colored checkbox per scan element; the plot channel shares the color.
	const QList<ScanElement *> elems = m_acq->scanElements();
	const QList<int> enabled = m_acq->enabledChannels();
	auto *chLabel = new QLabel(QStringLiteral("Channels (%1)").arg(elems.size()), section);
	Style::setStyle(chLabel, style::properties::label::subtle);
	lay->addWidget(chLabel);
	for(int i = 0; i < elems.size(); ++i) {
		ScanElement *el = elems[i];
		auto *box = new QCheckBox(displayName(el->id()), section);
		StyleHelper::ColoredSquareCheckbox(box, QColor(StyleHelper::getChannelColor(i)));
		box->setChecked(enabled.contains(int(el->index())));
		lay->addWidget(box);
		m_channelBoxes.insert(el->id(), box);
		m_channelIndex.insert(el->id(), int(el->index()));
		connect(box, &QCheckBox::toggled, this, &AcquisitionView::applyChannelSelection);
	}
	if(elems.isEmpty()) {
		auto *none = new QLabel("This stream exposes no scan elements.", section);
		none->setWordWrap(true);
		lay->addWidget(none);
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
	m_plot = new PlotWidget(this);
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
	m_tool->addWidgetToCentralContainerHelper(m_plot);
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
	m_plot->replot();
}

double AcquisitionView::sampleRate() const
{
	if(m_sampleRateAttr) {
		bool ok = false;
		const double v = m_sampleRateAttr->cachedValue().section(' ', 0, 0).toDouble(&ok);
		return ok ? v : 0;
	}
	return m_manualRate ? m_manualRate->value() : 0;
}

QString AcquisitionView::displayName(const QString &id) const
{
	Channel *ch = channelById(m_stream ? m_stream->parent() : nullptr, id, false);
	if(!ch) {
		return id;
	}
	const QString label = ch->label().isEmpty() ? ch->name() : ch->label();
	return label.isEmpty() || label == id ? id : id + " (" + label + ")";
}

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

	const double fs = sampleRate();
	QVector<float> x(count);
	for(int i = 0; i < count; ++i) {
		x[i] = fs > 0 ? float(i / fs) : float(i);
	}
	// While the user is zoomed / panned the axes are theirs; right click (undo) gives them back.
	const bool zoomed = m_plot->navigator()->isZoomed();
	m_plot->xAxis()->scaleDraw()->setUnitType(fs > 0 ? "s" : "");
	const double xmax = x.last() > 0 ? x.last() : 1;
	if(!zoomed && (m_plot->xAxis()->min() != 0 || m_plot->xAxis()->max() != xmax)) {
		m_plot->xAxis()->setInterval(0, xmax);
	}

	double ymin = std::numeric_limits<double>::max();
	double ymax = std::numeric_limits<double>::lowest();
	for(auto it = m_plotChannels.cbegin(); it != m_plotChannels.cend(); ++it) {
		const int c = frame.ids.indexOf(it.key());
		if(c < 0 || !it.value()->isEnabled()) {
			it.value()->curve()->setSamples(QVector<QPointF>());
			continue;
		}
		const QVector<double> &d = frame.data[c];
		const int n = std::min(count, int(d.size()) - start);
		QVector<float> y(std::max(0, n));
		for(int i = 0; i < n; ++i) {
			y[i] = float(d[start + i]);
			ymin = std::min(ymin, d[start + i]);
			ymax = std::max(ymax, d[start + i]);
		}
		it.value()->setSamples(x.data(), y.data(), size_t(y.size()), true);
	}
	if(!zoomed && ymin <= ymax) {
		const double span = ymax > ymin ? ymax - ymin : std::max(1.0, std::abs(ymax));
		m_plot->yAxis()->setInterval(ymin - span * 0.1, ymax + span * 0.1);
	}
	m_plot->replot();

	QString text = QStringLiteral("Buffer %1 · %2 samples · %3 channel(s)")
			       .arg(frame.index)
			       .arg(total)
			       .arg(frame.ids.size());
	if(swTrigger) {
		text += triggered ? QStringLiteral(" · triggered") : QStringLiteral(" · not triggered: ") + why;
	}
	setStatus(text);
}

void AcquisitionView::onRunningChanged(bool running)
{
	if(!running) {
		QSignalBlocker runBlock(m_runBtn);
		QSignalBlocker singleBlock(m_singleBtn);
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
