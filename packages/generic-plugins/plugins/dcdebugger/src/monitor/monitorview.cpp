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

#include "monitorview.h"

#include "acquisition/measurementstable.h"
#include "nodepanel.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QShowEvent>
#include <QSignalBlocker>
#include <QSplitter>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>
#include <limits>

#include <component/attribute.h>
#include <component/channel.h>
#include <component/context.h>
#include <component/device.h>
#include <component/navigation.h>
#include <dcdebugger/style_properties.h>
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

QString channelMenuKey(const QString &key) { return "channel:" + key; }

// The attribute polled by default: a processed value first, then the raw one.
int defaultSource(const QList<Attribute *> &attrs)
{
	for(const QString &name : {"input", "processed", "raw", "value"}) {
		for(int i = 0; i < attrs.size(); ++i) {
			if(attrs[i]->name() == name) {
				return i;
			}
		}
	}
	return 0;
}

// Channels of the same kind share a plot by default: temp0 / temp1 -> temp, accel_x -> accel.
QString kindOf(const Channel *ch)
{
	QString id = ch->id();
	id.remove(QRegularExpression(QStringLiteral("\\d+$")));
	static const QRegularExpression axis(QStringLiteral("_[a-z]$"));
	id.remove(axis);
	return id + (ch->isOutput() ? QStringLiteral(":out") : QString());
}

QTableWidgetItem *cell(const QString &text, Qt::Alignment align = Qt::AlignRight | Qt::AlignVCenter)
{
	auto *item = new QTableWidgetItem(text);
	item->setTextAlignment(align);
	return item;
}
} // namespace

QList<Channel *> MonitorView::monitorableChannels(QObject *root)
{
	QList<Device *> devices;
	if(auto *dev = qobject_cast<Device *>(root)) {
		devices.append(dev);
	} else if(root) {
		devices = root->findChildren<Device *>(QString(), Qt::FindDirectChildrenOnly);
	}
	QList<Channel *> out;
	for(Device *dev : devices) {
		for(Channel *ch : dev->findChildren<Channel *>(QString(), Qt::FindDirectChildrenOnly)) {
			if(!ch->findChildren<Attribute *>(QString(), Qt::FindDirectChildrenOnly).isEmpty()) {
				out.append(ch);
			}
		}
	}
	return out;
}

MonitorView::MonitorView(QObject *root, QWidget *parent)
	: QWidget(parent)
	, m_root(root)
	, m_monitor(new AttributeMonitor(this))
	, m_redraw(new QTimer(this))
{
	m_redraw->setInterval(50); // redraw at most 20 times a second, whatever the poll period
	connect(m_redraw, &QTimer::timeout, this, [this]() {
		if(m_dirty) {
			render();
		}
	});
	connect(m_monitor, &AttributeMonitor::ticked, this, [this]() { m_dirty = true; });

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
	m_tool->setLeftContainerWidth(220);
	lay->addWidget(m_tool);

	m_status = new QLabel("Stopped", this);
	Style::setStyle(m_status, style::properties::label::subtle);
	m_runBtn = new RunBtn(this);
	disconnect(m_runBtn, &QPushButton::toggled, m_runBtn, nullptr); // RunBtn's own Run / Stop text
	m_runBtn->setText("Start");
	m_clearBtn = new QPushButton("Clear", this);
	Style::setStyle(m_clearBtn, style::properties::dcdebugger::topButton);
	auto *gearBtn = new GearBtn(this);
	gearBtn->setChecked(true);
	m_tool->addWidgetToTopContainerHelper(m_status, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);
	m_tool->addWidgetToTopContainerHelper(m_clearBtn, TTA_RIGHT);
	m_tool->addWidgetToTopContainerHelper(gearBtn, TTA_RIGHT);
	m_tool->rightStack()->add(SETTINGS_KEY, buildSettings(this));
	m_tool->openRightContainerHelper(true);
	connect(gearBtn, &QPushButton::toggled, m_tool, &ToolTemplate::openRightContainerHelper);

	buildPlots();
	buildChannels();
	m_tool->openLeftContainerHelper(true);

	connect(m_runBtn, &QPushButton::toggled, this, [this](bool on) {
		m_runBtn->setText(on ? "Stop" : "Start");
		if(on) {
			m_monitor->start();
			m_redraw->start();
		} else {
			m_monitor->stop();
			m_redraw->stop();
			render();
		}
		updateStatus();
	});
	connect(m_clearBtn, &QPushButton::clicked, m_monitor, &AttributeMonitor::clear);
	connect(m_clearBtn, &QPushButton::clicked, this, &MonitorView::render);
	connect(m_monitor, &AttributeMonitor::readFailed, this, [this](const QString &key, const QString &msg) {
		if(Entry *e = entry(key)) {
			m_status->setText(e->name + ": " + msg);
		}
	});
	updateStatus();
}

MonitorView::~MonitorView() = default;

QWidget *MonitorView::buildSettings(QWidget *parent)
{
	auto *scroll = new QScrollArea(parent);
	scroll->setWidgetResizable(true);
	scroll->setFrameShape(QFrame::NoFrame);
	scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scroll->setObjectName("monitorSettingsMenu");
	auto *content = new QWidget(scroll);
	auto *outer = new QVBoxLayout(content);
	outer->setContentsMargins(0, 0, 0, 0);
	outer->setSpacing(10);

	// Settings menu: the Polling section, then the Plots section.
	MenuSectionCollapseWidget *section = makeSection("Polling", content);
	section->setObjectName("monitorPollingSection");
	QVBoxLayout *lay = section->contentLayout();
	lay->setSpacing(8);
	m_period = new gui::MenuSpinbox("Period", m_monitor->period() / 1000.0, "s", 0.001, 3600, true, false, false,
					section);
	m_period->setIncrementMode(gui::MenuSpinbox::IS_125);
	m_history =
		new gui::MenuSpinbox("History", m_monitor->history(), "s", 1, 24 * 3600, true, false, false, section);
	m_history->setIncrementMode(gui::MenuSpinbox::IS_125);
	m_scaled = new MenuOnOffSwitch("Scaled units", section);
	m_scaled->setToolTip("raw sources with scale / offset siblings are shown as (raw + offset) * scale");
	m_scaled->onOffswitch()->setChecked(m_monitor->scaled());
	lay->addWidget(m_period);
	lay->addWidget(m_history);
	lay->addWidget(m_scaled);
	connect(m_period, &gui::MenuSpinbox::valueChanged, this, [this](double s) {
		m_monitor->setPeriod(int(std::lround(s * 1000)));
		updateStatus();
	});
	connect(m_history, &gui::MenuSpinbox::valueChanged, this, [this](double s) {
		m_monitor->setHistory(s);
		render();
	});
	connect(m_scaled->onOffswitch(), &QAbstractButton::toggled, this, [this](bool on) {
		m_monitor->setScaled(on);
		render();
	});

	outer->addWidget(section);
	outer->addWidget(buildPlotSettings(content));
	outer->addStretch();
	// Built before the first plot: plotsChanged() fills the Plots section from the start.
	connect(this, &MonitorView::plotsChanged, this, &MonitorView::updatePlotControls);
	scroll->setWidget(content);
	return scroll;
}

QWidget *MonitorView::buildPlotSettings(QWidget *parent)
{
	MenuSectionCollapseWidget *section = makeSection("Plots", parent);
	section->setObjectName("monitorPlotsSection");
	QVBoxLayout *lay = section->contentLayout();
	lay->setSpacing(6);
	// The existing plots; Remove acts on the selected one. Filled by updatePlotControls().
	m_plotSelect = new MenuCombo("Plot", section);
	m_plotSelect->setObjectName("monitorPlotSelectMenu");
	m_plotSelect->combo()->setObjectName("monitorPlotSelect");
	m_plotSelect->setToolTip("Selected plot: Remove plot removes it");
	connect(m_plotSelect->combo(), qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int idx) {
		if(idx >= 0) {
			m_selectedPlot = idx;
		}
	});
	m_addPlotBtn = new QPushButton("Add plot", section);
	m_addPlotBtn->setObjectName("monitorAddPlot");
	m_addPlotBtn->setToolTip(QStringLiteral("Add a plot under the others (at most %1)").arg(MaxPlots));
	Style::setStyle(m_addPlotBtn, style::properties::button::basicButton);
	connect(m_addPlotBtn, &QPushButton::clicked, this, [this]() { addPlot(); });
	m_removePlotBtn = new QPushButton("Remove plot", section);
	m_removePlotBtn->setObjectName("monitorRemovePlot");
	m_removePlotBtn->setToolTip("Remove the selected plot; its channels move to the first plot");
	Style::setStyle(m_removePlotBtn, style::properties::button::basicButton);
	m_removePlotBtn->setVisible(false); // shown from 2 plots on
	connect(m_removePlotBtn, &QPushButton::clicked, this,
		[this]() { removePlot(m_plotSelect->combo()->currentIndex()); });
	lay->addWidget(m_plotSelect);
	lay->addWidget(m_addPlotBtn);
	lay->addWidget(m_removePlotBtn);
	return section;
}

void MonitorView::buildChannels()
{
	auto *vcm = new VerticalChannelManager(this);
	m_menuGroup = new QButtonGroup(this);
	m_menuGroup->setExclusive(true);

	auto *settingsBtn = new MenuControlButton(vcm);
	settingsBtn->setObjectName("monitorSettingsButton");
	settingsBtn->setName("Settings");
	settingsBtn->setOpenMenuChecksThis(true);
	settingsBtn->setDoubleClickToOpenMenu(false);
	settingsBtn->checkBox()->setVisible(false);
	settingsBtn->button()->setVisible(false);
	settingsBtn->setCheckable(true);
	settingsBtn->setChecked(true);
	vcm->addTop(settingsBtn);
	m_menuGroup->addButton(settingsBtn);
	connect(settingsBtn, &QAbstractButton::clicked, this, [this]() { m_tool->rightStack()->show(SETTINGS_KEY); });

	QMap<QString, int> kinds; // channel kind -> plot, over the existing plots
	QMap<Device *, CollapsableMenuControlButton *> groups;
	const QList<Channel *> channels = monitorableChannels(m_root);
	for(int i = 0; i < channels.size(); ++i) {
		Channel *ch = channels[i];
		auto *dev = qobject_cast<Device *>(ch->parent());
		const QString devName = dev ? (dev->name().isEmpty() ? dev->id() : dev->name()) : QString();

		// One collapsible group per device; its switch toggles all of the device's channels.
		CollapsableMenuControlButton *group = groups.value(dev);
		if(!group) {
			group = new CollapsableMenuControlButton(vcm);
			group->getControlBtn()->setName(devName);
			group->getControlBtn()->button()->setVisible(false);
			group->getControlBtn()->setCheckable(false);
			group->enableOnOffSwitch(true);
			group->onOffSwitch()->setChecked(false);
			vcm->add(group);
			groups.insert(dev, group);
		}

		Entry e;
		e.key = devName + "/" + ch->id() + (ch->isOutput() ? " (out)" : "");
		e.channel = ch;
		e.name = e.key;
		e.color = QColor(StyleHelper::getChannelColor(i));
		e.attributes = ch->findChildren<Attribute *>(QString(), Qt::FindDirectChildrenOnly);
		const QString kind = kindOf(ch);
		if(!kinds.contains(kind)) {
			kinds.insert(kind, int(kinds.size() % m_plots.size()));
		}
		e.plotIndex = kinds.value(kind);

		e.button = new MenuControlButton(group);
		e.button->setName(ch->id() + (ch->isOutput() ? " (out)" : ""));
		e.button->setCheckBoxStyle(MenuControlButton::CS_CIRCLE);
		e.button->setColor(e.color);
		e.button->setOpenMenuChecksThis(true);
		e.button->setDoubleClickToOpenMenu(true);
		e.button->button()->setVisible(false);
		e.button->setCheckable(true);
		e.button->checkBox()->setChecked(false);
		e.button->enableToolTip(true);
		group->add(e.button);
		m_menuGroup->addButton(e.button);
		connect(group->onOffSwitch(), &QAbstractButton::toggled, e.button->checkBox(), &QCheckBox::setChecked);

		m_tool->rightStack()->add(channelMenuKey(e.key), buildChannelMenu(e));
		m_entries.append(e);
	}

	for(Entry &e : m_entries) {
		applySource(e);
		const QString key = e.key;
		connect(e.button, &QAbstractButton::clicked, this,
			[this, key]() { m_tool->rightStack()->show(channelMenuKey(key)); });
		connect(e.button->checkBox(), &QCheckBox::toggled, this, [this, key](bool on) {
			Entry *en = entry(key);
			m_monitor->setEnabled(key, on);
			if(en && en->curve) {
				on ? en->curve->enable() : en->curve->disable();
			}
			render();
			updateStatus();
		});
		connect(e.source->combo(), qOverload<int>(&QComboBox::currentIndexChanged), this, [this, key]() {
			if(Entry *en = entry(key)) {
				applySource(*en);
				render();
			}
		});
		connect(e.plot->combo(), qOverload<int>(&QComboBox::currentIndexChanged), this, [this, key](int idx) {
			Entry *en = entry(key);
			if(en && idx >= 0) {
				assignPlot(*en, idx);
				render();
			}
		});
		assignPlot(e, e.plotIndex);
	}
	if(m_entries.isEmpty()) {
		auto *none = new QLabel("No channel with attributes to poll.", vcm);
		none->setWordWrap(true);
		vcm->add(none);
	}
	m_tool->leftStack()->add(CHANNELS_KEY, vcm);
	m_tool->rightStack()->show(SETTINGS_KEY);
}

QWidget *MonitorView::buildChannelMenu(Entry &e)
{
	auto *menu = new gui::MenuWidget(e.name, QPen(e.color), this);

	auto *sourceSection = new MenuSectionCollapseWidget("SOURCE", MenuCollapseSection::MHCW_NONE,
							    MenuCollapseSection::MHW_BASEWIDGET, menu);
	e.source = new MenuCombo("Attribute", sourceSection);
	for(Attribute *a : std::as_const(e.attributes)) {
		e.source->combo()->addItem(a->name());
	}
	e.source->combo()->setCurrentIndex(defaultSource(e.attributes));
	sourceSection->contentLayout()->addWidget(e.source);
	menu->add(sourceSection, "source", gui::MenuWidget::MA_TOPLAST);

	auto *plotSection = new MenuSectionCollapseWidget("PLOT", MenuCollapseSection::MHCW_NONE,
							  MenuCollapseSection::MHW_BASEWIDGET, menu);
	e.plot = new MenuCombo("Plot", plotSection);
	e.plot->combo()->setObjectName("monitorPlotCombo:" + e.key);
	for(int p = 0; p < m_plots.size(); ++p) {
		e.plot->combo()->addItem(QStringLiteral("Plot %1").arg(p + 1));
	}
	e.plot->combo()->setCurrentIndex(e.plotIndex);
	e.plot->setToolTip("Channels on the same plot share its axes; pick another plot to separate them.");
	plotSection->contentLayout()->addWidget(e.plot);
	menu->add(plotSection, "plot", gui::MenuWidget::MA_TOPLAST);

	auto *guard = new MouseWheelWidgetGuard(menu);
	guard->installEventRecursively(menu);
	return menu;
}

void MonitorView::buildPlots()
{
	m_splitter = new QSplitter(Qt::Vertical, this);
	m_splitter->setChildrenCollapsible(true);

	m_plotsArea = new QWidget(m_splitter);
	m_plotsLay = new QVBoxLayout(m_plotsArea);
	m_plotsLay->setContentsMargins(0, 0, 0, 0);
	m_plotsLay->setSpacing(6);

	m_table = new QTableWidget(m_splitter);
	const QStringList headers = {"Channel", "Value", "Min", "Max", "Mean"};
	m_table->setColumnCount(headers.size());
	m_table->setHorizontalHeaderLabels(headers);
	m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	m_table->verticalHeader()->setVisible(false);
	m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_table->setSelectionMode(QAbstractItemView::NoSelection);
	m_table->setFocusPolicy(Qt::NoFocus);
	Style::setStyle(m_table, style::properties::widget::tableViewWidget);

	m_splitter->addWidget(m_plotsArea);
	m_splitter->addWidget(m_table);
	// Extra height goes to the plots; the table keeps its size (see applyInitialSizes()).
	m_splitter->setStretchFactor(0, 1);
	m_splitter->setStretchFactor(1, 0);
	m_tool->addWidgetToCentralContainerHelper(m_splitter);

	addPlot(); // there is always at least one plot
}

PlotWidget *MonitorView::createPlot()
{
	auto *pw = new PlotWidget(m_plotsArea);
	pw->xAxis()->scaleDraw()->setFormatter(new MetricPrefixFormatter());
	pw->xAxis()->scaleDraw()->setFloatPrecision(2);
	pw->xAxis()->scaleDraw()->setUnitType("s");
	pw->yAxis()->scaleDraw()->setFormatter(new MetricPrefixFormatter());
	pw->yAxis()->scaleDraw()->setFloatPrecision(3);
	pw->xAxis()->setInterval(-m_monitor->history(), 0);
	pw->yAxis()->setInterval(-1, 1);
	pw->setShowXAxisLabels(true);
	pw->setShowYAxisLabels(true);
	pw->showAxisLabels();
	pw->navigator()->setBounded(false);
	return pw;
}

int MonitorView::addPlot()
{
	if(m_plots.size() >= MaxPlots) {
		return -1;
	}
	m_plots.append(createPlot());
	m_selectedPlot = plotCount() - 1; // the new plot is selected in the Plots section
	rebuildPlotsLayout();
	rebuildPlotCombos();
	render();
	Q_EMIT plotsChanged(plotCount());
	return plotCount() - 1;
}

bool MonitorView::removePlot(int index)
{
	if(m_plots.size() <= 1 || index < 0 || index >= m_plots.size()) {
		return false;
	}
	// Move the curves off first (assignPlot() detaches and frees them), then renumber.
	const int target = index == 0 ? 1 : 0;
	for(Entry &e : m_entries) {
		if(e.plotIndex == index) {
			assignPlot(e, target);
		}
	}
	PlotWidget *gone = m_plots.takeAt(index);
	for(Entry &e : m_entries) {
		if(e.plotIndex > index) {
			e.plotIndex--;
		}
	}
	// Keep the Plots section's selection on the same plot, or on the one now in the removed one's place.
	if(m_selectedPlot > index) {
		m_selectedPlot--;
	} else if(m_selectedPlot == index) {
		m_selectedPlot = std::min(index, plotCount() - 1);
	}
	m_plotsLay->removeWidget(gone);
	delete gone;
	rebuildPlotsLayout();
	rebuildPlotCombos();
	render();
	Q_EMIT plotsChanged(plotCount());
	return true;
}

PlotWidget *MonitorView::plot(int index) const
{
	return index >= 0 && index < m_plots.size() ? m_plots[index] : nullptr;
}

int MonitorView::plotIndexOf(const QString &key) const
{
	for(const Entry &e : m_entries) {
		if(e.key == key) {
			return e.plotIndex;
		}
	}
	return -1;
}

void MonitorView::rebuildPlotsLayout()
{
	for(PlotWidget *pw : std::as_const(m_plots)) {
		m_plotsLay->removeWidget(pw);
	}
	for(PlotWidget *pw : std::as_const(m_plots)) {
		m_plotsLay->addWidget(pw, 1);
	}
}

void MonitorView::rebuildPlotCombos()
{
	for(Entry &e : m_entries) {
		if(!e.plot) {
			continue;
		}
		QComboBox *combo = e.plot->combo();
		const QSignalBlocker blocker(combo);
		combo->clear();
		for(int p = 0; p < m_plots.size(); ++p) {
			combo->addItem(QStringLiteral("Plot %1").arg(p + 1));
		}
		combo->setCurrentIndex(e.plotIndex);
	}
}

void MonitorView::updatePlotControls()
{
	const int n = plotCount();
	m_selectedPlot = std::clamp(m_selectedPlot, 0, std::max(0, n - 1));
	if(m_plotSelect) {
		QComboBox *combo = m_plotSelect->combo();
		const QSignalBlocker blocker(combo);
		combo->clear();
		for(int p = 0; p < n; ++p) {
			combo->addItem(QStringLiteral("Plot %1").arg(p + 1));
		}
		combo->setCurrentIndex(n > 0 ? m_selectedPlot : -1);
	}
	if(m_addPlotBtn) {
		m_addPlotBtn->setEnabled(n < MaxPlots);
	}
	if(m_removePlotBtn) {
		m_removePlotBtn->setVisible(n > 1);
	}
}

void MonitorView::showEvent(QShowEvent *event)
{
	QWidget::showEvent(event);
	if(!m_sized) {
		// After the first layout pass, once the splitter has its real height.
		QTimer::singleShot(0, this, &MonitorView::applyInitialSizes);
	}
}

void MonitorView::applyInitialSizes()
{
	const int total = m_splitter->height() - m_splitter->handleWidth();
	if(m_sized || total <= 0) {
		return;
	}
	m_sized = true;
	const int table = std::min(MeasurementsHeight, total / 4);
	m_splitter->setSizes({total - table, table});
}

void MonitorView::applySource(Entry &e)
{
	const int idx = e.source->combo()->currentIndex();
	Attribute *src = idx >= 0 && idx < e.attributes.size() ? e.attributes[idx] : nullptr;
	Attribute *scale = nullptr;
	Attribute *offset = nullptr;
	// IIO convention: a raw value is scaled by its scale / offset siblings.
	if(src && src->name() == QLatin1String("raw")) {
		scale = attributeByName(e.channel, QStringLiteral("scale"));
		offset = attributeByName(e.channel, QStringLiteral("offset"));
	}
	if(m_monitor->keys().contains(e.key)) {
		m_monitor->setSource(e.key, src, scale, offset);
	} else {
		m_monitor->addItem(e.key, src, scale, offset);
	}
}

void MonitorView::assignPlot(Entry &e, int plotIndex)
{
	plotIndex = std::clamp(plotIndex, 0, plotCount() - 1);
	if(e.curve) {
		if(PlotWidget *old = plot(e.plotIndex)) {
			old->removePlotChannel(e.curve);
		}
		delete e.curve;
		e.curve = nullptr;
	}
	e.plotIndex = plotIndex;
	PlotWidget *pw = m_plots[plotIndex];
	e.curve = new PlotChannel(e.name, QPen(e.color, 1), pw->xAxis(), pw->yAxis(), pw);
	pw->addPlotChannel(e.curve);
	m_monitor->isEnabled(e.key) ? e.curve->enable() : e.curve->disable();
}

MonitorView::Entry *MonitorView::entry(const QString &key)
{
	for(Entry &e : m_entries) {
		if(e.key == key) {
			return &e;
		}
	}
	return nullptr;
}

void MonitorView::render()
{
	m_dirty = false;
	const double now = m_monitor->now();
	const double history = m_monitor->history();
	const int plots = plotCount();
	QVector<double> lo(plots, std::numeric_limits<double>::max());
	QVector<double> hi(plots, std::numeric_limits<double>::lowest());
	QVector<bool> used(plots, false);

	for(Entry &e : m_entries) {
		if(!e.curve || e.plotIndex < 0 || e.plotIndex >= plots) {
			continue;
		}
		if(!m_monitor->isEnabled(e.key)) {
			e.curve->curve()->setSamples(QVector<QPointF>());
			continue;
		}
		used[e.plotIndex] = true;
		const QVector<QPointF> s = m_monitor->samples(e.key);
		QVector<float> x(s.size());
		QVector<float> y(s.size());
		for(int i = 0; i < s.size(); ++i) {
			x[i] = float(s[i].x() - now); // now = 0, history to the left
			y[i] = float(s[i].y());
			lo[e.plotIndex] = std::min(lo[e.plotIndex], s[i].y());
			hi[e.plotIndex] = std::max(hi[e.plotIndex], s[i].y());
		}
		e.curve->setSamples(x.data(), y.data(), size_t(s.size()), true);
		if(Attribute *src = m_monitor->source(e.key)) {
			m_plots[e.plotIndex]->yAxis()->scaleDraw()->setUnitType(src->unit());
		}
	}

	// Every plot is shown, with or without curves; all follow the same history window.
	for(int p = 0; p < plots; ++p) {
		PlotWidget *pw = m_plots[p];
		if(pw->navigator()->isZoomed()) {
			continue;
		}
		pw->xAxis()->setInterval(-history, 0);
		if(used[p] && lo[p] <= hi[p]) {
			const double span =
				hi[p] > lo[p] ? hi[p] - lo[p] : std::max(1e-9, std::abs(hi[p]) * 0.01 + 1e-9);
			pw->yAxis()->setInterval(lo[p] - span * 0.1, hi[p] + span * 0.1);
		}
		pw->replot();
	}
	updateTable();
}

void MonitorView::updateTable()
{
	QList<const Entry *> rows;
	for(const Entry &e : m_entries) {
		if(m_monitor->isEnabled(e.key)) {
			rows.append(&e);
		}
	}
	m_table->setRowCount(rows.size());
	for(int r = 0; r < rows.size(); ++r) {
		const Entry *e = rows[r];
		const AttributeMonitor::Stats st = m_monitor->stats(e->key);
		Attribute *src = m_monitor->source(e->key);
		const QString unit = src ? src->unit() : QString();
		QPixmap swatch(10, 10);
		swatch.fill(e->color);
		auto *name = new QTableWidgetItem(QIcon(swatch), e->name + (src ? " · " + src->name() : QString()));
		m_table->setItem(r, 0, name);
		const bool has = st.count > 0;
		m_table->setItem(r, 1, cell(has ? siFormat(st.value, unit, 6) : QStringLiteral("—")));
		m_table->setItem(r, 2, cell(has ? siFormat(st.min, unit, 6) : QStringLiteral("—")));
		m_table->setItem(r, 3, cell(has ? siFormat(st.max, unit, 6) : QStringLiteral("—")));
		m_table->setItem(r, 4, cell(has ? siFormat(st.mean, unit, 6) : QStringLiteral("—")));
	}
}

void MonitorView::updateStatus()
{
	const int n = m_monitor->enabledKeys().size();
	m_status->setText(m_monitor->isRunning() ? QStringLiteral("Polling %1 channel(s) every %2")
							   .arg(n)
							   .arg(siFormat(m_monitor->period() / 1000.0, "s"))
						 : QStringLiteral("Stopped · %1 channel(s) selected").arg(n));
}

#include "moc_monitorview.cpp"
