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

#include "rawacquisitionview.h"

#include "nodeactivity.h"
#include "nodepanel.h"
#include "readpolicy.h"
#include "sidesplitter.h"
#include "streamutils.h"

#include <QCheckBox>
#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include <limits>

#include <component/attribute.h>
#include <component/channel.h>
#include <component/device.h>
#include <component/inputstream.h>
#include <dcdebugger/style_properties.h>
#include <filemanager.h>
#include <hoverwidget.h>
#include <iiowidget.h>
#include <iiowidgetbuilder.h>
#include <menucontrolbutton.h>
#include <menuonoffswitch.h>
#include <menuplotaxisrangecontrol.h>
#include <menusectionwidget.h>
#include <menuspinbox.h>
#include <plot_utils.hpp>
#include <plotaxis.h>
#include <plotchannel.h>
#include <plotlegend.h>
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

RawAcquisitionView::RawAcquisitionView(InputStream *stream, QWidget *parent)
	: QWidget(parent)
	, m_stream(stream)
	, m_acq(new StreamAcquisition(stream, this))
{
	// The tree flags the stream while this runs.
	NodeActivity::instance()->track(stream, m_acq);
	m_sampleRateAttrs = memberSampleRates(stream, false);

	// Every member channel is acquired: there is no channel selection.
	QList<int> all;
	for(Channel *ch : m_acq->channels()) {
		all.append(int(ch->streamIndex()));
	}
	m_acq->setEnabledChannels(all);

	auto *lay = new QVBoxLayout(this);
	lay->setContentsMargins(0, 0, 0, 0);
	// ToolTemplate starts with every container hidden.
	m_tool = new ToolTemplate(this);
	m_tool->layout()->setContentsMargins(0, 0, 0, 0);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->centralContainer()->setVisible(true);
	m_tool->topContainerMenuControl()->setVisible(false);
	lay->addWidget(m_tool);
	// Curves | plot | Settings menu.
	m_sides = new SideSplitter(200, 300, QStringLiteral("rawacquisition/sides"), this);
	m_tool->addWidgetToCentralContainerHelper(m_sides);

	m_status = new QLabel("Stopped", this);
	Style::setStyle(m_status, style::properties::label::subtle);
	m_runBtn = new RunBtn(this);
	m_singleBtn = new SingleShotBtn(this);
	m_exportBtn = new QPushButton("Export...", this);
	m_exportBtn->setToolTip("Save what the plot shows as CSV");
	Style::setStyle(m_exportBtn, style::properties::dcdebugger::topButton);
	m_exportBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	auto *gearBtn = new GearBtn(this);
	gearBtn->setChecked(m_sides->isRightOpen());
	m_tool->addWidgetToTopContainerHelper(m_status, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);
	m_tool->addWidgetToTopContainerHelper(m_singleBtn, TTA_RIGHT);
	m_tool->addWidgetToTopContainerHelper(m_exportBtn, TTA_RIGHT);
	m_tool->addWidgetToTopContainerHelper(gearBtn, TTA_RIGHT);
	connect(m_exportBtn, &QPushButton::clicked, this, &RawAcquisitionView::exportData);

	buildPlot(); // before the settings: the Y-Axis section binds to its axis
	buildChannels();
	m_sides->rightStack()->add(SETTINGS_KEY, buildSettings(this));
	m_sides->rightStack()->show(SETTINGS_KEY);
	connect(gearBtn, &QPushButton::toggled, m_sides, &SideSplitter::setRightOpen);
	connect(m_sides, &SideSplitter::rightOpenChanged, gearBtn, &QAbstractButton::setChecked);

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
	connect(m_acq, &StreamAcquisition::runningChanged, this, &RawAcquisitionView::onRunningChanged);
	connect(m_acq, &StreamAcquisition::frameReady, this, &RawAcquisitionView::onFrame);
	connect(m_acq, &StreamAcquisition::errorOccurred, this, [this](const QString &msg) {
		setStatus(msg, true);
		StatusBarManager::pushMessage("DC Debugger: " + msg, 5000);
	});
	if(all.isEmpty()) {
		setStatus("This stream exposes no streamable channels.", true);
		m_runBtn->setEnabled(false);
		m_singleBtn->setEnabled(false);
	}
}

RawAcquisitionView::~RawAcquisitionView() = default;

QWidget *RawAcquisitionView::buildSettings(QWidget *parent)
{
	auto *scroll = new QScrollArea(parent);
	scroll->setWidgetResizable(true);
	scroll->setFrameShape(QFrame::NoFrame);
	scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scroll->setObjectName("rawAcquisitionSettingsMenu");
	auto *content = new QWidget(scroll);
	auto *lay = new QVBoxLayout(content);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(10);
	lay->addWidget(buildAcquisitionSection(content));
	lay->addWidget(buildYAxisSection(content));
	lay->addStretch();
	scroll->setWidget(content);
	return scroll;
}

QWidget *RawAcquisitionView::buildAcquisitionSection(QWidget *parent)
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
	m_samples->setObjectName("rawAcquisitionSamples");
	m_samples->setIncrementMode(gui::MenuSpinbox::IS_POW2);
	m_samples->setScalingEnabled(false);
	connect(m_samples, &gui::MenuSpinbox::valueChanged, this, [this](double v) {
		m_acq->setSamples(size_t(std::max(2.0, v)));
		applyXAxis();
		m_plot->replot();
	});
	lay->addWidget(m_samples);

	// One control for the stream: shows the first member's rate, a write is applied to every member.
	if(Attribute *first = m_sampleRateAttrs.value(0)) {
		IIOWidget *rate = IIOWidgetBuilder(section).attribute(first).title("Sample rate").buildSingle();
		if(rate) {
			rate->setObjectName("rawAcquisitionSampleRate");
			lay->addWidget(rate);
			if(!ReadPolicy::suppressed()) { // e.g. built by a "go to origin" from the log
				rate->readAsync();
			}
		}
		linkSampleRates(this, m_sampleRateAttrs, [this](const QString &msg) { setStatus(msg, true); });
	}
	return section;
}

QWidget *RawAcquisitionView::buildYAxisSection(QWidget *parent)
{
	MenuSectionCollapseWidget *section = makeSection("Y-Axis", parent);
	QVBoxLayout *lay = section->contentLayout();
	lay->setSpacing(8);

	m_yAutoscale = new MenuOnOffSwitch("AUTOSCALE", section, false);
	m_yAutoscale->setObjectName("rawAcquisitionYAutoscale");
	m_yAutoscale->onOffswitch()->setChecked(true);
	// Min / Max follow the axis both ways: autoscale and zoom show in them, an edit moves the axis.
	m_yRangeCtrl = new gui::MenuPlotAxisRangeControl(m_plot->yAxis(), section);
	m_yRangeCtrl->setEnabled(false);
	for(gui::MenuSpinbox *sb : {m_yRangeCtrl->minSpinbox(), m_yRangeCtrl->maxSpinbox()}) {
		sb->setScalingEnabled(false); // raw codes: no SI prefixes
	}
	lay->addWidget(m_yAutoscale);
	lay->addWidget(m_yRangeCtrl);

	connect(m_yAutoscale->onOffswitch(), &QAbstractButton::toggled, this, [this](bool on) {
		m_yRangeCtrl->setEnabled(!on);
		if(on) {
			render();
		}
	});
	connect(m_yRangeCtrl, &gui::MenuPlotAxisRangeControl::intervalChanged, this, [this]() { m_plot->replot(); });
	return section;
}

void RawAcquisitionView::buildChannels()
{
	// Show / hide curves only: every channel stays acquired.
	auto *vcm = new VerticalChannelManager(this);
	vcm->setObjectName("rawAcquisitionCurves");
	QString name = QStringLiteral("Channels");
	if(auto *dev = qobject_cast<Device *>(m_stream->parent())) {
		name = dev->name().isEmpty() ? dev->id() : dev->name();
	}
	auto *group = new CollapsableMenuControlButton(vcm);
	group->getControlBtn()->setName(name);
	group->getControlBtn()->button()->setVisible(false);
	group->getControlBtn()->setCheckable(false);
	group->enableOnOffSwitch(true);
	group->onOffSwitch()->setChecked(true);
	group->onOffSwitch()->setToolTip("Show / hide every curve");
	vcm->add(group);
	const QList<Channel *> elems = m_acq->channels();
	for(int i = 0; i < elems.size(); ++i) {
		const QString id = elems[i]->id();
		auto *btn = new MenuControlButton(group);
		btn->setName(displayName(id));
		btn->setCheckBoxStyle(MenuControlButton::CS_CIRCLE);
		btn->setColor(QColor(StyleHelper::getChannelColor(i)));
		btn->button()->setVisible(false);
		btn->setCheckable(false);
		btn->enableToolTip(true);
		btn->checkBox()->setChecked(true);
		group->add(btn);
		QCheckBox *box = btn->checkBox();
		box->setObjectName("rawAcquisitionCurve:" + id);
		// No channel menu: a click on the row toggles the curve.
		connect(btn, &QAbstractButton::clicked, box, &QCheckBox::toggle);
		connect(group->onOffSwitch(), &QAbstractButton::toggled, box, &QCheckBox::setChecked);
		connect(box, &QCheckBox::toggled, this, [this, id](bool on) {
			if(PlotChannel *ch = m_plotChannels.value(id)) {
				on ? ch->enable() : ch->disable();
			}
			render();
		});
		m_curveBoxes.insert(id, box);
	}
	if(elems.isEmpty()) {
		auto *none = new QLabel("This stream exposes no streamable channels.", vcm);
		none->setWordWrap(true);
		vcm->add(none);
	}
	m_sides->leftStack()->add(CHANNELS_KEY, vcm);
}

void RawAcquisitionView::buildPlot()
{
	m_plot = new PlotWidget(this);
	m_plot->xAxis()->scaleDraw()->setFloatPrecision(0);
	m_plot->yAxis()->scaleDraw()->setFloatPrecision(0);
	m_plot->yAxis()->setInterval(-1, 1);
	m_plot->setShowXAxisLabels(true);
	m_plot->setShowYAxisLabels(true);
	m_plot->showAxisLabels();
	// Wheel zoom-out may go past the data range.
	m_plot->navigator()->setBounded(false);
	applyXAxis();

	const QList<Channel *> elems = m_acq->channels();
	for(int c = 0; c < elems.size(); ++c) {
		QPen pen(QColor(StyleHelper::getChannelColor(c)), 1);
		auto *ch = new PlotChannel(displayName(elems[c]->id()), pen, m_plot->xAxis(), m_plot->yAxis(), m_plot);
		m_plot->addPlotChannel(ch);
		ch->enable();
		m_plotChannels.insert(elems[c]->id(), ch);
	}
	// Built after the channels; it follows channels added later as well, and hides the hidden ones.
	m_legend = new PlotLegend(m_plot, m_plot);
	placeLegend();
	m_legend->setVisible(!elems.isEmpty());
	m_plot->replot();
	m_sides->setCentralWidget(m_plot);
}

void RawAcquisitionView::placeLegend()
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

void RawAcquisitionView::applyXAxis()
{
	// Fixed: one buffer spans the axis, sample 0 to Samples.
	m_plot->xAxis()->setInterval(0, double(m_acq->samples()));
}

QString RawAcquisitionView::displayName(const QString &id) const { return streamChannelName(m_stream, id, false); }

void RawAcquisitionView::onFrame(const AcquiredFrame &frame)
{
	int total = 0;
	for(const QVector<double> &d : frame.raw) {
		total = std::max(total, int(d.size()));
	}
	if(total == 0) {
		return;
	}
	m_frame = frame;
	m_hasFrame = true;
	render();
	setStatus(QStringLiteral("Buffer %1 · %2 samples · %3 channel(s)")
			  .arg(frame.index)
			  .arg(total)
			  .arg(frame.ids.size()));
}

void RawAcquisitionView::render()
{
	if(!m_hasFrame) {
		m_plot->replot();
		return;
	}
	double ylo = std::numeric_limits<double>::max();
	double yhi = std::numeric_limits<double>::lowest();
	for(auto it = m_plotChannels.cbegin(); it != m_plotChannels.cend(); ++it) {
		const int c = m_frame.ids.indexOf(it.key());
		if(c < 0 || c >= m_frame.raw.size()) {
			it.value()->curve()->setSamples(QVector<QPointF>());
			continue;
		}
		const QVector<double> &d = m_frame.raw[c];
		const bool shown = it.value()->isEnabled();
		QVector<float> xf(d.size());
		QVector<float> yf(d.size());
		for(int i = 0; i < d.size(); ++i) {
			xf[i] = float(i);
			yf[i] = float(d[i]);
			if(shown) { // autoscale follows what is shown
				ylo = std::min(ylo, d[i]);
				yhi = std::max(yhi, d[i]);
			}
		}
		it.value()->setSamples(xf.data(), yf.data(), size_t(d.size()), true);
	}
	// While zoomed / panned the axes are the user's; right click (undo) gives them back.
	if(!m_plot->navigator()->isZoomed()) {
		applyXAxis();
		if(m_yAutoscale->onOffswitch()->isChecked() && ylo <= yhi) {
			const double span = yhi > ylo ? yhi - ylo : std::max(1.0, std::abs(yhi));
			m_plot->yAxis()->setInterval(ylo - span * 0.1, yhi + span * 0.1);
		}
	}
	m_plot->replot();
}

QString RawAcquisitionView::csvData() const
{
	if(!m_hasFrame) {
		return {};
	}
	QString csv = QStringLiteral("sample");
	QList<int> cols;
	for(Channel *ch : m_acq->channels()) {
		const int c = m_frame.ids.indexOf(ch->id());
		if(c >= 0 && c < m_frame.raw.size()) {
			csv += "," + displayName(ch->id());
			cols.append(c);
		}
	}
	if(cols.isEmpty()) {
		return {};
	}
	csv += "\n";
	int rows = 0;
	for(int c : std::as_const(cols)) {
		rows = std::max(rows, int(m_frame.raw[c].size()));
	}
	for(int i = 0; i < rows; ++i) {
		csv += QString::number(i);
		for(int c : std::as_const(cols)) {
			const QVector<double> &d = m_frame.raw[c];
			csv += "," + (i < d.size() ? QString::number(d[i], 'g', 12) : QString());
		}
		csv += "\n";
	}
	return csv;
}

void RawAcquisitionView::exportData()
{
	const QString csv = csvData();
	if(csv.isEmpty()) {
		setStatus("Nothing to export: acquire a buffer first", true);
		return;
	}
	if(FileManagerHelper::saveDataToFile(this, csv, "Export plot data")) {
		setStatus("Exported " + QString::number(csv.count('\n') - 1) + " rows");
	}
}

void RawAcquisitionView::onRunningChanged(bool running)
{
	if(!running) {
		// Not signal-blocked: the buttons switch their text back on toggled.
		m_runBtn->setChecked(false);
		m_singleBtn->setChecked(false);
		m_runBtn->setEnabled(true);
		m_singleBtn->setEnabled(true);
	} else {
		setStatus("Running");
	}
}

void RawAcquisitionView::setStatus(const QString &text, bool error)
{
	m_status->setText(text);
	Style::setStyle(m_status, style::properties::label::warning, error);
	Style::setStyle(m_status, style::properties::label::subtle, !error);
}

#include "moc_rawacquisitionview.cpp"
